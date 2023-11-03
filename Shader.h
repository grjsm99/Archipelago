#pragma once

class Texture;

class Shader {
protected:
	ComPtr<ID3DBlob> pVSBlob, pPSBlob;
	ComPtr<ID3D12PipelineState> pPipelineState;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
	

	ComPtr<ID3D12DescriptorHeap>		cbvSrvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			cbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			cbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			srvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			srvGPUDescriptorNextHandle;
public:
	// 생성 관련 함수들
	Shader();
	virtual ~Shader();
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const wstring& _fileName, const string& _shaderName, const string& _shaderProfile, ComPtr<ID3DBlob>& _pBlob);

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	void CreateCbvSrvDescriptorHeaps(ComPtr<ID3D12Device> _pDevice, int nConstantBufferViews, int nShaderResourceViews);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() = 0;
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() = 0;
	virtual void PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	// CBV 생성
	void CreateConstantBufferView();
	// SRV 생성 (단일)
	void CreateShaderResourceView(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, int _Index);
	// SRV 생성 (다중)
	void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, shared_ptr<Texture> _pTexture, UINT _nDescriptorHeapIndex, UINT _nRootParameterStartIndex);
	
	void CreateShaderResourceViews(ComPtr<ID3D12Device> _pDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);
	
	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	virtual void PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return cbvGPUDescriptorStartHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return srvGPUDescriptorStartHandle; };
	
};

class BasicShader : public Shader {
private:
	ComPtr<ID3D12PipelineState> envPipelineState;
	ComPtr<ID3DBlob> pEnvVSBlob;
	ComPtr<ID3DBlob> pEnvPSBlob;
	
public:
	BasicShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BasicShader();
	virtual void PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;

};

class DepthShader : public Shader {
	public:
	DepthShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~DepthShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class EnvShader : public Shader {
private:
	shared_ptr<Texture> pTexture;
	vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvCPUDescriptorHandles;
public:
	EnvShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature, D3D12_CPU_DESCRIPTOR_HANDLE _rtvCPUHandle);
	virtual ~EnvShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	void SetMultiRenderTarget(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, UINT _nSwapChainBuffer, D3D12_CPU_DESCRIPTOR_HANDLE _rtvCPUHandle, D3D12_CPU_DESCRIPTOR_HANDLE _dsvCPUHandle);
	virtual void PrepareRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	vector<D3D12_CPU_DESCRIPTOR_HANDLE>& GetRtvCpuDescriptorHandles() { return rtvCPUDescriptorHandles; };
	ComPtr<ID3D12Resource> GetRTVResource(int _index);

};

class HitBoxShader : public Shader {

public:
	HitBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~HitBoxShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class TerrainShader : public Shader {
private:
	
	ComPtr<ID3D12PipelineState> envPipelineState;
	ComPtr<ID3DBlob> pEnvVSBlob;
	ComPtr<ID3DBlob> pEnvPSBlob;
public:
	TerrainShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~TerrainShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual void PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class BillBoardShader : public Shader {
	
private:
	// 빌보드는 기하쉐이더를 사용한다.
	ComPtr<ID3DBlob> pGSBlob;
public:
	BillBoardShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~BillBoardShader();
	
	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	
};

class SkyBoxShader : public Shader {
private:
	ComPtr<ID3D12PipelineState> envPipelineState;
	ComPtr<ID3DBlob> pEnvVSBlob;
	ComPtr<ID3DBlob> pEnvPSBlob;
public:
	SkyBoxShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~SkyBoxShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual void PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class InstanceShader : public Shader {


public:
	InstanceShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~InstanceShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
};

class WaterShader : public Shader {
private:
	ComPtr<ID3D12PipelineState> envPipelineState;
	ComPtr<ID3DBlob> pEnvVSBlob;
	ComPtr<ID3DBlob> pEnvPSBlob;
public:
	WaterShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~WaterShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
	virtual void PrepareRenderEnv(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class Shader2D : public Shader {

public:
	Shader2D(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~Shader2D();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;
};

class ParticleShader : public Shader {
private:
	ComPtr<ID3DBlob> pGSBlob;
	// 스트림 출력용 
	ComPtr<ID3DBlob> pVSBlobSO, pGSBlobSO;
	ComPtr<ID3D12PipelineState> pPipelineStateSO;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescSO;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescsSO;
	
public:
	ParticleShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	virtual ~ParticleShader();

	D3D12_RASTERIZER_DESC CreateRasterizerState() final;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() final;
	void PrepareRenderSO(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void InitSO(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	D3D12_STREAM_OUTPUT_DESC CreateStreamOutputState();
	
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() final;
	virtual D3D12_BLEND_DESC CreateBlendState() final;

};