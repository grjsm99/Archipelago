#pragma once
class Texture : public enable_shared_from_this<Texture>
{
public:
	Texture(int _nTexture, UINT textureType, int nSampler, int nRootParameter);
	~Texture();
private:
	vector<string> name;
	UINT textureType;
	int nTexture;
	
	
	vector<ComPtr<ID3D12Resource>> pTextureBuffers;	 // 텍스처 맵에 대한 리소스 버퍼
	vector<ComPtr<ID3D12Resource>> pTextureUploadBuffers;
	
	vector<UINT> resourceTypes;
	vector<DXGI_FORMAT> bufferFormats;
	vector<int> bufferElements;
	vector<int> bufferStrides;
	
	int	nRootParameter = 0;
	vector<int> rootParameterIndices;
	
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> srvGpuDescriptorHandles;
	
	int nSampler;
	vector<D3D12_GPU_DESCRIPTOR_HANDLE> samplerGpuDescriptorHandles;
	
	
public:
	void SetRootParameterIndex(int _index, UINT _nRootParameterIndex);
	void UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> _pCommandList);
	bool LoadFromFile(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _resourceType, int _index, int _startRootSignatureIndex);
	
	ComPtr<ID3D12Resource> CreateTexture(const ComPtr<ID3D12Device>& _pDevice, UINT _width, UINT _height, DXGI_FORMAT _format, D3D12_RESOURCE_FLAGS _resourceFlags, D3D12_RESOURCE_STATES _resourceStates, D3D12_CLEAR_VALUE* _clearValue, UINT _ResourceType, UINT _index);
	void CreateBuffer(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, UINT nIndex);
	void ReleaseUploadBuffers();
	
	void SetGpuDescriptorHandle(int _index, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuDescriptorHandle);
	
	// 해당 인덱스의 텍스처 리소스를 반환
	ComPtr<ID3D12Resource> GetResource(int _index) { return pTextureBuffers[_index]; };
	// 해당 인덱스의 텍스처의 서술자 핸들을 반환
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int _index) { return(srvGpuDescriptorHandles[_index]); }
	// 
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int _index);
	UINT GetTextureType(int _index) { return(resourceTypes[_index]); }
	int GetnTexture() { return(nTexture); }
	int GetnRootParameter() { return(nRootParameter); };
	

};

class TextureManager {
	unordered_map<string, shared_ptr<Texture>> storage;
public:
	shared_ptr<Texture> GetTexture(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void ReleaseUploadBuffers();
};