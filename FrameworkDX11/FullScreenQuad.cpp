#include "FullScreenQuad.h"

using namespace std;
using namespace DirectX;

#define NUM_VERTICES 6

struct SimpleVertexs
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};



FullScreenQuad::FullScreenQuad()
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pTextureResourceView = nullptr;
	m_pSamplerLinear = nullptr;

	// Initialize the world matrix
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}

FullScreenQuad::~FullScreenQuad()
{
	cleanup();
}

void FullScreenQuad::cleanup()
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	m_pVertexBuffer = nullptr;

	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	m_pIndexBuffer = nullptr;

	if (m_pTextureResourceView)
		m_pTextureResourceView->Release();
	m_pTextureResourceView = nullptr;

	if (m_pSamplerLinear)
		m_pSamplerLinear->Release();
	m_pSamplerLinear = nullptr;

	if (m_pMaterialConstantBuffer)
		m_pMaterialConstantBuffer->Release();
	m_pMaterialConstantBuffer = nullptr;
}

HRESULT FullScreenQuad::initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{

	// Define the vertices and indices for the fullscreen quad
	SimpleVertexs vertices[] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },  // Bottom left
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },   // Top left
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },    // Top right
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }    // Bottom right
	};

	WORD indices[] = {
		0, 1, 2,
		0, 2, 3,
	};


	// Create vertex buffer
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	ID3D11Buffer* vertexBuffer = nullptr;
	pd3dDevice->CreateBuffer(&vbd, &initData, &vertexBuffer);

	// Create index buffer
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	initData.pSysMem = indices;
	ID3D11Buffer* indexBuffer = nullptr;
	pd3dDevice->CreateBuffer(&ibd, &initData, &indexBuffer);

	// Create a sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

	// Set the sampler state
	pContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

	// Correctly assigning buffers to class members
	m_pVertexBuffer = vertexBuffer;
	m_pIndexBuffer = indexBuffer;


	return S_OK;
}

void FullScreenQuad::setPosition(XMFLOAT3 position)
{
	m_position = position;
}

void FullScreenQuad::update(float t, ID3D11DeviceContext* pContext, XMFLOAT3 rotation, XMFLOAT3 position)
{
	static float cummulativeTime = 0.1;
	cummulativeTime += t;

	// Cube:  Rotate around origin
	XMMATRIX mSpinY = XMMatrixRotationY(cummulativeTime * rotation.y);
	XMMATRIX mSpinX = XMMatrixRotationX(cummulativeTime * rotation.x);
	XMMATRIX mSpinZ = XMMatrixRotationZ(cummulativeTime * rotation.z);


	XMMATRIX mTranslate = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX world = mTranslate * mSpinY * mSpinX * mSpinZ;
	XMStoreFloat4x4(&m_World, world);

	pContext->UpdateSubresource(m_pMaterialConstantBuffer, 0, nullptr, &m_material, 0, 0);
}

void FullScreenQuad::draw(ID3D11DeviceContext* pContext)
{

	UINT stride = sizeof(SimpleVertexs);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->PSSetShaderResources(0, 1, &m_pTextureResourceView);
	pContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

	pContext->DrawIndexed(6, 0, 0);
}
