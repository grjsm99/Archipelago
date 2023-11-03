#include "stdafx.h"
#include "Scene.h"
#include "Timer.h"
#include "GameFramework.h"



Scene::Scene() {
	
}

Scene::~Scene() {
	
}

void Scene::CheckCollision() {

}

///////////////////////////////////////////////////////////////////////////////
/// PlayScene
PlayScene::PlayScene(int _stageNum) {

	nStage = _stageNum;
	globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	drawHitBoxToggle = true;
}

void PlayScene::Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameFramework& gameFramework = GameFramework::Instance();
	spawnTime = 0.0f;
	// 스테이지 생성
	LoadStage(_pDevice, _pCommandList);
	camera = make_shared<Camera>();
	camera->Create(_pDevice, _pCommandList);

	//camera->SetLocalPosition(XMFLOAT3(0.0, 10.0, -20)); // pPlayer 고정시
	camera->SetLocalPosition(XMFLOAT3(2.0, 40.0, -100));
	//camera->SetLocalPosition(XMFLOAT3(0.0, 0.5, -1));
	//
	camera->SetLocalRotation(Vector4::QuaternionRotation(XMFLOAT3(0,1,0), 0.0f));
	
	camera->SetPlayerPos(pPlayer->GetWorldPosition());

	camera->UpdateLocalTransform();
	camera->UpdateWorldTransform();

	pPlayer->SetChild(camera);
	pPlayer->UpdateObject();

	// 현재 두 플레이어가 있는 방을 첫방으로 설정
	//pNowRooms[0] = pRooms[0];
	//pNowRooms[1] = pRooms[0];
	ComPtr<ID3D12Resource> temp;
	UINT ncbElementBytes = ((sizeof(LightsMappedFormat) + 255) & ~255); //256의 배수
	pLightsBuffer = ::CreateBufferResource(_pDevice, _pCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	
	pLightsBuffer->Map(0, NULL, (void**)&pMappedLights);

	//GameObject::Init(_pDevice, _pCommandList);
}

void PlayScene::ReleaseUploadBuffers() {
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetMeshManager().ReleaseUploadBuffers();
	gameFramework.GetTextureManager().ReleaseUploadBuffers();
	for (auto& pUI : pUIs) {
		pUI.second->ReleaseUploadBuffers();
	}
}

PlayScene::~PlayScene() {
	pLightsBuffer->Unmap(0, NULL);
}

void PlayScene::ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	

	GameFramework& gameFramework = GameFramework::Instance();
	
	// 회전과 스케일링은 앞쪽에 move는 뒤쪽에 곱한다.
	//XMFLOAT4X4 transform = Matrix4x4::Identity();
	//if (_keysBuffers['E'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayers[0]->GetRotateMatrix(0.0f, 5.0f, 0.0f), transform);
	//}
	//if (_keysBuffers['Q'] & 0xF0) {
	//	transform = Matrix4x4::Multiply(pPlayers[0]->GetRotateMatrix(0.0f, -5.0f, 0.0f), transform);
	//}
	if (_keysBuffers['E'] & 0xF0) {
		pPlayer->RotateRigid(XMFLOAT3(0, 1, 0), 90.0f, _timeElapsed);
	}
	if (_keysBuffers['Q'] & 0xF0) {
		pPlayer->RotateRigid(XMFLOAT3(0, 1, 0), -90.0f, _timeElapsed);
	}
	if (_keysBuffers['3'] & 0xF0) {
		pPlayer->RotateRigid(pPlayer->GetLocalRightVector(), 90.0f, _timeElapsed);
	}
	if (_keysBuffers['4'] & 0xF0) {
		pPlayer->RotateRigid(pPlayer->GetLocalRightVector(), -90.0f, _timeElapsed);
	}
	// 
	if (_keysBuffers['W'] & 0xF0) {
		pPlayer->MoveFrontRigid(true, _timeElapsed);
	}
	if (_keysBuffers['S'] & 0xF0) {
		pPlayer->MoveFrontRigid(false, _timeElapsed);
	}
	if (_keysBuffers['D'] & 0xF0) {
		pPlayer->MoveRightRigid(true, _timeElapsed);
	}
	if (_keysBuffers['A'] & 0xF0) {
		pPlayer->MoveRightRigid(false, _timeElapsed);
	}
	if (_keysBuffers['1'] & 0xF0) {
		pPlayer->MoveUpRigid(true, _timeElapsed);
	}
	if (_keysBuffers['2'] & 0xF0) {
		pPlayer->MoveUpRigid(false, _timeElapsed);
	}
	if (_keysBuffers['P'] & 0xF0) {
		if (pMissiles.size() < 50)
			pPlayer->FireMissile(pMissiles, _pDevice, _pCommandList);
	}
	if (_keysBuffers['I'] & 0xF0) {
		if (keybufferTime < 0.0f && isEnv) {
			pPlayer->SetPlane();
			keybufferTime = 0.5f;
		}
	}
	if (_keysBuffers['R'] & 0xF0) {
		if (keybufferTime < 0.0f && pPlayer->GetPlane()) {

			if(isEnv)
				camera->SetLocalPosition(XMFLOAT3(2.0, 5.0, 12));
			else 
				camera->SetLocalPosition(XMFLOAT3(2.0, 40.0, -100));
			isEnv = !isEnv;
			camera->UpdateLocalTransform();
			camera->UpdateWorldTransform();
			keybufferTime = 0.5f;

		}
	}
	
	if (_keysBuffers['O'] & 0xF0) {
		if (keybufferTime < 0.0f) {
			drawHitBoxToggle = !drawHitBoxToggle;
			keybufferTime = 0.5f;
		}
		
	}
	//pPlayers[0]->ApplyTransform(transform, false);
	

}

