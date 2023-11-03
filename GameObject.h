#pragma once
#include "Mesh.h"
#include "Status.h"


#define RESOURCE_TEXTURE1D			0x01
#define RESOURCE_TEXTURE2D			0x02
#define RESOURCE_TEXTURE2D_ARRAY	0x03	//[]
#define RESOURCE_TEXTURE2DARRAY		0x04
#define RESOURCE_TEXTURE_CUBE		0x05
#define RESOURCE_BUFFER				0x06
#define RESOURCE_STRUCTURED_BUFFER	0x07


class Light;
class Player;
class TerrainMap;
class Camera;

// 테스트 용
static int guid = 0;

struct INSTANCING_FORMAT {
	XMFLOAT4X4 transform;
};

class GameObject : public enable_shared_from_this<GameObject> {
private:
	static ComPtr<ID3D12Resource> pInstanceBuffer;
	static ComPtr<ID3D12Resource>  pInstanceUploadBuffer;
	static D3D12_VERTEX_BUFFER_VIEW instanceBufferView;
	static INSTANCING_FORMAT* pMappedInstance;
public:
	static void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	static const D3D12_VERTEX_BUFFER_VIEW& GetInstanceBufferView();
	static INSTANCING_FORMAT* GetMappedData();
	
protected:
	int gid;
	string name;

	XMFLOAT4X4 worldTransform;
	
	// 부모좌표계 기준
	XMFLOAT4X4 localTransform;
	XMFLOAT3 localPosition;
	XMFLOAT3 localScale;
	XMFLOAT4 localRotation;

	// 물체가 가지고 있는 빛의 포인터
	shared_ptr<Light> pLight;


	BoundingOrientedBox boundingBox;

	// true일경우 하위 오브젝트들을 모두 포함하는 바운딩박스를 가짐
	bool isOOBBCover;
	BoundingOrientedBox baseOrientedBox;
	
	
	weak_ptr<GameObject> pParent;
	vector<shared_ptr<GameObject>> pChildren;
		
	weak_ptr<Mesh> pMesh;

	
	vector<shared_ptr<Material>> materials;

public:
	friend RigidBody;
	
	GameObject();
	virtual ~GameObject();
	// 게임 오브젝트 복사 생성자
	//
	virtual void Create();
	virtual void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

	// get set 함수
	const string& GetName() const;
	// 부모좌표계기준 벡터들을 얻는다.
	XMFLOAT3 GetLocalRightVector() const;
	XMFLOAT3 GetLocalUpVector() const;
	XMFLOAT3 GetLocalLookVector() const;
	XMFLOAT4 GetLocalRotate() const;
	XMFLOAT3 GetLocalPosition() const;
	// 로컬 이동
	void MoveRight(float distance);
	void Move(const XMFLOAT3& _moveVector, float _timeElapsed = 1.0f);
	void MoveUp(float distance);
	void MoveFront(float distance);

	void Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed = 1.0f);
	void Rotate(const XMFLOAT4& _quat);
	// 월드좌표계 기준 자신의 위치를 리턴한다.
	XMFLOAT3 GetWorldPosition() const;
	XMFLOAT3 GetWorldRightVector() const;
	XMFLOAT3 GetWorldUpVector() const;
	XMFLOAT3 GetWorldLookVector() const;

	// 자신의 바운딩 박스의 래퍼런스를 리턴한다.
	const BoundingOrientedBox& GetBoundingBox() const;

	// 해당 인스턴스가 가진 오브젝트의 정보
	shared_ptr<GameObject> GetObj();
	// 위치를 강제로 이동시킨다.
	void SetLocalPosition(const XMFLOAT3& _position);
	// 특정 회전값을 대입한다.
	void SetLocalRotation(const XMFLOAT4& _rotation);
	// 특정 Scale값을 대입한다.
	void SetLocalScale(const XMFLOAT3& _scale);
	// 해당 오브젝트 프레임을 최상위 부모 ( 바운딩박스 커버 )로 설정
	void SetOOBBCover(bool _isCover);


	// 자식을 추가한다.
	void SetChild(const shared_ptr<GameObject> _pChild);
	// 메쉬를 설정한다.
	void SetMesh(const shared_ptr<Mesh>& _pMesh);	
	// 메쉬의 바운딩 박스를 오브젝트로 옮긴다.
	void SetBoundingBox(const BoundingOrientedBox& _box);

	void UpdateLocalTransform();
	// eachTransform를 가지고 worldTransform를 업데이트 한다.
	virtual void UpdateWorldTransform();
	// 변환행렬을 적용하고 worldTransform을 업데이트 한다.

	//  OOBB 갱신
	void UpdateOOBB();
	void MergeOOBB(const shared_ptr<GameObject>& _coverObject);

	// 오브젝트 내용 전체적으로 갱신
	void UpdateObject();

	// 충돌 체크
	void CheckCollision(const shared_ptr<GameObject>& _other);
	//bool CheckCollisionWithTerrain(shared_ptr<TerrainMap> _pTerrain, XMFLOAT3 _moveVector);
	// 애니메이션
	shared_ptr<GameObject> FindFrame(const string& _name);
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void Animate(double _timeElapsed, const XMFLOAT3& _playerPos);
	virtual void Remove();
	virtual bool CheckRemove() const;
	

	// 렌더
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	void RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, list<shared_ptr<GameObject>>& _objList, const D3D12_VERTEX_BUFFER_VIEW& _instanceBV);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	// 월드 변환행렬을 쉐이더로 넘겨준다.
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariableInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index);
	void UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject);
	virtual void CopyObject(const GameObject& _other);
	
};
class FullScreenObject {
public:
	
