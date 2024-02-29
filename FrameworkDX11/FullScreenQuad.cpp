#include "FullScreenQuad.h"

using namespace std;
using namespace DirectX;

#define NUM_VERTICES 6

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
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1, -1, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1, 1, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1, 1, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1, -1, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	WORD indices[] = {
		0, 1, 2,
		0, 2, 3,
	};


	// Create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	pd3dDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	// Create the index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initData.pSysMem = indices;
	pd3dDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

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

	pContext->PSSetShaderResources(0, 1, &m_pTextureResourceView);
	pContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pContext->DrawIndexed(6, 0, 0);
}
