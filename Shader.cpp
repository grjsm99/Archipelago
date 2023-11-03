#include "stdafx.h"
#include "Shader.h"
#include "GameFramework.h"

Shader::Shader() {

}


Shader::~Shader() {

}


void Shader::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pipelineStateDesc.pRootSignature = _pRootSignature.Get();
	pipelineStateDesc.RasterizerState = CreateRasterizerState();
	pipelineStateDesc.BlendState = CreateBlendState();
	pipelineStateDesc.InputLayout = CreateInputLayout();
	pipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob) {
	UINT nCompileFlag = 0;

	ComPtr<ID3DBlob> pErrorBlob;
	// 디버그용 플래그	
	nCompileFlag = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	
	HRESULT hResult = D3DCompileFromFile(_fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, _shaderName.c_str(), _shaderProfile.c_str(), nCompileFlag, 0, &_pBlob, &pErrorBlob);
	char* pErrorString = NULL;
	if (pErrorBlob) { 
		pErrorString = (char*)pErrorBlob->GetBufferPointer();
		cout << "바이트 코드 생성 오류 : " << pErrorString << "\n";
	}

	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = _pBlob->GetBufferSize();
	shaderByteCode.pShaderBytecode = _pBlob->GetBufferPointer();

	return shaderByteCode;
}



D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC Shader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}


void Shader::CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index)
{
	// 셰이더 내 디스크립터 힙에 해당 srv를 생성
	ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(_Index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = _pTexture->GetGpuDescriptorHandle(_Index);
	if (pShaderResource && !gpuDescriptorHandle.ptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(_Index);
		
		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
		srvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		_pTexture->SetGpuDescriptorHandle(_Index, srvGPUDescriptorNextHandle);
		srvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

void Shader::CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex)
{
	srvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);
	srvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * _nDescriptorHeapIndex);

	int nTextures = _pTexture->GetnTexture();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = _pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = _pTexture->GetShaderResourceViewDesc(i);
		_pDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
		srvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		
		_pTexture->SetGpuDescriptorHandle(i, srvGPUDescriptorNextHandle);

		srvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = _pTexture->GetnRootParameter();
	for (int i = 0; i < nRootParameters; i++) _pTexture->SetRootParameterIndex(i, _nRootParameterStartIndex + i);
}

void Shader::CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
	for (int i = 0; i < nResources; i++)
	{
		if (ppd3dResources[i])
		{
			auto t = ppd3dResources[i];
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pdxgiSrvFormats[i];
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			// 해당 쉐이더 내에 해당 리소스에 대한 srv 생성
			_pDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, srvCPUDescriptorNextHandle);
			srvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			srvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
}

void Shader::PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pPipelineState) {
		_pCommandList->SetPipelineState(pPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	// 각 쉐이더가 가지고 있는 디스크립터 힙을 연결한다.
	if (cbvSrvDescriptorHeap) {
		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}
void Shader::PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
}

void Shader::PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
}
void Shader::CreateCbvSrvDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	_pDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&cbvSrvDescriptorHeap));

	cbvCPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cbvGPUDescriptorStartHandle = cbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	srvCPUDescriptorStartHandle.ptr = cbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	srvGPUDescriptorStartHandle.ptr = cbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	srvCPUDescriptorNextHandle = srvCPUDescriptorStartHandle;
	srvGPUDescriptorNextHandle = srvGPUDescriptorStartHandle;
}


//////////////////// Basic Shader
BasicShader::BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 50);
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	envPipelineStateDesc.pRootSignature = _pRootSignature.Get();
	envPipelineStateDesc.RasterizerState = CreateRasterizerState();
	envPipelineStateDesc.BlendState = CreateBlendState();
	envPipelineStateDesc.InputLayout = CreateInputLayout();
	envPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

	envPipelineStateDesc.SampleMask = UINT_MAX;
	envPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	envPipelineStateDesc.NumRenderTargets = 1;
	envPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	envPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	envPipelineStateDesc.SampleDesc.Count = 1;
	envPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);

	envPipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "eDefaultVertexShader", "vs_5_1", pEnvVSBlob);
	envPipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShader", "ps_5_1", pEnvPSBlob);

	_hr = _pDevice->CreateGraphicsPipelineState(&envPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&envPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();
	pEnvVSBlob.Reset();
	pEnvPSBlob.Reset();

	inputElementDescs.clear();
}

