#include "stdafx.h"
#include "GameObject.h"
#include "Light.h"
#include "GameFramework.h"


ComPtr<ID3D12Resource> GameObject::pInstanceBuffer;
ComPtr<ID3D12Resource>  GameObject::pInstanceUploadBuffer;
D3D12_VERTEX_BUFFER_VIEW  GameObject::instanceBufferView;

INSTANCING_FORMAT* GameObject::pMappedInstance;

shared_ptr<BillBoardMesh> Effect::pMesh;
unordered_map<string, shared_ptr<Texture>> Effect::pTextures;
//////////////////////////////////////////


void GameObject::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// 인스턴스 정보를 담는 리소스를 만들어 Map
	pInstanceBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, sizeof(INSTANCING_FORMAT) * MAX_INSTANCE, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pInstanceUploadBuffer);
	instanceBufferView.BufferLocation = pInstanceBuffer->GetGPUVirtualAddress();
	instanceBufferView.StrideInBytes = sizeof(INSTANCING_FORMAT);
	instanceBufferView.SizeInBytes = sizeof(INSTANCING_FORMAT) * MAX_INSTANCE;

	//pInstanceBuffer->Map(0, NULL, (void**)&pMappedInstance);

}

const D3D12_VERTEX_BUFFER_VIEW& GameObject::GetInstanceBufferView() {
	return instanceBufferView;
}

INSTANCING_FORMAT* GameObject::GetMappedData() {
	return pMappedInstance;
}

GameObject::GameObject() {
	name = "unknown";
	worldTransform = Matrix4x4::Identity();
	localTransform = Matrix4x4::Identity();
	localPosition = XMFLOAT3(0, 0, 0);
	localRotation = XMFLOAT4(0, 0, 0, 1);
	localScale = XMFLOAT3(1,1,1);
	boundingBox = BoundingOrientedBox();
	isOOBBCover = false;
	gid = guid++;
}
GameObject::~GameObject() {

}


void GameObject::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	GameObject::Create();
	shared_ptr<GameObject> temp = gameFramework.GetGameObjectManager().GetGameObject(_ObjectName, _pDevice, _pCommandList);
	
	// 인스턴스의 자식으로 그 오브젝트의 정보를 설정
	if(temp) SetChild(temp);
}

void GameObject::Create() {

}

const string& GameObject::GetName() const {
	return name;
}

XMFLOAT3 GameObject::GetLocalRightVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(1, 0, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalUpVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 1, 0);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalLookVector() const {
	XMFLOAT3 rightVector = XMFLOAT3(0, 0, 1);
	rightVector = Vector3::Transform(rightVector, Matrix4x4::RotateQuaternion(localRotation));
	return rightVector;
}
XMFLOAT3 GameObject::GetLocalPosition() const {
	return localPosition;
}
XMFLOAT4 GameObject::GetLocalRotate() const {
	return localRotation;
}

