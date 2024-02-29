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

class FullScreenQuad
{
public:
	FullScreenQuad();
	~FullScreenQuad();

	void cleanup();

	HRESULT								initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								update(float t, ID3D11DeviceContext* pContext, XMFLOAT3 rotation, XMFLOAT3 position);
	void								draw(ID3D11DeviceContext* pContext);
	void								setPosition(XMFLOAT3 position);
	void SetTexture(ID3D11ShaderResourceView* shaderRV) { m_pTextureResourceView = shaderRV; }
		
private:

	XMFLOAT4X4							m_World;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11ShaderResourceView* m_pTextureResourceView;
	ID3D11SamplerState* m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;
	ID3D11Buffer* m_pMaterialConstantBuffer = nullptr;
	XMFLOAT3							m_position;
};

