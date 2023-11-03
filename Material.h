#pragma once
#include "Texture.h"

struct VS_MaterialMappedFormat {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT4 emissive;
	UINT nTypes;
};

class Material {
private:
	shared_ptr<Texture> pTexture;
	ComPtr<ID3D12Resource> pMaterialBuffer;
	UINT nType = 0;			// 쉐이더 내에서 mask값을 위한 변수
	
public:
	Material();
	~Material();
	//void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void DefaultMaterial(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadMaterial(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void SetTexture(shared_ptr<Texture> _pTexture) { pTexture = _pTexture; };
};