void GameObject::MoveRight(float distance) {
	XMFLOAT3 moveVector = GetLocalRightVector();	// RightVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}

void GameObject::Move(const XMFLOAT3& _moveVector, float _timeElapsed) {
	localPosition = Vector3::Add(localPosition, _moveVector);
}

void GameObject::MoveUp(float distance) {
	XMFLOAT3 moveVector = GetLocalUpVector();	// UpVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::MoveFront(float distance) {
	XMFLOAT3 moveVector = GetLocalLookVector();	// LookVector를 가져와서
	moveVector = Vector3::Normalize(moveVector);	// 단위벡터로 바꾼후
	moveVector = Vector3::ScalarProduct(moveVector, distance);	// 이동거리만큼 곱해준다.
	localPosition = Vector3::Add(localPosition, moveVector);
}
void GameObject::Rotate(const XMFLOAT3& _axis, float _angle, float _timeElapsed) {
	localRotation = Vector4::QuaternionMultiply(localRotation, Vector4::QuaternionRotation(_axis, _angle * _timeElapsed));
}

void GameObject::Rotate(const XMFLOAT4& _quat) {

	localRotation = Vector4::QuaternionMultiply(localRotation, _quat);
}

XMFLOAT3 GameObject::GetWorldRightVector() const {
	return Vector3::Normalize(worldTransform._11, worldTransform._12, worldTransform._13);
}
XMFLOAT3 GameObject::GetWorldUpVector() const {
	return Vector3::Normalize(worldTransform._21, worldTransform._22, worldTransform._23);
}
XMFLOAT3 GameObject::GetWorldLookVector() const {
	return Vector3::Normalize(worldTransform._31, worldTransform._32, worldTransform._33);
}
XMFLOAT3 GameObject::GetWorldPosition() const {
	return XMFLOAT3(worldTransform._41, worldTransform._42, worldTransform._43);
}

const BoundingOrientedBox& GameObject::GetBoundingBox() const {
	return boundingBox;
}

shared_ptr<GameObject> GameObject::GetObj() {
	if (!pParent.lock()) return pChildren[0];
	else {
		cout << "메서드가 호출된 오브젝트는 인스턴스가 아닙니다.\n";
		return shared_from_this();
	}
}
void GameObject::SetLocalPosition(const XMFLOAT3& _position) {
	localPosition = _position;
}

void GameObject::SetLocalRotation(const XMFLOAT4& _rotation) {
	localRotation = _rotation;
}

void GameObject::SetLocalScale(const XMFLOAT3& _scale) {
	localScale = _scale;
}

void GameObject::SetOOBBCover(bool _isCover) {
	isOOBBCover = _isCover;
}

void GameObject::SetChild(const shared_ptr<GameObject> _pChild) {
	// 입양할 아이가, 부모가 있을 경우 부모로 부터 독립시킨다.
	if (auto pPreParent = _pChild->pParent.lock()) {
		pPreParent->pChildren.erase(ranges::find(pPreParent->pChildren, _pChild));
	}

	// 나의 자식으로 입양
	pChildren.push_back(_pChild);

	// 자식의 부모를 나로 지정
	_pChild->pParent = shared_from_this();
}



void GameObject::SetMesh(const shared_ptr<Mesh>& _pMesh) {
	pMesh = _pMesh;
}

void GameObject::SetBoundingBox(const BoundingOrientedBox& _box) {
	baseOrientedBox = _box;
}

void GameObject::UpdateLocalTransform() {
	localTransform = Matrix4x4::Identity();
	// S

	localTransform._11 = localScale.x;
	localTransform._22 = localScale.y;
	localTransform._33 = localScale.z;
	// SxR
	localTransform = Matrix4x4::Multiply(localTransform, Matrix4x4::RotateQuaternion(localRotation));
	// xT
	localTransform._41 = localPosition.x;
	localTransform._42 = localPosition.y;
	localTransform._43 = localPosition.z;

}
void GameObject::UpdateWorldTransform() {
	//UpdateLocalTransform();

	if (auto pParentLock = pParent.lock()) {	// 부모가 있을 경우
		worldTransform = Matrix4x4::Multiply(localTransform, pParentLock->worldTransform);
	}
	else {	// 부모가 없을 경우
		worldTransform = localTransform;
	}

	// 자식들도 worldTransform을 업데이트 시킨다.
	for (auto& pChild : pChildren) {
		pChild->UpdateWorldTransform();
	}
}



void GameObject::UpdateOOBB() {
	
	if (isOOBBCover) {
		XMFLOAT3 tmp = baseOrientedBox.Center;
		baseOrientedBox.Transform(boundingBox, XMLoadFloat4x4(&worldTransform));

		XMStoreFloat4(&boundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&boundingBox.Orientation)));


	}
	for (const auto& pChild : pChildren) {
		pChild->UpdateOOBB();
	}
}

void GameObject::MergeOOBB(const shared_ptr<GameObject>& _coverObject) {
	if (!isOOBBCover) {

		BoundingOrientedBox& coverBox = _coverObject->baseOrientedBox;
		if (GetName() == "GameObject_Gunship") {
			// gunship의 경우 boundingBox의 중심이 원점이 아닌 0 1.012005 -4.939685이므로 조정해줌
			coverBox.Center = pMesh.lock()->GetOOBB().Center;
		}
		// 두 OOBB의 거리차를 구함
		XMFLOAT3 gapPosition = Vector3::Subtract(_coverObject->GetWorldPosition(), GetWorldPosition());

		XMFLOAT3 worldCenter = Vector3::Add(baseOrientedBox.Center, gapPosition);
		XMFLOAT3 distance = Vector3::Subtract(worldCenter, coverBox.Center);

		coverBox.Extents.x = max(abs(distance.x) + baseOrientedBox.Extents.x, coverBox.Extents.x);
		coverBox.Extents.y = max(abs(distance.y) + baseOrientedBox.Extents.y, coverBox.Extents.y);
		coverBox.Extents.z = max(abs(distance.z) + baseOrientedBox.Extents.z, coverBox.Extents.z);
	}
	for (const auto& pChild : pChildren) {
		pChild->MergeOOBB(_coverObject);
	}
}

void GameObject::UpdateObject() {
	UpdateLocalTransform();
	UpdateWorldTransform();
	UpdateOOBB();
}

