#pragma once
#include "Player.h"
#include "Light.h"
#include "Image2D.h"

class Scene {
protected:

public:
	Scene();
	virtual ~Scene();

public:
	virtual void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void ReleaseUploadBuffers() = 0;
	virtual void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void CheckCollision();
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) = 0;
	virtual void RenderParticle(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
	virtual void PostRenderParticle() = 0;
	virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// PlayScene
class PlayScene : public Scene {
private:
	
	int nStage;
	float spawnTime;
	float playerHP;
	
	float keybufferTime;
	bool drawHitBoxToggle;
	
	// 플레이어의 포인터. 첫 플레이 씬 생성 시에 플레이어가 생성되어 저장
	// 스테이지(씬) 전환 시에 그 씬으로 플레이어 포인터를	넘겨줌
	shared_ptr<Player> pPlayer;

	shared_ptr<GameObject> pWater;
	
	list<shared_ptr<GameObject>> pEnemys;
	list<shared_ptr<GameObject>> pMissiles;
	list<shared_ptr<Effect>> pEffects;
	list<shared_ptr<Particle>> pParticles;
	
	shared_ptr<SkyBox> pSkyBox;
	
	shared_ptr<TerrainMap> pTerrain;
	vector<shared_ptr<BillBoard>> pBillBoards;
	unordered_map<string, shared_ptr<Image2D>> pUIs;

	ComPtr<ID3D12Resource> pLightsBuffer;
	vector<shared_ptr<Light>> pLights;
	shared_ptr<LightsMappedFormat> pMappedLights;
	
	shared_ptr<FullScreenObject> pDepthObject;

	XMFLOAT4 globalAmbient;
	shared_ptr<Camera> camera;

	
public:
	PlayScene(int _stageNum);
	~PlayScene() final;

public:
	void Init(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void ReleaseUploadBuffers();
	void ProcessKeyboardInput(const array<UCHAR, 256>& _keysBuffers, float _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	
	void AnimateObjects(double _timeElapsed, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void CheckCollision() final;
	void UpdateShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void UpdateLightShaderVariables(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, float _timeElapsed) final;
	void RenderParticle(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void PostRenderParticle() final;
	void PreRender(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;
	void AddLight(const shared_ptr<Light>& _pLight);
	
	shared_ptr<TerrainMap> GetTerrain() const;
	virtual void LoadStage(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};