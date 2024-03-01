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


class FullScreenQuad
{
public:
	FullScreenQuad();
	~FullScreenQuad();

	void cleanup();

	HRESULT								initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void draw(ID3D11DeviceContext* pContext);
	ID3D11Buffer* getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer* getIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView** getTextureResourceView() { return &m_pTextureResourceView; }
	XMFLOAT4X4* getTransform() { return &m_World; }
	ID3D11SamplerState** getTextureSamplerState() { return &m_pSamplerLinear; }
	void setTexture(ID3D11ShaderResourceView* textureSRV);
	void setSize(float s) { size = s; };
	void setPos(XMFLOAT2 p) { pos = p; };
private:

	float size = 1.0f;
	XMFLOAT2 pos = XMFLOAT2(0, 0);
	XMFLOAT4X4							m_World;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11ShaderResourceView* m_pTextureResourceView;
	ID3D11ShaderResourceView* m_pNormalMapResourceView;
	ID3D11SamplerState* m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;
	ID3D11Buffer* m_pMaterialConstantBuffer = nullptr;
	XMFLOAT3							m_position;
};