void GameObject::CheckCollision(const shared_ptr<GameObject>& _other) {
	// 미사용
	if (isOOBBCover) {
		if (_other->isOOBBCover)
		{ 
			if (boundingBox.Intersects(_other->boundingBox)) {
				//CollideReact(shared_from_this(), _other);
			}
			return;
		}
	}

	// 둘다 Cover일때까지 내려감
	for (const auto& pChild : _other->pChildren) {
		CheckCollision(pChild);
	}
	for (const auto& pChild : pChildren) {
		pChild->CheckCollision(_other);
	}
}

shared_ptr<GameObject> GameObject::FindFrame(const string& _name) {
	if (name == _name) {
		return shared_from_this();
	}
	for (const auto& pChild : pChildren) {
		if (auto pFound = pChild->FindFrame(_name)) {
			return pFound;
		}
	}
	return nullptr;
}

void GameObject::PrepareAnimate() {
	
}

void GameObject::Animate(double _timeElapsed) {
	for (const auto& pChild : pChildren) {
		pChild->Animate(_timeElapsed);
	}
}

void GameObject::Animate(double _timeElapsed, const XMFLOAT3& _playerPos) {

};
void GameObject::Remove() {
};
bool GameObject::CheckRemove() const {
	return false;
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	if (pMesh.lock()) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		UpdateShaderVariable(_pCommandList);
		
		// 각 마테리얼에 맞는 서브메쉬를 그린다.
		for (int i = 0; i < materials.size(); ++i) {
			// 해당 서브매쉬와 매칭되는 마테리얼을 Set 해준다.

			materials[i]->UpdateShaderVariable(_pCommandList); 
			pMesh.lock()->Render(_pCommandList, i);
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->Render(_pCommandList);
	}
}

void GameObject::RenderInstancing(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, list<shared_ptr<GameObject>>& _objList, const D3D12_VERTEX_BUFFER_VIEW& _instanceBV) {
	// 미사용
	
	// 각 프레임마다 그 프레임이 가진 월드변환 행렬을 갱신해준다.

	if (pMesh.lock()) {
		// UpdateShaderVariable로 월드행렬을 전달하는것이 아닌, 리소스로 전달 (IA)

		/*int i = 0;
		for (auto& pObj : _objList) {
			shared_ptr<GameObject> cur_frame = pObj->FindFrame(name);
			cout << cur_frame->GetName() << "\n";
			cur_frame->UpdateShaderVariableInstance(_pCommandList, i++);
		}*/
		
		// 각 서브메쉬에 대한 DP call을 한번씩 해준다.
		for (int i = 0; i < materials.size(); ++i) {
			materials[i]->UpdateShaderVariable(_pCommandList);
			pMesh.lock()->RenderInstancing(_pCommandList, i, _instanceBV, _objList.size());
		}
	}
	for (const auto& pChild : pChildren) {
		pChild->RenderInstancing(_pCommandList, _objList, _instanceBV);
	}
}


void GameObject::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox) {
	
	if (isOOBBCover) {	// 메쉬가 있을 경우에만 렌더링을 한다.
		UpdateHitboxShaderVariable(_pCommandList);
		// 사용할 쉐이더의 그래픽스 파이프라인을 설정한다 [수정요망]
		_hitBox.Render(_pCommandList);
	}
	for (const auto& pChild : pChildren) {
		pChild->RenderHitBox(_pCommandList, _hitBox);
	}
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void GameObject::UpdateShaderVariableInstance(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index) {
	INSTANCING_FORMAT* pInstancing = GameObject::GetMappedData();
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	memcpy(&pInstancing[_index].transform, &world, sizeof(INSTANCING_FORMAT));
}


void GameObject::UpdateHitboxShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	XMFLOAT4X4 world = Matrix4x4::ScaleTransform(Vector3::ScalarProduct(baseOrientedBox.Extents, 1.0f));
	XMFLOAT4X4 translate = Matrix4x4::Identity();
	translate._41 += baseOrientedBox.Center.x;
	translate._42 += baseOrientedBox.Center.y;
	translate._43 += baseOrientedBox.Center.z;
	// 바운딩박스의 extents, center값 만큼 이동 후 변환 행렬 적용 ( 공전 )
	world = Matrix4x4::Multiply(Matrix4x4::Multiply(world, translate), worldTransform);

	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	//world = Matrix4x4::Identity();
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
	
}



