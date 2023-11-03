#pragma once
#include "GameObject.h"
#include "Player.h"
class Room {
public:
	
private:
	// 방의 고유 번호
	int id;

	// 방의 타입.
	string type;
	

	// 플레이어가 방 내에 있는지 확인하기 위함
	BoundingOrientedBox boundingBox;

	// 룸내에 인접한 다른 룸들
	vector<weak_ptr<Room>> pSideRooms;

	// 각종 오브젝트들
	array <weak_ptr<Player>, 2> pPlayers;
	vector<shared_ptr<GameObject>> pItems;
	vector<shared_ptr<GameObject>> pEffects;
	vector<shared_ptr<GameObject>> pPlayerAttacks;
	vector<shared_ptr<GameObject>> pEnemyAttacks;
	vector<shared_ptr<GameObject>> pObstacles;

public:
	Room();
	~Room();

public:
	int GetID() const;
	string GetType() const;

	const BoundingOrientedBox& GetBoundingBox() const;
	vector<weak_ptr<Room>>& GetSideRooms();

	void SetType(string _type);
	void SetPlayer(array<shared_ptr<Player>, 2>& _pPlayers);
	void AnimateObjects(double _timeElapsed);

	void CheckCollision();
	void CheckCollisionPlayerAndObstacle();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox);
	vector<int> LoadRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	shared_ptr<GameObject> LoadObjectFromRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