void PlayScene::AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	if (pPlayer->GetIsDead()) return;
	keybufferTime -= _timeElapsed;
	spawnTime -= _timeElapsed;
	// 랜덤 적 생성
	if (spawnTime < 0) {
		if (pEnemys.size() < MAX_INSTANCE) {
			shared_ptr<Enemy> pEnemy = make_shared<Enemy>();
			XMFLOAT3 ran = XMFLOAT3(0, 0, 0);
			while (ran.x <= 100) ran.x = (int)random(8, 8888) % 2056 - 8;
			while (ran.z <= 100) ran.z = (int)random(8, 8888) % 2056 - 8;
			ran.y = pTerrain->GetHeight(ran.x, ran.z) + random(60, 200);
			pEnemy->Create("Apache", ran, _pDevice, _pCommandList);
			pEnemy->SetLocalScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
			pEnemy->UpdateObject();
			pEnemys.push_back(pEnemy);

			spawnTime = 2.0f;
		}
	}

	// 플레이어가 살아있는 경우 애니메이션을 수행
	if (!pPlayer->GetIsDead()) {
		pPlayer->Animate(_timeElapsed);
		camera->SetPlayerPos(pPlayer->GetWorldPosition());
		// 패킷 send
	}		
	for (auto& pLight : pLights) {
		if (pLight) {
			pLight->UpdateLight();
		}
	}
	for (auto& pEffect : pEffects) {
		if (pEffect) {
			pEffect->Animate(_timeElapsed);
		}
	}

	for (auto& pMissile : pMissiles) {
		pMissile->Animate(_timeElapsed);
	}

	for (auto& pEnemy : pEnemys) {
		pEnemy->Animate(_timeElapsed, pPlayer->GetLocalPosition());
	}
	
	pWater->Animate(_timeElapsed);

}
void PlayScene::CheckCollision() {
	for (auto& pEnemy : pEnemys) {
		for (auto& pMissile : pMissiles) {
			if (pEnemy->GetObj()->GetBoundingBox().Contains(pMissile->GetObj()->GetBoundingBox())) {
				pMissile->Remove();
				pEnemy->Remove();

				shared_ptr<Effect> pEffect = make_shared<Effect>();
				pEffect->Create(0.03, 8, 8, 50, 50, pEnemy->GetLocalPosition(), "Explode_8x8");
				pEffects.push_back(pEffect);
				break;
			}
		}
	}
	
	
	pMissiles.remove_if([](const shared_ptr<GameObject>& _pMissile) {
		return _pMissile->CheckRemove();
		});
	pEnemys.remove_if([](const shared_ptr<GameObject>& pEnemy) {
		return pEnemy->CheckRemove();
		});
	
	pEffects.remove_if([](const shared_ptr<Effect>& pEffect) {
		return pEffect->CheckRemove();
		});

	pParticles.remove_if([](const shared_ptr<Particle>& _pParticle) {
		return _pParticle->CheckRemove();
		});

}