BasicShader::~BasicShader() {

}


void BasicShader::PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if (pPipelineState) {

		_pCommandList->SetPipelineState(envPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	if (cbvSrvDescriptorHeap) {

		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}



D3D12_RASTERIZER_DESC BasicShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC BasicShader::CreateInputLayout() {
	inputElementDescs.assign(3, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

//////////////////// Depth Shader

DepthShader::DepthShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	// depth buffer용 srv.
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 1);

	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DepthVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DepthPixelShader", "ps_5_1", pPSBlob);
	
	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();
	
	inputElementDescs.clear();
}

DepthShader::~DepthShader() {

}

D3D12_RASTERIZER_DESC DepthShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC DepthShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



D3D12_DEPTH_STENCIL_DESC DepthShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC DepthShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

void DepthShader::PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	Shader::PrepareRender(_pCommandList);

	// 쉐이더 내 힙의 첫 주소의 srv를 연결
	//::SynchronizeResourceTransition(_pCommandList.Get(), , D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
	_pCommandList->SetGraphicsRootDescriptorTable(9, srvGPUDescriptorStartHandle);
}

EnvShader::EnvShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature, D3D12_CPU_DESCRIPTOR_HANDLE _rtvCPUHandle) {
	// 큐브용 srv.
	pTexture = make_shared<Texture>(6, RESOURCE_TEXTURE2D, 0, 1);
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.0f, 1.0f, 1.0f, 1.0f } };
	
	// 텍스처 생성. 렌더타겟과 같은 형태로 6개를 만들어둔다.
	for (int i = 0; i < 6; ++i) {
		pTexture->CreateTexture(_pDevice, 1920, 1080, format, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue, RESOURCE_TEXTURE2D, i);
	}
	 
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 18);
	// 시그니처의 10번을 사용. 
	CreateShaderResourceViews(_pDevice, pTexture, 0, 10);
	
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 렌더타겟뷰 주소를 담을 공간 할당
	rtvCPUDescriptorHandles.resize(6);
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvPtr = _rtvCPUHandle;

	for (UINT i = 0; i < 6; i++)
	{

		// i번째 리소스에 대한 렌더타겟뷰를 생성후 주소를 저장
		ID3D12Resource* pd3dTextureResource = pTexture->GetResource(i).Get();
		
		// 기본 렌더타겟뷰 다음 주소에 바로 만든다.
		if(pd3dTextureResource) _pDevice->CreateRenderTargetView(pd3dTextureResource, &d3dRenderTargetViewDesc, rtvPtr);
		rtvCPUDescriptorHandles[i] = rtvPtr;
		
		rtvPtr.ptr += ::gnRtvDescriptorIncrementSize;
	}

	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "EnvVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "EnvPixelShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

EnvShader::~EnvShader() {
	
}

void EnvShader::SetMultiRenderTarget(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _nSwapChainBuffer, D3D12_CPU_DESCRIPTOR_HANDLE _rtvCPUHandle, D3D12_CPU_DESCRIPTOR_HANDLE _dsvCPUHandle) {
	D3D12_CPU_DESCRIPTOR_HANDLE* allRtvHandle = new D3D12_CPU_DESCRIPTOR_HANDLE[6 + 1];
	for (int i = 0; i < 1; ++i) {
		// nRenderTarget = 1로 고정
		allRtvHandle[i] = _rtvCPUHandle;
		_pCommandList->ClearRenderTargetView(allRtvHandle[i], Colors::White, 0, NULL);
	}

	XMVECTORF32 c[6] = { Colors::Red, Colors::Green, Colors::Blue, Colors::Yellow, Colors::Cyan, Colors::Magenta };
	for (int i = 0; i < 6; i++)
	{
		allRtvHandle[i + 1] = rtvCPUDescriptorHandles[i];
		
		//pd3dCommandList->ClearRenderTargetView(pd3dRtvCPUHandles[i], Colors::White, 0, NULL);
		_pCommandList->ClearRenderTargetView(allRtvHandle[i+1], c[i], 0, NULL);
	}

	_pCommandList->OMSetRenderTargets(1 + 6, allRtvHandle, FALSE, &_dsvCPUHandle);

	if (allRtvHandle) delete[] allRtvHandle;
}

