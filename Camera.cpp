#include "stdafx.h"
#include "Camera.h"
#include "GameFramework.h"

Camera::Camera() {
	viewTransform = Matrix4x4::Identity();
	projectionTransform = Matrix4x4::Identity();
	viewPort = { 0,0, 1920, 1080, 0, 1 };
	scissorRect = { 0,0, 1920, 1080 };

}

Camera::~Camera() {
	pCameraBuffer->Unmap(0, NULL);
}

void Camera::Create(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	GameObject::Create();
	GameFramework& gameFramework = GameFramework::Instance();	// gameFramework�� ���۷����� �����´�.

	name = "ī�޶�";


	auto [width, height] = gameFramework.GetClientSize();
	viewPort = { 0,0, (float)width, (float)height, 0, 1 };
	scissorRect = { 0,0, width, height };

	UINT cbElementSize = (sizeof(VS_CameraMappedFormat) + 255) & (~255);

	ComPtr<ID3D12Resource> temp;
	pCameraBuffer = CreateBufferResource(_pDevice, _pCommandList, NULL, cbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, temp);
	pCameraBuffer->Map(0, NULL, (void**)&pMappedCamera);

	//UpdateViewTransform();
	UpdateProjectionTransform(0.1f, 10000.0f, float(width) / height, 90.0f);
}

void Camera::SetViewPortAndScissorRect(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	_pCommandList->RSSetViewports(1, &viewPort);
	_pCommandList->RSSetScissorRects(1, &scissorRect);
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&viewTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->view, &view, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(XMLoadFloat4x4(&projectionTransform)));	// ���̴��� ��?�켱 ����̱� ������ ��ġ��ķ� �ٲپ �����ش�.
	memcpy(&pMappedCamera->projection, &projection, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 envProj;
	XMStoreFloat4x4(&envProj, XMMatrixTranspose(XMLoadFloat4x4(&envProjectionTransform)));
	memcpy(&pMappedCamera->envProj, &envProj, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 envView;
	for (int i = 0; i < 6; ++i) {
		XMStoreFloat4x4(&envView, XMMatrixTranspose(XMLoadFloat4x4(&envViewTransform[i])));
		memcpy(&pMappedCamera->envView[i], &envView, sizeof(XMFLOAT4X4));
	}
	
	XMFLOAT3 worldPosition = GetWorldPosition();
	memcpy(&pMappedCamera->position, &worldPosition, sizeof(XMFLOAT3));
	
	XMFLOAT3 rr = playerPosition;
	memcpy(&pMappedCamera->playerPosition, &rr, sizeof(XMFLOAT3));
	
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = pCameraBuffer->GetGPUVirtualAddress();
	_pCommandList->SetGraphicsRootConstantBufferView(0, gpuVirtualAddress);
}

void Camera::UpdateViewTransform() {
	// �÷��̾� ��ġ���� ���������� �ٶ󺸰� �������� ������� ����
	XMFLOAT3 worldPosition = GetWorldPosition();
	// +-xyz
	XMFLOAT3 envLook[6] = { {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };
	XMFLOAT3 envUp[6] = { {0,1,0}, {0,1,0}, {0,0,-1}, {0,0,1}, {0,1,0}, {0,1,0} };
	
	for (int i = 0; i < 6; ++i) {
		XMFLOAT3 envLookAtWorld = Vector3::Add(playerPosition, envLook[i]);
		envViewTransform[i] = Matrix4x4::LookAtLH(playerPosition, envLookAtWorld, envUp[i]);
	}
	XMFLOAT3 lookAtWorld = Vector3::Add(worldPosition, GetWorldLookVector());
	viewTransform = Matrix4x4::LookAtLH(worldPosition, lookAtWorld, GetWorldUpVector());
}

void Camera::UpdateProjectionTransform(float _nearDistance, float _farDistance, float _aspectRatio, float _fovAngle) {
	projectionTransform = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(_fovAngle), _aspectRatio, _nearDistance, _farDistance);
	envProjectionTransform = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(_fovAngle), 1.0f, _nearDistance, _farDistance);

}

void Camera::UpdateWorldTransform() {
	GameObject::UpdateWorldTransform();
	UpdateViewTransform();
}