void PlayScene::UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 적의 월드 행렬을 리소스 내에 직접 쓴다.
	int i = 0;
	for (auto& pEnemy : pEnemys) {
		pEnemy->UpdateShaderVariableInstance(_pCommandList, i++);
	}
}

void PlayScene::UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	int nLight = (UINT)pLights.size();
	for (int i = 0; i < nLight; ++i) {
		
		memcpy(&pMappedLights->lights[i], pLights[i].get(), sizeof(Light));
	}

	memcpy(&pMappedLights->globalAmbient, &globalAmbient, sizeof(XMFLOAT4));

	memcpy(&pMappedLights->nLight, &nLight, sizeof(int));

	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pLightsBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(2, gpuVirtualAddress);

}


void PlayScene::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) {

	GameFramework& gameFramework = GameFramework::Instance();

	float timeElapsed = _timeElapsed;
	
	// 프레임워크에서 렌더링 전에 루트시그니처를 set
	//shared_ptr<Camera> pP1Camera = pPlayers[0]->GetCamera();
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	
	if (pPlayer->GetIsDead()) {
		Image2D::GetShader()->PrepareRender(_pCommandList);
		pUIs["2DUI_gameover"]->Render(_pCommandList);
		return;
	}
	//UpdateShaderVariables(_pCommandList);

	SkyBoxMesh::GetShader()->PrepareRender(_pCommandList);
	pSkyBox->Render(_pCommandList, camera);
	
	// Enemy를 먼저 렌더
	Mesh::GetShader()->PrepareRender(_pCommandList);
	for (auto& pEnemy : pEnemys) {
		if (pEnemy) pEnemy->Render(_pCommandList);
	}

	// 외곽선 렌더
	Mesh::GetDepthShader()->PrepareRender(_pCommandList);
	pDepthObject->Render(_pCommandList);
	
	Mesh::GetShader()->PrepareRender(_pCommandList);
	for (auto& pMissile : pMissiles) {
		if (pMissile) pMissile->Render(_pCommandList);
	}

	if(isEnv) Mesh::GetEnvShader()->PrepareRender(_pCommandList);
	pPlayer->Render(_pCommandList);

	Image2D::GetShader()->PrepareRender(_pCommandList);
	pUIs["2DUI_hpbar"]->Render(_pCommandList);
	pUIs["2DUI_hp"]->Render(_pCommandList);
	
	TerrainMesh::GetShader()->PrepareRender(_pCommandList);
	pTerrain->Render(_pCommandList);
	
	BillBoardMesh::GetShader()->PrepareRender(_pCommandList);
	
	for (auto& billboard : pBillBoards) {
		billboard->Render(_pCommandList);
	}
	for (auto& pEffect : pEffects) {
		pEffect->Render(_pCommandList);
	}

	
	_pCommandList->SetGraphicsRoot32BitConstants(8, 1, &timeElapsed, 0);
	for (auto& pParticle : pParticles) {
		pParticle->Render(_pCommandList);
	}
	

	WaterMesh::GetShader()->PrepareRender(_pCommandList);
	pWater->Render(_pCommandList);

	//Mesh::GetInstanceShader()->PrepareRender(_pCommandList);

	//if (drawHitBoxToggle) {
	//	HitBoxMesh::GetShader()->PrepareRender(_pCommandList);
	//	HitBoxMesh& hitBoxMesh = GameFramework::Instance().GetMeshManager().GetHitBoxMesh();
	//	pPlayer->RenderHitBox(_pCommandList, hitBoxMesh);
	//	for (auto pEnemy : pEnemys) {
	//		if (pEnemy) pEnemy->RenderHitBox(_pCommandList, hitBoxMesh);
	//	}
	//	for (auto pMissile : pMissiles) {
	//		if (pMissile) pMissile->RenderHitBox(_pCommandList, hitBoxMesh);
	//	}
	//}
}

void PlayScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	camera->SetViewPortAndScissorRect(_pCommandList);
	camera->UpdateShaderVariable(_pCommandList);

	UpdateLightShaderVariables(_pCommandList);
	SkyBoxMesh::GetShader()->PrepareRenderEnv(_pCommandList);
	pSkyBox->Render(_pCommandList, camera);

	TerrainMesh::GetShader()->PrepareRenderEnv(_pCommandList);
	pTerrain->Render(_pCommandList);


	Mesh::GetShader()->PrepareRenderEnv(_pCommandList);
	for (auto& pEnemy : pEnemys) {
		if (pEnemy) pEnemy->Render(_pCommandList);
	}

	WaterMesh::GetShader()->PrepareRenderEnv(_pCommandList);
	pWater->Render(_pCommandList);
	
}