void GameObject::LoadFromFile(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, const shared_ptr<GameObject>& _coverObject) {
	GameFramework& gameFramework = GameFramework::Instance();

	// nameSize (UINT) / name(string)
	ReadStringBinary(name, _file);

	// localTransform(float3, 3, 4)
	_file.read((char*)&localPosition, sizeof(XMFLOAT3));
	_file.read((char*)&localScale, sizeof(XMFLOAT3));
	_file.read((char*)&localRotation, sizeof(XMFLOAT4));
	UpdateLocalTransform();

	string meshFileName;
	// meshNameSize(UINT) / meshName(string)
	ReadStringBinary(meshFileName, _file);
	
	// 메시가 없을경우 스킵
	if (meshFileName.size() != 0) {

		pMesh = gameFramework.GetMeshManager().GetMesh(meshFileName, _pDevice, _pCommandList, shared_from_this());
		if (pMesh.lock()->GetName().contains("body")) {
			_coverObject->SetBoundingBox(boundingBox);
		}
		// 마테리얼 정보
		// nMaterial (UINT)
		UINT nMaterial = 0;
		_file.read((char*)&nMaterial, sizeof(UINT));
		materials.resize(nMaterial);

		// 마테리얼들을 불러옴
		for (auto& mat : materials) {
			mat = make_shared<Material>();
			mat->LoadMaterial(_file, _pDevice, _pCommandList);
		}
	}
	int nChildren;
	_file.read((char*)&nChildren, sizeof(int));
	pChildren.reserve(nChildren);

	for (int i = 0; i < nChildren; ++i) {
		shared_ptr<GameObject> newObject = make_shared<GameObject>();
		newObject->LoadFromFile(_file, _pDevice, _pCommandList, _coverObject);
		SetChild(newObject);
	}
}

void GameObject::CopyObject(const GameObject& _other) {
	name = _other.name;
	worldTransform = _other.worldTransform;
	localTransform = _other.localTransform;
	boundingBox = _other.boundingBox;
	baseOrientedBox = _other.baseOrientedBox;
	isOOBBCover = _other.isOOBBCover;
	localPosition = _other.localPosition;
	localScale = _other.localScale;
	localRotation = _other.localRotation;
	pMesh = _other.pMesh;
	materials = _other.materials;

	for (int i = 0; i < _other.pChildren.size(); ++i) {
		shared_ptr<GameObject> child = make_shared<GameObject>();
		child->CopyObject(*_other.pChildren[i]);
		SetChild(child);
	}
}

/////////////// FullScreenObject

FullScreenObject::FullScreenObject(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	
	
	vector<XMFLOAT3> pos(6);
	vector<XMFLOAT2> uv(6);
	pos[0] = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	pos[1] = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	pos[2] = XMFLOAT3(1.0f, -1.0f, 0.0f);
	pos[3] = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	pos[4] = XMFLOAT3(1.0f, -1.0f, 0.0f);
	pos[5] = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	
	uv[0] = XMFLOAT2(0.0f, 0.0f);
	uv[1] = XMFLOAT2(1.0f, 0.0f);
	uv[2] = XMFLOAT2(1.0f, 1.0f);
	uv[3] = XMFLOAT2(0.0f, 0.0f);
	uv[4] = XMFLOAT2(1.0f, 1.0f);
	uv[5] = XMFLOAT2(0.0f, 1.0f);
	
	pPositionBuffer = CreateBufferResource(_pDevice, _pCommandList, pos.data(), sizeof(XMFLOAT3) * pos.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pPositionUploadBuffer);
	positionBufferView.BufferLocation = pPositionBuffer->GetGPUVirtualAddress();
	positionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	positionBufferView.SizeInBytes = sizeof(XMFLOAT3) * pos.size();
	
	pTexCoordBuffer = CreateBufferResource(_pDevice, _pCommandList, uv.data(), sizeof(XMFLOAT2) * uv.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, pTexCoordUploadBuffer);
	texCoordBufferView.BufferLocation = pTexCoordBuffer->GetGPUVirtualAddress();
	texCoordBufferView.StrideInBytes = sizeof(XMFLOAT2);
	texCoordBufferView.SizeInBytes = sizeof(XMFLOAT2) * uv.size();
	
	
}

FullScreenObject::~FullScreenObject() {

}

void FullScreenObject::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	
	_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2] = { positionBufferView, texCoordBufferView };
	_pCommandList->IASetVertexBuffers(0, 2, vertexBufferViews);
	_pCommandList->DrawInstanced(6, 1, 0, 0);
}

////// GunshipObject , ApacheObject ////// 

GunShipObject::GunShipObject() {
	
}

GunShipObject::~GunShipObject() {

}

void GunShipObject::PrepareAnimate() {
	rotorFrame = FindFrame("GameObject_Rotor");
	tailRotorFrame = FindFrame("GameObject_Back_Rotor");
//	missileFrame = FindFrame("")
}

void GunShipObject::Animate(double _fTimeElapsed) {

	if (rotorFrame) {
		rotorFrame->Rotate(rotorFrame->GetLocalUpVector(), 1440.0f * _fTimeElapsed);
		rotorFrame->UpdateObject();

	}
	if (tailRotorFrame) {
		tailRotorFrame->Rotate(tailRotorFrame->GetLocalRightVector(), 1440.0f * _fTimeElapsed);
		tailRotorFrame->UpdateObject();
	}
}

