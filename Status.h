#pragma once

class GameObject;
class TerrainMap;
// RigidBody 멤버 자체를 private로 하면 되므로 struct로 함
class RigidBody {
public:
	// 충돌체크용 지형 포인터
	static shared_ptr<TerrainMap> pTerrain;
protected:
	// m/s
	XMFLOAT3 acc; // right, up, look
	float moveSpeed;
	XMFLOAT3 moveVector;
	XMFLOAT3 rotateAxis;

	XMFLOAT4X4 prevWorld;


	// degree/s
	float rotateAngle;

	weak_ptr<GameObject> self;
public:
	RigidBody();
	~RigidBody();

	void InitVector();
	void Jump(float _power);

	bool CheckCollisionWithTerrain(const shared_ptr<GameObject>& _obj);
	
	void RotateRigid(const XMFLOAT3& _axis, float _angle, float _timeElapsed);
	void MoveFrontRigid(bool _isfront, float _timeElapsed);
	void MoveRightRigid(bool _isright, float _timeElapsed);
	void MoveUpRigid(bool _isup, float _timeElapsed);
	
	void KnockBack(const XMFLOAT3& _vector);
};