void EnvShader::PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	Shader::PrepareRender(_pCommandList);

	// 쉐이더 내 힙의 첫 주소의 srv를 연결
	//::SynchronizeResourceTransition(_pCommandList.Get(), , D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
	pTexture->UpdateShaderVariable(_pCommandList);
}

ComPtr<ID3D12Resource> EnvShader::GetRTVResource(int _index)
{
	return pTexture->GetResource(_index);
}


D3D12_RASTERIZER_DESC EnvShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC EnvShader::CreateInputLayout() {
	inputElementDescs.assign(3, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}
	

/////////////////// HitBox Shader

HitBoxShader::HitBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	Init(_pDevice, _pRootSignature);
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "HitboxVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "HitboxPixelShader", "ps_5_1", pPSBlob);

	HRESULT h = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

HitBoxShader::~HitBoxShader() {

}

D3D12_RASTERIZER_DESC HitBoxShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC HitBoxShader::CreateInputLayout() {
	inputElementDescs.assign(1, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}

////////////////////// Terrain Shader

TerrainShader::TerrainShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	// 텍스처용 srv 한개( 베이스, 디테일을 같이 묶어서) 사용. 지형의 position값은 CB로 따로 전달함
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 1);
	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	envPipelineStateDesc.pRootSignature = _pRootSignature.Get();
	envPipelineStateDesc.RasterizerState = CreateRasterizerState();
	envPipelineStateDesc.BlendState = CreateBlendState();
	envPipelineStateDesc.InputLayout = CreateInputLayout();
	envPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

	envPipelineStateDesc.SampleMask = UINT_MAX;
	envPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	envPipelineStateDesc.NumRenderTargets = 1;
	envPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	envPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	envPipelineStateDesc.SampleDesc.Count = 1;
	envPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "TerrainVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "TerrainPixelShader", "ps_5_1", pPSBlob);
	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);


	envPipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "eTerrainVertexShader", "vs_5_1", pEnvVSBlob);
	envPipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "TerrainPixelShader", "ps_5_1", pEnvPSBlob);

	_hr = _pDevice->CreateGraphicsPipelineState(&envPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&envPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();
	pEnvVSBlob.Reset();
	pEnvPSBlob.Reset();

	inputElementDescs.clear();
}

TerrainShader::~TerrainShader() {

}


void TerrainShader::PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if (pPipelineState) {

		_pCommandList->SetPipelineState(envPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	if (cbvSrvDescriptorHeap) {

		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}


D3D12_RASTERIZER_DESC TerrainShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC TerrainShader::CreateInputLayout() {
	inputElementDescs.assign(4, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}


BillBoardShader::BillBoardShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	// 텍스처용 SRV 한개씩 사용.  pos값과 size값은 리소스로 만들어 IA로 따로 보냄
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 50);
	
	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.GS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardGeometryShader", "gs_5_1", pGSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "BillboardPixelShader", "ps_5_1", pPSBlob);
	
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	
	pVSBlob.Reset();
	pPSBlob.Reset();
	pGSBlob.Reset();

	inputElementDescs.clear();
}

BillBoardShader::~BillBoardShader() {

}

D3D12_RASTERIZER_DESC BillBoardShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC BillBoardShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});
	
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



D3D12_DEPTH_STENCIL_DESC BillBoardShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC BillBoardShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return blendDesc;
}



SkyBoxShader::SkyBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	// SRV 6개 사용
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 6);

	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	envPipelineStateDesc.pRootSignature = _pRootSignature.Get();
	envPipelineStateDesc.RasterizerState = CreateRasterizerState();
	envPipelineStateDesc.BlendState = CreateBlendState();
	envPipelineStateDesc.InputLayout = CreateInputLayout();
	envPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

	envPipelineStateDesc.SampleMask = UINT_MAX;
	envPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	envPipelineStateDesc.NumRenderTargets = 1;
	envPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	envPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	envPipelineStateDesc.SampleDesc.Count = 1;
	envPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "SkyBoxVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkyBoxPixelShader", "ps_5_1", pPSBlob);



	
	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	
	envPipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "eSkyBoxVertexShader", "vs_5_1", pEnvVSBlob);
	envPipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "SkyBoxPixelShader", "ps_5_1", pEnvPSBlob);
	
	_hr = _pDevice->CreateGraphicsPipelineState(&envPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&envPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();
	pEnvVSBlob.Reset();
	pEnvPSBlob.Reset();

	inputElementDescs.clear();
}