void GunShipObject::CopyObject(const GameObject& _other) {
	GunShipObject& tmp = (GunShipObject&)_other;
	
	rotorFrame = tmp.rotorFrame;
	tailRotorFrame = tmp.tailRotorFrame;

	GameObject::CopyObject(_other);

	PrepareAnimate();
}


ApacheObject::ApacheObject() {

}

ApacheObject::~ApacheObject() {

}

void ApacheObject::PrepareAnimate() {
	rotorFrame = FindFrame("GameObject_rotor");

	tailRotor1Frame = FindFrame("GameObject_black m 6");
	tailRotor2Frame = FindFrame("GameObject_black m 7");
	rotorCoreFrame = FindFrame("GameObject_black m 8");
	//	missileFrame = FindFrame("")
}

void ApacheObject::Animate(double _fTimeElapsed) {
	if (rotorFrame) {
		rotorFrame->Rotate(rotorFrame->GetLocalUpVector(), 1440.0f * _fTimeElapsed);
		rotorFrame->UpdateObject();
	}
	if (tailRotor1Frame) {
		tailRotor1Frame->Rotate(XMFLOAT3(1,0,0), 1440.0f * _fTimeElapsed);
		tailRotor1Frame->UpdateObject();
	}
	if (tailRotor2Frame) {
		tailRotor2Frame->Rotate(XMFLOAT3(1, 0, 0), 1440.0f * _fTimeElapsed);
		tailRotor2Frame->UpdateObject();
	}
	if (rotorCoreFrame) {
		rotorCoreFrame->Rotate(rotorFrame->GetLocalUpVector(), 1440.0f * _fTimeElapsed);
		rotorCoreFrame->UpdateObject();
	}
}

void ApacheObject::CopyObject(const GameObject& _other) {
	ApacheObject& tmp = (ApacheObject&)_other;

	rotorFrame = tmp.rotorFrame;
	tailRotor1Frame = tmp.tailRotor1Frame;

	GameObject::CopyObject(_other);

	PrepareAnimate();
}

////// MissileObject //////

Missile::Missile() {
	lifeTime = 2.0f;
}

Missile::~Missile() {
	
}
void Missile::Create(XMFLOAT3 _position, XMFLOAT4 _rotate, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	
	name = "Missile";
	GameObject::Create("Missile", _pDevice, _pCommandList);
	self = shared_from_this();
	SetLocalScale(XMFLOAT3(3,3,3));
	
	//SetChild(gameFramework.GetGameObjectManager().GetGameObject(, _pDevice, _pCommandList));
	moveSpeed = 0.0f;
	XMFLOAT3 nPos = _position;
	//nPos.y -= 5.0f;
	SetLocalPosition(nPos);
	SetLocalRotation(_rotate);
	UpdateObject();
}

bool Missile::CheckRemove() const {
	return lifeTime <= 0.0f;
}

void Missile::Animate(double _timeElapsed) {
	lifeTime -= _timeElapsed;
	moveSpeed += 2.0f * _timeElapsed;
	MoveFrontRigid(true, _timeElapsed);

	if(!CheckCollisionWithTerrain(shared_from_this()))
	{
		Rotate(rotateAxis, rotateAngle, _timeElapsed);
		Move(moveVector, _timeElapsed);
	}
	if (moveVector.x != 0 || moveVector.y != 0 || moveVector.z != 0 || rotateAngle != 0.0f)
		UpdateObject();
	InitVector();
}

void Missile::CopyObject(const GameObject& _other) {
	Missile& tmp = (Missile&)_other;

	GameObject::CopyObject(_other);
	PrepareAnimate();
}

void Missile::Remove() {
	lifeTime = 0.0f;
}


//////////// HeightMapImage, TerrainMap ////////////


HeightMapImage::HeightMapImage(LPCTSTR _fileName, int _width, int _length, XMFLOAT3 _scale) {

	width = _width;
	length = _length;
	scale = _scale;
	BYTE* ptemp = new BYTE[width * length];

	ifstream in(_fileName, ios::binary);
	in.read((char*)ptemp, (width * length));

	pHeightMapPixels = new BYTE[width * length];
	for (int y = 0; y < length; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pHeightMapPixels[x + ((length - 1 - y) * width)] = ptemp[x + (y * width)];
		}
	}

	if (ptemp) delete[] ptemp;
}

HeightMapImage::~HeightMapImage() {
	if (pHeightMapPixels) delete[] pHeightMapPixels;
	pHeightMapPixels = NULL;
}


float HeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / scale.x;
	fz = fz / scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= width) || (fz >= length)) return(INT_MAX);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)pHeightMapPixels[x + (z * width)];
	float fBottomRight = (float)pHeightMapPixels[(x + 1) + (z * width)];
	float fTopLeft = (float)pHeightMapPixels[x + ((z + 1) * width)];
	float fTopRight = (float)pHeightMapPixels[(x + 1) + ((z + 1) * width)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}


XMFLOAT3 HeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= width) || (z >= length)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * width);
	int xHeightMapAdd = (x < (width - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (length - 1)) ? width : -width;
	float y1 = (float)pHeightMapPixels[nHeightMapIndex] * scale.y;
	float y2 = (float)pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * scale.y;
	float y3 = (float)pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

TerrainMap::TerrainMap(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, LPCTSTR _fileName, int _width, int _length, int _blockWidth, int _blockLength, XMFLOAT3 _scale, XMFLOAT4 _color) {
	
	worldTransform = Matrix4x4::Identity();
	GameFramework gameFramework = GameFramework::Instance();

	width = _width;
	length = _length;

	int cxQuadsPerBlock = _blockWidth - 1;
	int czQuadsPerBlock = _blockLength - 1;

	scale = _scale;

	pHeightMap = make_shared<HeightMapImage>(_fileName, width, length, scale);
	
	long cxBlocks = (width - 1) / cxQuadsPerBlock;
	long czBlocks = (length - 1) / czQuadsPerBlock;
	
	// 격자 메쉬가 들어갈 공간 할당
	pTerrainMeshs.assign(cxBlocks * czBlocks, {});
	
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (_blockWidth - 1);
			zStart = z * (_blockLength - 1);
			shared_ptr<TerrainMesh> terrainMesh = make_shared<TerrainMesh>(_pDevice, _pCommandList, xStart, zStart, _blockWidth, _blockLength, _scale, _color, pHeightMap.get());
			SetMesh(x + (z * cxBlocks), terrainMesh);

		}
	}


	// 텍스처 두개, 루트 파라미터는 1개를 사용
	shared_ptr< Texture> pTerrainTexture = make_shared<Texture>(2, RESOURCE_TEXTURE2D, 0, 1);
	
	//  각 베이스, 디테일 텍스처
	pTerrainTexture->LoadFromFile("Base_Texture", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 5);

	pTerrainTexture->LoadFromFile("Detail_Texture_7", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 1, 5);

	shared_ptr<Shader> pTerrainShader = TerrainMesh::GetShader();

	// 지형쉐이더에 SRV를 생성해서 넣어줌
	pTerrainShader->CreateShaderResourceViews(_pDevice, pTerrainTexture, 0, 5);

	SetTexture(pTerrainTexture);

}

TerrainMap::~TerrainMap() {

}


void TerrainMap::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void TerrainMap::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (pTerrainMeshs.size() > 0) {
		
		// 지형의 기본 월드 행렬값을 업데이트
		UpdateShaderVariable(_pCommandList);
		// 텍스처가 있다면 업데이트 한다.
		if (pTexture) {
				pTexture->UpdateShaderVariable(_pCommandList);
				// 지형 메쉬 분할이 없을때만 0번인덱스만 Render.
				pTerrainMeshs[0]->Render(_pCommandList);
		}
		
	}
}

/////////// Water ///////////////