void PlayScene::RenderParticle(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {

	
}

void PlayScene::PostRenderParticle() {

	for (auto& pPatricle : pParticles) {
		pPatricle->OnPostRender();
	}
}
void PlayScene::AddLight(const shared_ptr<Light>& _pLight) {
	pLights.push_back(_pLight);
}

shared_ptr<TerrainMap> PlayScene::GetTerrain() const {
	return pTerrain;
}

void PlayScene::LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	// 씬에 그려질 오브젝트들을 전부 빌드.
	
	GameFramework& gameFramework = GameFramework::Instance();
	gameFramework.GetGameObjectManager().GetGameObject("Missile", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().GetGameObject("Apache", _pDevice, _pCommandList);
	gameFramework.GetGameObjectManager().GetGameObject("Sphere", _pDevice, _pCommandList);
	
	// depth shader의 서술자 힙에 depth buffer에 대한 srv를 생성한다.

	Effect::CreateBaseMesh(_pDevice, _pCommandList);
	Effect::LoadEffect("Explode_8x8", _pDevice, _pCommandList);
	
	//::SynchronizeResourceTransition(_pCommandList.Get(), gameFramework.GetTest().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
	
	//::SynchronizeResourceTransition(_pCommandList.Get(), gameFramework.GetTest().Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	
	pDepthObject = make_shared<FullScreenObject>(_pDevice, _pCommandList);
	
	pPlayer = make_shared<Player>();
	pPlayer->Create("Apache", _pDevice, _pCommandList);
	pPlayer->SetLocalScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
	//pPlayer->SetLocalScale(XMFLOAT3(20.0f, 20.0f, 20.0f));
	pPlayer->UpdateObject();
	shared_ptr<Light> baseLight = make_shared<Light>();
	baseLight->lightType = 3;
	baseLight->position = XMFLOAT3(0, 500, 0);
	baseLight->direction = XMFLOAT3(0, -1, 0);
	baseLight->diffuse = XMFLOAT4(0.2, 0.2, 0.2, 1);
	AddLight(baseLight);
	
	
	// size, startpos, uvsize
	// pos = x,y -> 0~2
	
	string name = "2DUI_hp";
	shared_ptr<Image2D> phpUI = make_shared<Image2D>(name, XMFLOAT2(0.6, 0.2), XMFLOAT2(1.385,1.75), XMFLOAT2(1,1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;
	
	name = "2DUI_hpbar";
	phpUI = make_shared<Image2D>(name, XMFLOAT2(0.7, 0.3), XMFLOAT2(1.3, 1.7), XMFLOAT2(1, 1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;
	
	name = "2DUI_gameover";
	phpUI = make_shared<Image2D>(name, XMFLOAT2(2, 2), XMFLOAT2(0,0), XMFLOAT2(1, 1), _pDevice, _pCommandList);
	pUIs[name] = phpUI;

	XMFLOAT3 size = XMFLOAT3(2056, 2056, 2056);
	pWater = make_shared<Water>(size, WATER_HEIGHT, _pDevice, _pCommandList);
	


	XMFLOAT3 tScale = XMFLOAT3(8.0f, 2.0f, 8.0f);
	XMFLOAT4 tColor = XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f);
	
	pTerrain = make_shared<TerrainMap>(_pDevice, _pCommandList, _T("Texture/HeightMap.raw"), 257, 257, 257, 257, tScale, tColor);
	RigidBody::pTerrain = pTerrain;

	shared_ptr<BillBoard> billboard; 
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Flower01", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	billboard = make_shared<BillBoard>();
	billboard->Create("Flower02", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Grass01", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);
	
	billboard = make_shared<BillBoard>();
	billboard->Create("Tree02", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	billboard = make_shared<BillBoard>();
	billboard->Create("Tree03", _pDevice, _pCommandList);
	pBillBoards.push_back(billboard);

	shared_ptr<Particle> pParticle = make_shared<Particle>
		(_pDevice, _pCommandList, pPlayer->GetWorldPosition(), 100000);
	pParticles.push_back(pParticle);

	pSkyBox = make_shared<SkyBox>();
	pSkyBox->Create(_pDevice, _pCommandList);
}