SkyBoxShader::~SkyBoxShader() {

}

D3D12_RASTERIZER_DESC SkyBoxShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC SkyBoxShader::CreateInputLayout() {
	inputElementDescs.assign(2, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

D3D12_BLEND_DESC SkyBoxShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	for (int i = 0; i < 7; ++i) {
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[i].BlendEnable = FALSE;
		blendDesc.RenderTarget[i].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	return blendDesc;
}

void SkyBoxShader::PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if (pPipelineState) {

		_pCommandList->SetPipelineState(envPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}	
	if (cbvSrvDescriptorHeap) {

		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}

/////////// Instance Basic Shader

InstanceShader::InstanceShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {


	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 50);
	
	Init(_pDevice, _pRootSignature);
	
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultVertexShaderInstance", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "DefaultPixelShaderInstance", "ps_5_1", pPSBlob);

	HRESULT h = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);

	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

InstanceShader::~InstanceShader() {

}


D3D12_RASTERIZER_DESC InstanceShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC InstanceShader::CreateInputLayout() {
	inputElementDescs.assign(7, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[4] = { "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[5] = { "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 3, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}


/////////////

WaterShader::WaterShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 2);

	Init(_pDevice, _pRootSignature);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC envPipelineStateDesc;
	ZeroMemory(&envPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	envPipelineStateDesc.pRootSignature = _pRootSignature.Get();
	envPipelineStateDesc.RasterizerState = CreateRasterizerState();
	envPipelineStateDesc.BlendState = CreateBlendState();
	envPipelineStateDesc.InputLayout = CreateInputLayout();
	envPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

	envPipelineStateDesc.SampleMask = UINT_MAX;
	envPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	envPipelineStateDesc.NumRenderTargets = 1;
	envPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	envPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	envPipelineStateDesc.SampleDesc.Count = 1;
	envPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "VertexWaterShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "PixelWaterShader", "ps_5_1", pPSBlob);

	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	

	envPipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "eVertexWaterShader", "vs_5_1", pEnvVSBlob);
	envPipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "PixelWaterShader", "ps_5_1", pEnvPSBlob);

	_hr = _pDevice->CreateGraphicsPipelineState(&envPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&envPipelineState);
	pVSBlob.Reset();
	pPSBlob.Reset();
	pEnvVSBlob.Reset();
	pEnvPSBlob.Reset();

	inputElementDescs.clear();
}

WaterShader::~WaterShader() {

}


D3D12_RASTERIZER_DESC WaterShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	
	// 물안으로 전부 들어갔을때도 보이도록 함
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC WaterShader::CreateInputLayout() {
	inputElementDescs.assign(3, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();
	return inputLayoutDesc;
}


D3D12_DEPTH_STENCIL_DESC WaterShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC WaterShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

void WaterShader::PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if (pPipelineState) {

		_pCommandList->SetPipelineState(envPipelineState.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	if (cbvSrvDescriptorHeap) {

		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}
}


////////////

Shader2D::Shader2D(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {

	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 50);

	Init(_pDevice, _pRootSignature);

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "Vertex2DShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "Pixel2DShader", "ps_5_1", pPSBlob);

	HRESULT h = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	
	pVSBlob.Reset();
	pPSBlob.Reset();

	inputElementDescs.clear();
}

Shader2D::~Shader2D() {

}


D3D12_RASTERIZER_DESC Shader2D::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC Shader2D::CreateInputLayout() {
	inputElementDescs.assign(1, {});

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();
	return inputLayoutDesc;
}


D3D12_DEPTH_STENCIL_DESC Shader2D::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	
	// 2D UI는 항상 맨앞에 그려진다.
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC Shader2D::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

ParticleShader::ParticleShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	// 
	CreateCbvSrvDescriptorHeaps(_pDevice, 0, 10);

	Init(_pDevice, _pRootSignature);
	InitSO(_pDevice, _pRootSignature);
	// 스트림 출력용 파이프라인 설정
	

	pipelineStateDesc.VS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleVertexShader", "vs_5_1", pVSBlob);
	pipelineStateDesc.GS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleGeometryShader", "gs_5_1", pGSBlob);
	pipelineStateDesc.PS = CompileShaderFromFile(L"Shaders.hlsl", "ParticlePixelShader", "ps_5_1", pPSBlob);

	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	
	HRESULT _hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&pPipelineState);
	_hr = _pDevice->CreateGraphicsPipelineState(&pipelineStateDescSO, __uuidof(ID3D12PipelineState), (void**)&pPipelineStateSO);
	pVSBlob.Reset();
	pPSBlob.Reset();
	pGSBlob.Reset();
	pVSBlobSO.Reset();
	pGSBlobSO.Reset();

	inputElementDescs.clear();
}
void ParticleShader::InitSO(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature) {
	ZeroMemory(&pipelineStateDescSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pipelineStateDescSO.pRootSignature = _pRootSignature.Get();
	pipelineStateDescSO.RasterizerState = CreateRasterizerState();
	pipelineStateDescSO.BlendState = CreateBlendState();
	pipelineStateDescSO.InputLayout = CreateInputLayout();
	pipelineStateDescSO.DepthStencilState = CreateDepthStencilState();
	pipelineStateDescSO.StreamOutput = CreateStreamOutputState();

	pipelineStateDescSO.SampleMask = UINT_MAX;
	pipelineStateDescSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	pipelineStateDescSO.NumRenderTargets = 0;
	pipelineStateDescSO.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	pipelineStateDescSO.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	pipelineStateDescSO.SampleDesc.Count = 1;
	pipelineStateDescSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipelineStateDescSO.VS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleSOVertexShader", "vs_5_1", pVSBlobSO);
	pipelineStateDescSO.GS = CompileShaderFromFile(L"Shaders.hlsl", "ParticleSOGeometryShader", "gs_5_1", pGSBlobSO);
}

ParticleShader::~ParticleShader() {

}

D3D12_RASTERIZER_DESC ParticleShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_INPUT_LAYOUT_DESC ParticleShader::CreateInputLayout() {
	inputElementDescs.assign(4, {});
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "TIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	//inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	//inputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = &inputElementDescs[0];
	inputLayoutDesc.NumElements = (UINT)inputElementDescs.size();

	return inputLayoutDesc;
}



D3D12_DEPTH_STENCIL_DESC ParticleShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC ParticleShader::CreateBlendState() {
	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_STREAM_OUTPUT_DESC ParticleShader::CreateStreamOutputState()
{
	D3D12_STREAM_OUTPUT_DESC streamOutputDesc;
	::ZeroMemory(&streamOutputDesc, sizeof(D3D12_STREAM_OUTPUT_DESC));
	UINT nStreamOutputDecls = 4;
	D3D12_SO_DECLARATION_ENTRY* pd3dStreamOutputDecls = new D3D12_SO_DECLARATION_ENTRY[nStreamOutputDecls];
	pd3dStreamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
	pd3dStreamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
	pd3dStreamOutputDecls[2] = { 0, "SIZE", 0, 0, 2, 0 };
	pd3dStreamOutputDecls[3] = { 0, "TIME", 0, 0, 1, 0 };

	UINT* pBufferStrides = new UINT[1];
	pBufferStrides[0] = sizeof(ParticleVertex);

	streamOutputDesc.NumEntries = nStreamOutputDecls;
	streamOutputDesc.pSODeclaration = pd3dStreamOutputDecls;
	streamOutputDesc.NumStrides = 1;
	streamOutputDesc.pBufferStrides = pBufferStrides;
	// 레스터라이저로 넘어가지 않도록 설정
	streamOutputDesc.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	return(streamOutputDesc);
};


void ParticleShader::PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pPipelineState) {
		_pCommandList->SetPipelineState(pPipelineStateSO.Get());
	}
	else {
		cout << "파이프라인 Set 실패\n";
	}
	// 각 쉐이더가 가지고 있는 디스크립터 힙을 연결한다.
	if (cbvSrvDescriptorHeap) {
		_pCommandList->SetDescriptorHeaps(1, cbvSrvDescriptorHeap.GetAddressOf());
	}

}