Water::Water(XMFLOAT3 _size, float _height, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	name = "water";
	GameFramework& gameFramework = GameFramework::Instance();

	ptMesh = make_shared<WaterMesh>(_size, _height, _pDevice, _pCommandList);
	pMesh = ptMesh;

	shared_ptr<Texture> pTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadFromFile("water", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);

	materials.resize(1);
	materials[0] = make_shared<Material>();
	materials[0]->DefaultMaterial(_pDevice, _pCommandList);
	materials[0]->SetTexture(pTexture);
	
	shared_ptr<Shader> pWaterShader = WaterMesh::GetShader();
	pWaterShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void Water::Animate(double _timeElapsed) {
	worldTransform._41 += _timeElapsed * 0.2f;
	worldTransform._42 += _timeElapsed * 0.08f;
}

Water::~Water() {
	
}
//////////// BillBoard ////////////////

BillBoard::BillBoard() {

}

BillBoard::~BillBoard() {

}

void BillBoard::Create(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();

	pTexture = gameFramework.GetTextureManager().GetTexture(_name, _pDevice, _pCommandList);
	pMesh = make_unique<BillBoardMesh>(_name, _pDevice, _pCommandList);
	name = _name;
	shared_ptr<Shader> pBillBoardShader = BillBoardMesh::GetShader();

	// 빌보드 쉐이더의 힙 내에 텍스처 SRV를 생성해서 넣어줌
	pBillBoardShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void BillBoard::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	
	if (pMesh) {
		if (pTexture) {
			pTexture->UpdateShaderVariable(_pCommandList);
		}
		pMesh->Render(_pCommandList);
		
	}
}

///////////// SkyBox ////////////////

SkyBox::SkyBox() {

}

SkyBox::~SkyBox() {

}



//////////// Effect //////////

Effect::Effect() {

}

Effect::~Effect() {

}

void Effect::CreateBaseMesh(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	pMesh = make_shared<BillBoardMesh>(_pDevice, _pCommandList);
}

void Effect::LoadEffect(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	shared_ptr<Texture> pTexture = gameFramework.GetTextureManager().GetTexture(_name, _pDevice, _pCommandList);
	pTextures[_name] = pTexture;
	
	shared_ptr<Shader> pBillBoardShader = BillBoardMesh::GetShader();
	pBillBoardShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
}

void Effect::Create(float _nextFrameTime, USHORT _x, USHORT _y, float _xs, float _ys, const XMFLOAT3& _pos, const string& _name) {
	name = _name;
	xframeCount = _x;
	yframeCount = _y;
	xsize = _xs;
	ysize = _ys;
	nextFrameTime = _nextFrameTime;
	lifeTime = nextFrameTime * xframeCount * yframeCount;
	life = lifeTime;
	position = _pos;
}

void Effect::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	UpdateShaderVariable(_pCommandList);
	if (pMesh) {
		if (pTextures[name]) {
			pTextures[name]->UpdateShaderVariable(_pCommandList);
		}
		pMesh->Render(_pCommandList);
	}
}

void Effect::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 world = Matrix4x4::Identity();
	world._12 = startUV.x;
	world._13 = startUV.y;
	
	world._11 = endUV.x;
	world._22 = endUV.y;

	world._33 = xsize;
	world._44 = ysize;
	
	world._41 = position.x;
	world._42 = position.y;
	world._43 = position.z;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

void Effect::Animate(float _timeElapsed) {
	// lifeTime = life일때는 0프레임, lifeTime = 0일때 xsize * ysize - 1프레임
	USHORT nowFrame = (USHORT)((life - lifeTime) / nextFrameTime);
	lifeTime -= _timeElapsed;
	startUV.x = (nowFrame % xframeCount) / (float)xframeCount;
	startUV.y = (nowFrame / yframeCount) / (float)yframeCount;
	endUV.x = startUV.x + 1.0f / xframeCount;
	endUV.y = startUV.y + 1.0f / yframeCount;
}

bool Effect::CheckRemove() const {
	return lifeTime < 0.0f;
}

void SkyBox::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// 큐브 오브젝트 리소스 생성
	for (int i = 0; i < 6; i++) {
		pMeshs[i] = make_shared<SkyBoxMesh>(i, _pDevice, _pCommandList);
		pTextures[i] = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	}
	// 앞뒤, 상하, 왼오
	pTextures[0]->LoadFromFile("SkyBox_Back_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[1]->LoadFromFile("SkyBox_Front_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[2]->LoadFromFile("SkyBox_Top_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[3]->LoadFromFile("SkyBox_Bottom_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[4]->LoadFromFile("SkyBox_Left_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	pTextures[5]->LoadFromFile("SkyBox_Right_0", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);
	//pTextures[5]->LoadFromFile("Flower02", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);

	//GameFramework gameFramework = gameFramework.Instance();
	
	shared_ptr<Shader> pSkyBoxShader = SkyBoxMesh::GetShader();
	for (int i = 0; i < 6; i++) {
		pSkyBoxShader->CreateShaderResourceViews(_pDevice, pTextures[i], 0, 4);
	}

	// 스카이박스 쉐이더의 힙 내에 텍스처 SRV를 생성해서 넣어줌
}

void SkyBox::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera) {
	
	// 카메라 포지션을 worldTransform으로 보내준다.
	UpdateShaderVariable(_pCommandList, _pCamera);
	
	for (int i = 0; i < 6; ++i) {
		if (pTextures[i]) {
			pTextures[i]->UpdateShaderVariable(_pCommandList);
			if(pMeshs[i])
				pMeshs[i]->Render(_pCommandList);
		}
	}
}

void SkyBox::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, shared_ptr<Camera> _pCamera) {
	XMFLOAT4X4 world = Matrix4x4::Identity();
	XMFLOAT3 cameraPos = _pCamera->GetWorldPosition();
	world._41 = cameraPos.x;
	world._42 = cameraPos.y;
	world._43 = cameraPos.z;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMLoadFloat4x4(&world)));
	_pCommandList->SetGraphicsRoot32BitConstants(1, 16, &world, 0);
}

