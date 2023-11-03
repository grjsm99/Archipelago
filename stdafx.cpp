#include "stdafx.h"
#include "DDSTextureLoader12.h"
//#include "WICTextureLoader12.h"


UINT gnRtvDescriptorIncrementSize = 0;
UINT gnDsvDescriptorIncrementSize = 0;
UINT gnCbvSrvDescriptorIncrementSize = 0;
D3D12_VERTEX_BUFFER_VIEW t1;
D3D12_VERTEX_BUFFER_VIEW t2;
random_device rd;
mt19937 gen;

bool isEnv = true;

float random(float min, float max) {
	uniform_real_distribution<float> dis(min, max);
	return dis(rd);
}


void ReadStringBinary(string& _dest, ifstream& _file){
	// 글자수 만큼 string을 읽는 함수
	int len;
	_file.read((char*)&len, sizeof(len));
	_dest.assign(len, ' ');
	_file.read((char*)_dest.data(), sizeof(char) * len);
}
ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates, ComPtr<ID3D12Resource>& _pUploadBuffer) {
	ComPtr<ID3D12Resource> pBuffer;

	// 리소스의 처음 상태
	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;	// 리소스가 디폴트 타입일경우
	if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {		// 리소스가 업로드 타입일경우
		resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (_heapType == D3D12_HEAP_TYPE_READBACK) {	// 리소스가 리드백 타입일경우
		resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	// 힙에 대한 설명
	auto heapType = CD3DX12_HEAP_PROPERTIES(_heapType);

	// 리소스에 대한 설명
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(_byteSize);

	// 힙의 정보와 리소스 정보를 가지고 리소스를 만든다.
	HRESULT hResult = _pDevice->CreateCommittedResource(&heapType, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates, NULL, IID_PPV_ARGS(&pBuffer));

	if (_pData) {
		if (_heapType == D3D12_HEAP_TYPE_DEFAULT) {
			// 디폴트 리소스의 경우 CPU가 직접 데이터를 쓰지 못하기 때문에 업로드 버퍼를 거쳐 디폴트버퍼에 복사한다.

			// GPU에 업로드 버퍼를 만든다(할당한다).
			heapType.Type = D3D12_HEAP_TYPE_UPLOAD;
			hResult = _pDevice->CreateCommittedResource(&heapType, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&_pUploadBuffer));

			auto t =  _pDevice->GetDeviceRemovedReason();
			// 위에서 만든 업로드버퍼의 주소값을 알아내어 CPU메모리 에 있는 데이터(_pData)를 GPU메모리(pUploadBuffer)에 복사한다.
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			_pUploadBuffer->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			_pUploadBuffer->Unmap(0, NULL);

			_pCommandList->CopyResource(pBuffer.Get(), _pUploadBuffer.Get());

			D3D12_RESOURCE_BARRIER resourceBarrier;
			::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
			resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resourceBarrier.Transition.pResource = pBuffer.Get();
			resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			resourceBarrier.Transition.StateAfter = _resourceStates;
			resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			_pCommandList->ResourceBarrier(1, &resourceBarrier);
		}
		else if (_heapType == D3D12_HEAP_TYPE_UPLOAD) {
			D3D12_RANGE readRange = { 0, 0 };
			shared_ptr<UINT8> pBufferDataBegin;
			pBuffer->Map(0, &readRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin.get(), _pData, _byteSize);
			pBuffer->Unmap(0, NULL);
		}
		else if (_heapType == D3D12_HEAP_TYPE_READBACK) {
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, _pData, _byteSize);
			pBuffer->Unmap(0, NULL);
		}
	}

	return pBuffer;
}

//xmfloat 출력하기
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3) {
	os << "(" << f3.x << " " << f3.y << " " << f3.z << ")";
	return os;
}
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4) {
	os << "(" << f4.x << " " << f4.y << " " << f4.z << " " << f4.w << ")";
	return os;
}
std::ostream& operator<<(std::ostream& os, const XMFLOAT4X4& f4x4) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			os << f4x4.m[i][j] << " ";
		}
		os << "\n";
	}
	return os;
}


