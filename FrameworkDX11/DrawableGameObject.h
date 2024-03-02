#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include "DirectXHelpers.h"
#include "WICTextureLoader.h"

using namespace DirectX;


class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	void cleanup();
	void LoadOBJ();

	HRESULT								initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								update(float t, ID3D11DeviceContext* pContext, XMFLOAT3 rotation, XMFLOAT3 position);
	void								draw(ID3D11DeviceContext* pContext);
	void CalculateModelVectors(SimpleVertex* vertices, int vertexCount);
	void CalculateTangentBinormalLH(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal);
	void CalculateTangentBinormalRH(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal);
	ID3D11Buffer*						getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer*						getIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView**			getTextureResourceView() { return &m_pTextureResourceView; 	}
	XMFLOAT4X4*							getTransform() { return &m_World; }
	ID3D11SamplerState**				getTextureSamplerState() { return &m_pSamplerLinear; }
	ID3D11Buffer*						getMaterialConstantBuffer() { return m_pMaterialConstantBuffer;}
	void								setPosition(XMFLOAT3 position);
	XMFLOAT3							getPosition();
	void SetTexture(ID3D11ShaderResourceView* shaderRV) { m_pTextureResourceView = shaderRV; }

private:
	
	XMFLOAT4X4							m_World;

	ID3D11Buffer*						m_pVertexBuffer;
	ID3D11Buffer*						m_pIndexBuffer;
	ID3D11ShaderResourceView*			m_pTextureResourceView;
	ID3D11ShaderResourceView*			m_pNormalMapResourceView;
	ID3D11SamplerState *				m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;
	ID3D11Buffer*						m_pMaterialConstantBuffer = nullptr;
	XMFLOAT3							m_position;
};