/////////////////////////// GameObjectManager /////////////////////
shared_ptr<GameObject> GameObjectManager::GetGameObject(const string& _name, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (!storage.contains(_name)) {	// 처음 불러온 오브젝트일 경우
		shared_ptr<GameObject> newObject;
		if (_name == "Gunship") {
			newObject = make_shared<GunShipObject>();
		}
		else if (_name == "Apache") {
			newObject = make_shared<ApacheObject>();
		}
		else if (_name == "Missile") {
			newObject = make_shared<Missile>();
		}
		else  newObject  = make_shared<GameObject>();
		ifstream file("GameObject/" + _name, ios::binary);	// 파일을 연다
		
		if (!file) {
			cout << "GameObject Load Failed : " << _name << "\n";
			return nullptr;
		}
		
		newObject->SetOOBBCover(true);
		newObject->LoadFromFile(file, _pDevice, _pCommandList, newObject);

		BoundingOrientedBox box;
		file.read((char*)&box.Center, sizeof(XMFLOAT3));
		file.read((char*)&box.Extents, sizeof(XMFLOAT3));
		newObject->SetBoundingBox(box);
		// eachTransfrom에 맞게 각 계층의 오브젝트들의 worldTransform을 갱신
		storage[_name] = newObject;
		newObject->UpdateObject();
	}
	// 스토리지 내 오브젝트 정보와 같은 오브젝트를 복사하여 생성한다.
	shared_ptr<GameObject> Object;
	if (_name == "Gunship") Object = make_shared<GunShipObject>();
	else if (_name == "Apache") Object = make_shared<ApacheObject>();
	else if (_name == "Missile") Object = make_shared<Missile>();

	else  Object = make_shared<GameObject>();
	
	Object->CopyObject(*storage[_name]);
	return Object;
}

Particle::Particle(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, XMFLOAT3 _position,  UINT _nMaxParticles)
{
	pMesh = make_shared<ParticleMesh>(_pDevice, _pCommandList, _position, _nMaxParticles);

	// 베이스 텍스처
	pTexture = make_shared<Texture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadFromFile("RoundSoftParticle", _pDevice, _pCommandList, RESOURCE_TEXTURE2D, 0, 4);

	auto pShader = ParticleMesh::GetShader();
	XMFLOAT4* pxmf4RandomValues = new XMFLOAT4[1024];
	for (int i = 0; i < 1024; i++) { pxmf4RandomValues[i].x = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].y = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].z = float((rand() % 10000) - 5000) / 5000.0f; pxmf4RandomValues[i].w = float((rand() % 10000) - 5000) / 5000.0f; }

	pRandomValueTexture = make_shared<Texture>(1, RESOURCE_BUFFER, 0, 1);
	pRandomValueTexture->CreateBuffer(_pDevice, _pCommandList, pxmf4RandomValues, 1024, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	pRandomValueOnSphereTexture = make_shared<Texture>(1, RESOURCE_TEXTURE1D, 0, 1);
	pRandomValueOnSphereTexture->CreateBuffer(_pDevice, _pCommandList, pxmf4RandomValues, 256, sizeof(XMFLOAT4), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, 0);

	pShader->CreateShaderResourceViews(_pDevice, pTexture, 0, 4);
	pShader->CreateShaderResourceViews(_pDevice, pRandomValueTexture, 0, 6);
	pShader->CreateShaderResourceViews(_pDevice, pRandomValueOnSphereTexture, 0, 7);

	//SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());
	delete[] pxmf4RandomValues;
}


Particle::~Particle() {
	
}

void Particle::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	
	ParticleMesh::GetShader()->PrepareRenderSO(_pCommandList);

	////UpdateShaderVariables(pd3dCommandList);

	if (pMesh) pMesh->PreRender(_pCommandList, 0); //Stream Output
	if (pMesh) pMesh->Render(_pCommandList, 0); //Stream Output
	if (pMesh) pMesh->PostRender(_pCommandList, 0); //Stream Output

	ParticleMesh::GetShader()->PrepareRender(_pCommandList);
	if (pTexture)
	{
		pTexture->UpdateShaderVariable(_pCommandList);
		if (pRandomValueTexture) pRandomValueTexture->UpdateShaderVariable(_pCommandList);
		if (pRandomValueOnSphereTexture) pRandomValueOnSphereTexture->UpdateShaderVariable(_pCommandList);
	}

	if (pMesh) pMesh->PreRender(_pCommandList, 1); //Draw
	if (pMesh) pMesh->Render(_pCommandList, 1); //Draw

}

void Particle::Animate() {

}

void Particle::OnPostRender() {
	if (pMesh) pMesh->OnPostRender(0);
}