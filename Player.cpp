#include "stdafx.h"
#include "Player.h"
#include "GameFramework.h"



Player::Player() {
	hp = 100.0f;
	isDead = false;
	moveSpeed = 0.02f;
	reloadTime = 0.0f;
}

Player::~Player() {

}

void Player::Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_ObjectName, _pDevice, _pCommandList);

	GameFramework& gameFramework = GameFramework::Instance();
	
	self = shared_from_this();
	SetLocalPosition(XMFLOAT3(500, 200, 500));
	name = "플레이어";

	// 환경매핑을 보여주기 위한 구체
	pEnv = gameFramework.GetGameObjectManager().GetGameObject("Sphere", _pDevice, _pCommandList);
	pEnv->SetLocalScale(XMFLOAT3(100, 100, 100));
	pEnv->UpdateObject();
	SetChild(pEnv);
	//pLight = make_shared<Light>(shared_from_this());
	// 클래스 상속 관계에서 포인터 형 변환시 dynamic_cast(런타임 이후 동작)을 사용하자
	//auto playScene = dynamic_pointer_cast<PlayScene>(gameFramework.GetCurrentScene());
	//playScene->AddLight(pLight);

	UpdateObject();
	
}

bool Player::GetIsDead() const {
	return isDead;
}

void Player::FireMissile(list<shared_ptr<GameObject>>& _pMissiles, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (reloadTime > 0) return;
	reloadTime = 0.1f;
	shared_ptr<Missile> pMissileLeft = make_shared<Missile>();
	shared_ptr<Missile> pMissileRight = make_shared<Missile>();
	XMFLOAT3 dist = GetLocalRightVector();
	dist = Vector3::ScalarProduct(dist, 2.0f);
	XMFLOAT3 left = Vector3::Subtract(GetWorldPosition(), dist);
	XMFLOAT3 right = Vector3::Subtract(GetWorldPosition(), Vector3::ScalarProduct(dist, -1.0f));
	pMissileLeft->Create(left, GetLocalRotate(), _pDevice, _pCommandList);
	pMissileRight->Create(right, GetLocalRotate(), _pDevice, _pCommandList);
	_pMissiles.push_back(pMissileLeft);
	_pMissiles.push_back(pMissileRight);
}

void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	//GameObject::Render(_pCommandList);
	if (isPlane) pChildren[0]->Render(_pCommandList);
	else pChildren[1]->Render(_pCommandList);
}

shared_ptr<Camera> Player::GetCamera() const {
	return pCamera.lock();
}

void Player::Animate(double _timeElapsed) {
	if (isDead) return;
	prevWorld = worldTransform;
	
	if (hp < 0) isDead = true;
	reloadTime -= _timeElapsed;
	
	if(pChildren[0]) pChildren[0]->Animate(_timeElapsed);

	if (rotateAngle != 0.0f)
	{
		Rotate(rotateAxis, rotateAngle, _timeElapsed);
	}
	Move(moveVector, _timeElapsed);
	//UpdateObject();
	
	// 비행기가 기울어져 있으면 수평으로 날수있게 회전한다.
	XMFLOAT3 up = GetLocalUpVector();

	XMFLOAT3 baseUp = XMFLOAT3(0, 1, 0);
	float angle2 = Vector3::Angle(up, baseUp);
	XMFLOAT3 axis2 = Vector3::CrossProduct(up, baseUp);
	//RotateRigid(axis2, angle2, _timeElapsed);
	if (angle2 != 0.0f)
	{
		Rotate(axis2, angle2, 1);
	}
	UpdateObject();

	if (!CheckCollisionWithTerrain(shared_from_this()))
	{
		moveVector = Vector3::ScalarProduct(moveVector, 0.98f);
	}
	else {
		worldTransform = prevWorld;
		XMFLOAT3 pVec = Vector3::ScalarProduct(moveVector, -1.0f);
		pVec.y += 0.1f;
		Move(pVec, _timeElapsed);
		Rotate(rotateAxis, -rotateAngle, _timeElapsed);
		moveVector = XMFLOAT3(0, 0, 0);
		UpdateObject();
	}
	
	rotateAngle = 0.0f;
	//InitVector();

}

Enemy::Enemy() {
	isDead = false;
	moveSpeed = 0.3f;
	//SetLocalPosition(_position);

	//UpdateObject();
}

Enemy::~Enemy() {
	
}



bool Enemy::CheckRemove() const {
	return isDead;
}

void Enemy::Remove() {
	isDead = true;
}

void Enemy::Create(string _objectName, const XMFLOAT3& _pos, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create(_objectName, _pDevice, _pCommandList);
	self = shared_from_this();
	SetLocalPosition(_pos);
	UpdateObject();
}

void Enemy::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index) {

}

void Enemy::Animate(double _timeElapsed, const XMFLOAT3& _playerPos) {

	GameObject::Animate(_timeElapsed);
	// 플레이어와 적 사이 각도를 구해 그 방향으로 회전한다.
	XMFLOAT3 enemyPos = GetWorldPosition();
	XMFLOAT3 dir = Vector3::Subtract(_playerPos, enemyPos );
	XMFLOAT3 look = GetLocalLookVector();
	
	float angle = Vector3::Angle(look, dir);
	XMFLOAT3 axis = Vector3::CrossProduct(look, dir);
	if (GetLocalUpVector().y < 0) axis.y *= -1;
	RotateRigid(axis, angle, _timeElapsed);

	if (rotateAngle != 0.0f)
	{
		Rotate(axis, rotateAngle, 1);
	}
	
	// 비행기가 기울어져 있으면 수평으로 날수있게 회전한다.
	XMFLOAT3 up = GetLocalUpVector();
	XMFLOAT3 baseUp = XMFLOAT3(0, 1, 0);
	float angle2 = Vector3::Angle(up, baseUp);
	XMFLOAT3 axis2 = Vector3::CrossProduct(up, baseUp);
	RotateRigid(axis2, angle2, _timeElapsed);

	if (rotateAngle != 0.0f)
	{
		Rotate(axis2, rotateAngle, 1);
	}
	MoveFrontRigid(true, _timeElapsed);


	if (moveVector.x != 0 || moveVector.y != 0 || moveVector.z != 0)
	{
		Move(moveVector, _timeElapsed);
	}
	

	UpdateObject();
	if (CheckCollisionWithTerrain(shared_from_this()))
	{
		Remove();
		return;
	}
	InitVector();
}
