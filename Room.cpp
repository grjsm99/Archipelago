#include "stdafx.h"
#include "GameFramework.h"
#include "Door.h"
#include "Obstacle.h"

class GameFramework;

Room::Room() {
	id = 0;
	type = "none";
}

Room::~Room() {

}

void Room::AnimateObjects(double _timeElapsed) {
	for (const auto& pItem : pItems) {
		pItem->Animate(_timeElapsed);
	}

	for (const auto& pEffect : pEffects) {
		pEffect->Animate(_timeElapsed);
	}

	for (const auto& pPlayerAttack : pPlayerAttacks) {
		pPlayerAttack->Animate(_timeElapsed);
	}

	for (const auto& pEnemyAttack : pEnemyAttacks) {
		pEnemyAttack->Animate(_timeElapsed);
	}

	for (const auto& pObstacle : pObstacles) {
		pObstacle->Animate(_timeElapsed);
	}


}

void Room::CheckCollision() {
	CheckCollisionPlayerAndObstacle();
}

void Room::CheckCollisionPlayerAndObstacle() {

	for (auto playerWeakPtr : pPlayers) {
		shared_ptr<Player> player = playerWeakPtr.lock();
		for (auto obstacle : pObstacles) {
			// 플레이어와 Obstacle과 부딪혔을 경우
			// 플레이어의 부딪힌 부분의 오브젝트와 장애물의 부딪힌 부분의 오브젝트를 반환
			player->CheckCollision(obstacle, player);
		}
		player->Rotate(Vector4::QuaternionRotation(XMFLOAT3(0, 1, 0), player->GettAngle()));
		player->Move(player->GettVector());
		//selfLock->Rotate(Vector4::QuaternionRotation(rotateAxis, -rotateAngle));

		// OOBB를 원래대로 업데이트 한다.
		player->UpdateObject();
	}

}


int Room::GetID() const {
	return id;
}

string Room::GetType() const {
	return type;
}

const BoundingOrientedBox& Room::GetBoundingBox() const {
	return boundingBox;
}

vector<weak_ptr<Room>>& Room::GetSideRooms() {
	return pSideRooms;
}



void Room::SetType(string _type) {
	type = _type;
}

void Room::SetPlayer(array<shared_ptr<Player>, 2>& _pPlayers) {
	for (int i = 0; i < pPlayers.size(); ++i) {
		pPlayers[i] = _pPlayers[i];
	}
}



void Room::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {


	for (const auto& pItem : pItems) {
		pItem->Render(_pCommandList);
	}

	for (const auto& pEffect : pEffects) {
		pEffect->Render(_pCommandList);
	}

	for (const auto& pPlayerAttack : pPlayerAttacks) {
		pPlayerAttack->Render(_pCommandList);
	}

	for (const auto& pEnemyAttack : pEnemyAttacks) {
		pEnemyAttack->Render(_pCommandList);
	}

	for (const auto& pObstacle : pObstacles) {
		pObstacle->Render(_pCommandList);
	}
}

void Room::RenderHitBox(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, HitBoxMesh& _hitBox) {
	for (const auto& pItem : pItems) {
		pItem->RenderHitBox(_pCommandList, _hitBox);
	}

	for (const auto& pEffect : pEffects) {
		pEffect->RenderHitBox(_pCommandList, _hitBox);
	}

	for (const auto& pPlayerAttack : pPlayerAttacks) {
		pPlayerAttack->RenderHitBox(_pCommandList, _hitBox);
	}

	for (const auto& pEnemyAttack : pEnemyAttacks) {
		pEnemyAttack->RenderHitBox(_pCommandList, _hitBox);
	}

	for (const auto& pObstacle : pObstacles) {
		pObstacle->RenderHitBox(_pCommandList, _hitBox);
	}
}

vector<int> Room::LoadRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	// roomID (UINT)
	_file.read((char*)&id, sizeof(UINT));

	// nEffect (UINT)
	UINT nEffect, nObstacle, nItem;

	_file.read((char*)&nEffect, sizeof(UINT));
	pEffects.reserve(nEffect);
	for (int i = 0; i < nEffect; ++i) {
		pEffects.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	_file.read((char*)&nObstacle, sizeof(UINT));
	pObstacles.reserve(nObstacle);
	for (int i = 0; i < nObstacle; ++i) {
		pObstacles.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	_file.read((char*)&nItem, sizeof(UINT));
	pItems.reserve(nItem);
	for (int i = 0; i < nItem; ++i) {
		pItems.push_back(LoadObjectFromRoom(_file, _pDevice, _pCommandList));
	}

	vector<int> nextRooms;

	UINT nNextRoom, nextRoomID;
	_file.read((char*)&nNextRoom, sizeof(UINT));
	for (int i = 0; i < nNextRoom; ++i) {
		_file.read((char*)&nextRoomID, sizeof(UINT));
		nextRooms.push_back(nextRoomID);
	}
	return nextRooms;
}

shared_ptr<GameObject> Room::LoadObjectFromRoom(ifstream& _file, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	string classType, fileName;
	ReadStringBinary(classType, _file);
	ReadStringBinary(fileName, _file);
	shared_ptr<GameObject> newObject;
	/*if (classType == "Door") {
		newObject = make_shared<Door>();
	}
	else if (classType == "Obstacle") {
		newObject = make_shared<Obstacle>();
	}
	else*/ {
		newObject = make_shared<GameObject>();
	}
	// GameObject에 담기는 기본 정보들 초기화, 오브젝트 매니저에서 불러옴
	newObject->Create(fileName, _pDevice, _pCommandList);

	XMFLOAT3 buffer;
	XMFLOAT4 bufferQ;
	// worldTransform (float3)
	_file.read((char*)&buffer, sizeof(XMFLOAT3));
	newObject->SetLocalPosition(buffer);

	// worldScale (float3)
	_file.read((char*)&buffer, sizeof(XMFLOAT3));
	newObject->SetLocalScale(buffer);

	// worldRotation (float4)
	_file.read((char*)&bufferQ, sizeof(XMFLOAT4));

	newObject->SetLocalRotation(bufferQ);

	newObject->UpdateLocalTransform();
	newObject->UpdateWorldTransform();

	newObject->UpdateOOBB();
	return newObject;
}