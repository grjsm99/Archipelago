#pragma once

#include "Shader.h"
#include "Material.h"

class GameObject;
class Particle;

class Mesh {
protected:
	static shared_ptr<Shader> shader;
	static shared_ptr<Shader> instanceShader;
	static shared_ptr<Shader> depthShader;
	static shared_ptr<Shader> envShader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static void MakeEnvShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature, D3D12_CPU_DESCRIPTOR_HANDLE _rtvCPUHandle);
	static shared_ptr<Shader> GetShader();
	static shared_ptr<Shader> GetInstanceShader();
	static shared_ptr<Shader> GetDepthShader();
	static shared_ptr<Shader> GetEnvShader();
protected:
	string name;


	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// 버텍스(정점과 노멀벡터)의 개수

	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pNormalBuffer;		// 노멀벡터의 정보
	ComPtr<ID3D12Resource> pNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW normalBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// 텍스처 좌표의 정보
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	vector<UINT> nSubMeshIndex;	// subMesh들의 인덱스 개수
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexBuffers;	// subMesh들의 인덱스 정보
	vector<ComPtr<ID3D12Resource>> pSubMeshIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW> subMeshIndexBufferViews;

	BoundingOrientedBox oobb;

public:		// 생성관련 멤버 함수▼
	// 생성자 및 소멸자
	Mesh();
	virtual ~Mesh();

public:		// 멤버 함수▼

	// get, set함수
	const string& GetName() const;
	const BoundingOrientedBox& GetOOBB() const;
	void LoadFromFile(const string& _fileName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex);
	virtual void RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _subMeshIndex, const D3D12_VERTEX_BUFFER_VIEW& _instanceBufferView, int _numInstance);
};

// 조명까지 계산 하는 메쉬

// boundingBox 메쉬
class HitBoxMesh {
private:
	static shared_ptr<Shader> shader;
	
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// 인덱스 정보
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	HitBoxMesh();
	~HitBoxMesh();
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};


class TerrainMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
protected:

	int width;
	int length;
	XMFLOAT3 scale;

	// OnGetColor에서 노말값으로 색을 미리 계산. 노말대신 색상값을 보낸다 ( 한번만 계산하면 되므로 )

	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertex;	// 버텍스(정점과 노멀벡터)의 개수
	UINT nIndex;	// 인덱스의 개수

	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pColorBuffer;		// 노멀벡터의 정보
	ComPtr<ID3D12Resource> pColorUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW colorBufferView;

	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// uv0의 정보
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

	ComPtr<ID3D12Resource> pTexCoord1Buffer;	// uv1의 정보
	ComPtr<ID3D12Resource> pTexCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord1BufferView;

	ComPtr<ID3D12Resource> pIndexBuffer;	// 인덱스 정보
	ComPtr<ID3D12Resource> pIndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
public:
	TerrainMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _xStart, int _zStart, int _width, int _length, XMFLOAT3 _scale, XMFLOAT4 _color, void* _pContext);
	~TerrainMesh();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	float OnGetHeight(int x, int z, void* pContext);
	XMFLOAT4 OnGetColor(int x, int z, void* pContext);

};

class WaterMesh : public Mesh {
	
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();

public:
	WaterMesh(XMFLOAT3 _scale, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~WaterMesh();
};


class BillBoardMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();

private:
	int nBillboard;
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;

	ComPtr<ID3D12Resource> pPositionBuffer;	// 빌보드의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pSizeBuffer;		// 빌보드의 크기 정보
	ComPtr<ID3D12Resource> pSizeUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW sizeBufferView;

	void* pContext;
public:
	BillBoardMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	BillBoardMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~BillBoardMesh();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class SkyBoxMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	
	ComPtr<ID3D12Resource> pPositionBuffer;
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;


	ComPtr<ID3D12Resource> pTexCoord0Buffer;	// 텍스처 좌표의 정보
	ComPtr<ID3D12Resource> pTexCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoord0BufferView;

public:
	SkyBoxMesh(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~SkyBoxMesh();
	void MakeResource(int _id, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

};
class ParticleMesh {
private:
	static shared_ptr<Shader> shader;
public:
	static void MakeShader(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12RootSignature>& _pRootSignature);
	static shared_ptr<Shader> GetShader();
private:

	
	D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
	UINT nVertices;
	UINT stride;
	
	ComPtr<ID3D12Resource> pPositionBuffer;	// 버텍스의 위치 정보
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;
	
	bool start = true;
	bool end = false;
	UINT maxParticleCount = 100000;
	
	ComPtr<ID3D12Resource> pStreamOutputBuffer;
	ComPtr<ID3D12Resource> pStreamOutputUploadBuffer;
	ComPtr<ID3D12Resource> pDrawBuffer;
	ComPtr<ID3D12Resource> pDrawUploadBuffer;
	
	ComPtr<ID3D12Resource> pDefaultBufferFilledSize;
	ComPtr<ID3D12Resource> pUploadBufferFilledSize;
	 UINT64* nUploadBufferFilledSize = NULL;
	 ComPtr<ID3D12Resource> pReadbackBufferFilledSize;
	
	D3D12_STREAM_OUTPUT_BUFFER_VIEW streamOutputBufferView;
public:
	ParticleMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, XMFLOAT3 _position, UINT nMaxParticles);
	~ParticleMesh();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _nPipelineState);
	void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _nPipelineState);
	void PostRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _nPipelineState);
	void OnPostRender(int _nPipelineState);
	bool IsEnd() { return end; };
};

class ParticleVertex
{
public:
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT3 velocity = XMFLOAT3(0, 0, 0);
	XMFLOAT2 size;
	float lifeTime;
	

public:
	ParticleVertex() { }
	~ParticleVertex() { }
};



/////////////////////////////////////////////////////////////////////////////////////
///	MeshManager
class MeshManager {
public:
	MeshManager();
	~MeshManager();

protected:
	unordered_map<string, shared_ptr<Mesh>> storage;
	HitBoxMesh hitBoxMesh;
public:
	HitBoxMesh& GetHitBoxMesh();
	shared_ptr<Mesh> GetMesh(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _obj);
	void ReleaseUploadBuffers();
};