	ComPtr<ID3D12Resource> pPositionBuffer;	// 화면을 꽉채우는 삼각형 두개로만 되어있다.
	ComPtr<ID3D12Resource> pPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW positionBufferView;

	ComPtr<ID3D12Resource> pTexCoordBuffer;	// 화면을 꽉채우는 삼각형 두개로만 되어있다.
	ComPtr<ID3D12Resource> pTexCoordUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW texCoordBufferView;
	
	FullScreenObject(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~FullScreenObject();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
private:
	
};

class GunShipObject : public GameObject {

public:
	GunShipObject();
	virtual ~GunShipObject();
	shared_ptr<GameObject> rotorFrame;
	shared_ptr<GameObject> tailRotorFrame;
	shared_ptr<GameObject> missileFrame;
	
public:
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
};

class ApacheObject : public GameObject {
public:
	ApacheObject();
	virtual ~ApacheObject();
	shared_ptr<GameObject> rotorFrame;
	shared_ptr<GameObject> rotorCoreFrame;
	shared_ptr<GameObject> tailRotor1Frame;
	shared_ptr<GameObject> tailRotor2Frame;

public:
	virtual void PrepareAnimate();
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
};

class Missile : public GameObject, public RigidBody {
private:
	float lifeTime;
public:
	Missile();
	virtual ~Missile();

public:
	void Create(XMFLOAT3 _position, XMFLOAT4 _rotate, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	bool CheckRemove() const;
	virtual void Animate(double _timeElapsed);
	virtual void CopyObject(const GameObject& _other);
	void Remove();
};

class HeightMapImage {
private:
	BYTE* pHeightMapPixels;

	int							width;
	int							length;
	XMFLOAT3					scale;
	
public:
	HeightMapImage(LPCTSTR _fileName, int _width, int _length, XMFLOAT3 _scale);
	~HeightMapImage();
	float GetHeight(float x, float z, bool bReverseQuad = false);
	float GetWidth() { return (float)width; };
	float GetLength () { return (float)length; };
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(scale); }

	BYTE* GetHeightMapPixels() { return(pHeightMapPixels); }
	int GetHeightMapWidth() { return(width); }
	int GetHeightMapLength() { return(length); }
};



class TerrainMap {

private:
	XMFLOAT4X4 worldTransform;

	shared_ptr<HeightMapImage> pHeightMap;
	shared_ptr<Texture> pTexture;
	
	vector<shared_ptr<TerrainMesh>> pTerrainMeshs;
	
	int width;
	int length;
	
	XMFLOAT3 scale;
	
public:
	
	TerrainMap(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, LPCTSTR _fileName, int _width, int _length, int _blockWidth, int _blockLength, XMFLOAT3 _scale, XMFLOAT4 _color);
	~TerrainMap();
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(pHeightMap->GetHeight(x, z, bReverseQuad) * scale.y); };//World
	float GetWidth() { return pHeightMap->GetWidth(); };
	float GetLength() { return pHeightMap->GetLength(); };
	
	void SetMesh(int _index, shared_ptr<TerrainMesh> _pMesh) { pTerrainMeshs[_index] = _pMesh; };
	void SetTexture(shared_ptr<Texture> _pTexture) { pTexture = _pTexture; };
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
	//void CreateShaderVariables(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	//void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE _cbvGPUDescriptorHandle) { cbvGPUDescriptorStartHandle = _cbvGPUDescriptorHandle; };
};

class Water : public GameObject {
private:
	// ref count 보존용
	shared_ptr<Mesh> ptMesh;

public:
	Water(XMFLOAT3 _size, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Animate(double _timeElapsed);
	~Water();
};

class BillBoard {
	
private:
	string name;
	
	unique_ptr<BillBoardMesh> pMesh;
	shared_ptr<Texture> pTexture;
public:
	BillBoard();
	~BillBoard();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Create(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

class SkyBox {

	array<shared_ptr<SkyBoxMesh>, 6> pMeshs;
	array<shared_ptr<Texture>, 6> pTextures;
public:
	SkyBox();
	~SkyBox();
	
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera);
	void Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera);
};

class Effect {
private:
	// 빌보드 쉐이더를 사용
	static shared_ptr<BillBoardMesh> pMesh;
	static unordered_map<string, shared_ptr<Texture>> pTextures;
public:
	static void CreateBaseMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	static void LoadEffect(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
private:
	// 다음 프레임으로 넘어갈때 걸리는 시간
	string name;
	float nextFrameTime;
	// 스프라이트의 x,y로 있는 프레임의 수
	USHORT xframeCount;
	USHORT yframeCount;
	
	float xsize, ysize;
	XMFLOAT2 startUV;
	XMFLOAT2 endUV;
	// 생존시간 = nextFrameTime * x * y
	float life;
	
	float lifeTime;
	XMFLOAT3 position;
	
public:
	Effect();
	~Effect();
	void Create(float _nextFrameTime, USHORT _x, USHORT _y, float _xs, float _ys, const XMFLOAT3& _pos, const string& _name);
	
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Animate(float _timeElapsed);
	
	bool CheckRemove() const;
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};


class Particle {
private:
	bool isDead = false;
	shared_ptr<ParticleMesh> pMesh;
	shared_ptr<Texture> pTexture;
	shared_ptr<Texture> pRandomValueTexture = NULL;
	shared_ptr<Texture> pRandomValueOnSphereTexture = NULL;
public:

	Particle(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, XMFLOAT3 _position, UINT _nMaxParticles);
	virtual ~Particle();

	void ReleaseUploadBuffers();
	void Animate();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void OnPostRender();
	bool CheckRemove() {  return isDead; };
};


class GameObjectManager {
	unordered_map<string, shared_ptr<GameObject>> storage;

public:
	shared_ptr<GameObject> GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	
};

