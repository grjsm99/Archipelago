#pragma once
#include "GameObject.h"
#include "Status.h"
#include "Camera.h"

class Player : public GameObject, public RigidBody {
private:
	// 플레이어가 죽은지를 판단
	float hp;
	bool isDead;
	bool isPlane = true;
	weak_ptr<Camera> pCamera;
	float reloadTime;
	shared_ptr<GameObject> pEnv;
	
public:
	Player();
	~Player();

public:
	void Create(string _ObjectName, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	bool GetIsDead() const;

	void FireMissile(list<shared_ptr<GameObject>>& _pMissiles, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	shared_ptr<Camera> GetCamera() const;

	void Animate(double _timeElapsed);
	void SetPlane() { isPlane = !isPlane;};
	bool GetPlane() { return isPlane; };
	float Hit(float _damage) { hp -= _damage; return hp; };
};

class Enemy : public GameObject, public RigidBody {
private:

public:
	//static void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);

private:
	bool isDead;
	
public:
	Enemy();
	~Enemy();

public:
	bool CheckRemove() const;
	void Remove();
	
	void Animate(double _timeElapsed, const XMFLOAT3& _playerPos);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, int _index);
	void Create(string _objectName, const XMFLOAT3& _pos, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