void SynchronizeResourceTransition(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource, D3D12_RESOURCE_STATES d3dStateBefore, D3D12_RESOURCE_STATES d3dStateAfter)
{
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dResource;
	d3dResourceBarrier.Transition.StateBefore = d3dStateBefore;
	d3dResourceBarrier.Transition.StateAfter = d3dStateAfter;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
}


////////// 텍스처 load 함수

ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ComPtr<ID3D12Resource> pd3dTexture = NULL;		// 텍스처 리소스
	std::unique_ptr<uint8_t[]> ddsData;				// DDS 파일 데이터
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;
	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &pd3dTexture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	//	D3D12_RESOURCE_DESC d3dResourceDesc = pd3dTexture->GetDesc();
	//	UINT nSubResources = d3dResourceDesc.DepthOrArraySize * d3dResourceDesc.MipLevels;
	UINT nSubResources = (UINT)vSubresources.size();
	//	UINT64 nBytes = 0;
	//	pd3dDevice->GetCopyableFootprints(&d3dResourceDesc, 0, nSubResources, 0, NULL, NULL, NULL, &nBytes);
	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture.Get(), 0, nSubResources);
	
	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

	//UINT nSubResources = (UINT)vSubresources.size();
	//D3D12_SUBRESOURCE_DATA *pd3dSubResourceData = new D3D12_SUBRESOURCE_DATA[nSubResources];
	//for (UINT i = 0; i < nSubResources; i++) pd3dSubResourceData[i] = vSubresources.at(i);

	//	std::vector<D3D12_SUBRESOURCE_DATA>::pointer ptr = &vSubresources[0];
	::UpdateSubresources(pd3dCommandList, pd3dTexture.Get(), *ppd3dUploadBuffer, 0, 0, nSubResources, &vSubresources[0]);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture.Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	return(pd3dTexture);
}

//ID3D12Resource* CreateTextureResourceFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
//{
//	ID3D12Resource* pd3dTexture = NULL;
//	std::unique_ptr<uint8_t[]> decodedData;
//	D3D12_SUBRESOURCE_DATA d3dSubresource;
//
//	HRESULT hResult = DirectX::LoadWICTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_DEFAULT, &pd3dTexture, decodedData, d3dSubresource);
//
//	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
//	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
//	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
//	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//	d3dHeapPropertiesDesc.CreationNodeMask = 1;
//	d3dHeapPropertiesDesc.VisibleNodeMask = 1;
//
//	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture, 0, 1);
//
//	D3D12_RESOURCE_DESC d3dResourceDesc;
//	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
//	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
//	d3dResourceDesc.Alignment = 0;
//	d3dResourceDesc.Width = nBytes;
//	d3dResourceDesc.Height = 1;
//	d3dResourceDesc.DepthOrArraySize = 1;
//	d3dResourceDesc.MipLevels = 1;
//	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
//	d3dResourceDesc.SampleDesc.Count = 1;
//	d3dResourceDesc.SampleDesc.Quality = 0;
//	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);
//
//	::UpdateSubresources(pd3dCommandList, pd3dTexture, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubresource);
//
//	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
//	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
//	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	d3dResourceBarrier.Transition.pResource = pd3dTexture;
//	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
//	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
//
//	return(pd3dTexture);
//}

ComPtr<ID3D12Resource> CreateTexture2DResource(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	ComPtr<ID3D12Resource> pd3dTexture = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dTextureResourceDesc;
	::ZeroMemory(&d3dTextureResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dTextureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dTextureResourceDesc.Alignment = 0;
	d3dTextureResourceDesc.Width = nWidth;
	d3dTextureResourceDesc.Height = nHeight;
	d3dTextureResourceDesc.DepthOrArraySize = nElements;
	d3dTextureResourceDesc.MipLevels = nMipLevels;
	d3dTextureResourceDesc.Format = dxgiFormat;
	d3dTextureResourceDesc.SampleDesc.Count = 1;
	d3dTextureResourceDesc.SampleDesc.Quality = 0;
	d3dTextureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dTextureResourceDesc.Flags = d3dResourceFlags;

	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dTextureResourceDesc, d3dResourceStates, pd3dClearValue, __uuidof(ID3D12Resource), (void**)&pd3dTexture);

	return(pd3dTexture);
}



