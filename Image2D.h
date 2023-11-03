#pragma once
#include "GameObject.h"


class Image2D {

private:
	// 2D UI 전용 쉐이더	
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
	
private:
	string name;
	bool active;
	XMFLOAT2 startuv;
	XMFLOAT2 sizeuv;
	XMFLOAT2 position;
	
	shared_ptr<Texture> pTexture;
	
	// position 내의 x,z값을 사용
	// cbuffer에서의 worldT._11, 22 = 시작점의 위치
	// _41, _42, _43, _44 = uv.x의 시작점, 크기, uv.y의 시작점, 크기
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

public:

	Image2D(const string& _fileName, XMFLOAT2 _size, XMFLOAT2 _position, XMFLOAT2 _uvsize, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~Image2D();
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void ReleaseUploadBuffers() { pPositionUploadBuffer.Reset(); };
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void SetStartUV(XMFLOAT2 _startuv) { startuv = _startuv; };
	void SetSizeUV(XMFLOAT2 _sizeuv) { sizeuv = _sizeuv; };
};
