// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once


#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")    // 콘솔창 띄우기( 테스트를 위한 용도 )
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define MAX_LIGHTS 100
#define MAX_INSTANCE 50
#define WATER_HEIGHT 150.0f
#define GRAVITY 9.8

//#define DEBUG

// Windows 헤더 파일
#include <windows.h>
#include <random>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <exception>
#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <mmsystem.h>

// com_error 디버그용
#include <comdef.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


// // SDKDDKVer.h를 포함하면 최고 수준의 가용성을 가진 Windows 플랫폼이 정의됩니다.
// 이전 Windows 플랫폼용 애플리케이션을 빌드하려는 경우에는 SDKDDKVer.h를 포함하기 전에
// WinSDKVer.h를 포함하고 _WIN32_WINNT를 지원하려는 플랫폼으로 설정합니다.
#include <SDKDDKVer.h>

// io
#include <iostream>

// 스마트 포인터 
#include <memory>

// 컨테이너
#include <vector>
#include <array>
#include <map>
#include <ranges>
#include <queue>
#include <stack>
#include <format>
#include <fstream>
#include <unordered_map>

#include <numeric>
#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

extern UINT gnCbvSrvDescriptorIncrementSize;
extern UINT	gnRtvDescriptorIncrementSize;
extern UINT gnDsvDescriptorIncrementSize;

extern D3D12_VERTEX_BUFFER_VIEW t1;
extern D3D12_VERTEX_BUFFER_VIEW t2;

using Microsoft::WRL::ComPtr;

extern bool isEnv;
// float 난수 생성
float random(float min, float max);

// file로 부터 string을 읽는다.
void ReadStringBinary(string& _dest, ifstream& _file);
void SynchronizeResourceTransition(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource, D3D12_RESOURCE_STATES d3dStateBefore, D3D12_RESOURCE_STATES d3dStateAfter);
// 리소스 생성
ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, void* _pData, UINT _byteSize, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_STATES _resourceStates, ComPtr<ID3D12Resource>& _pUploadBuffer);

//xmfloat 출력하기
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4X4& f4x4);

ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates);
ComPtr<ID3D12Resource> CreateTexture2DResource(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);


namespace Vector3 {
	inline XMFLOAT3 Normalize(const XMFLOAT3& _vector) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&_vector)));
		return(result);
	}
	inline XMFLOAT3 Normalize(float _x, float _y, float _z) {
		XMFLOAT3 result(_x, _y, _z);
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&result)));
		return(result);
	}

	inline XMFLOAT3 ScalarProduct(const XMFLOAT3& _vector, float _scalar) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector) * _scalar);
		return result;
	}
	inline XMFLOAT3 Add(const XMFLOAT3& _vector1, const XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) + XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Add(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2, float _scalar) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) + (XMLoadFloat3(&_vector2) * _scalar));
		return result;
	}
	inline XMFLOAT3 Subtract(const XMFLOAT3& _vector1, const  XMFLOAT3& _vector2) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&_vector1) - XMLoadFloat3(&_vector2));
		return result;
	}
	inline XMFLOAT3 Transform(const XMFLOAT3& _vector, const XMFLOAT4X4& _matrix) {
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Transform(XMLoadFloat3(&_vector), XMLoadFloat4x4(&_matrix)));
		return result;
	}
	inline float DotProduct(XMFLOAT3& _vector1, XMFLOAT3& _vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result.x);
	}

	inline XMFLOAT3 CrossProduct(XMFLOAT3& _vector1, XMFLOAT3& _vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result);
	}

	inline float Angle(XMFLOAT3& _vector1, XMFLOAT3& _vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3AngleBetweenVectors(XMLoadFloat3(&_vector1), XMLoadFloat3(&_vector2)));
		return(xmf3Result.x);
	}
}

namespace Vector4 {
	inline XMFLOAT4 Add(XMFLOAT4& _vector1, XMFLOAT4& _vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&_vector1) + XMLoadFloat4(&_vector2));
		return(xmf4Result);
	}
	
	inline XMFLOAT4 QuaternionMultiply(const XMFLOAT4& _quaternion1, const XMFLOAT4& _quaternion2) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionMultiply(XMLoadFloat4(&_quaternion1), XMLoadFloat4(&_quaternion2)));
		return result;
	}

	inline XMFLOAT4 QuaternionRotation(const XMFLOAT3& _axis, float _angle) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMQuaternionRotationAxis(XMLoadFloat3(&_axis), XMConvertToRadians(_angle)));
		return result;
	}

	inline XMFLOAT4 Transform(const XMFLOAT4& _vector, const XMFLOAT4X4& _matrix) {
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMVector4Transform(XMLoadFloat4(&_vector), XMLoadFloat4x4(&_matrix)));
		return result;
	}

	inline XMFLOAT4 Multiply(XMFLOAT4& _vector1, XMFLOAT4& _vector2)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, XMLoadFloat4(&_vector1) * XMLoadFloat4(&_vector2));
		return result;
	}

	inline XMFLOAT4 Multiply(float _scalar, XMFLOAT4& _vector)
	{
		XMFLOAT4 result;
		XMStoreFloat4(&result, _scalar * XMLoadFloat4(&_vector));
		return result;
	}
}


namespace Matrix4x4 {
	inline XMFLOAT4X4 Identity() {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixIdentity());
		return result;
	}

	inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& _matrix1, const XMFLOAT4X4& _matrix2) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMLoadFloat4x4(&_matrix1) * XMLoadFloat4x4(&_matrix2));
		return(result);
	}

	inline XMFLOAT4X4 RotationAxis(const XMFLOAT3& _axis, float _angle) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationAxis(XMLoadFloat3(&_axis), XMConvertToRadians(_angle));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 RotateQuaternion(const XMFLOAT4& _quaternion) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&_quaternion));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 RotatePitchYawRoll(float _pitch, float _yaw, float _roll) {
		XMFLOAT4X4 result;
		XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(_pitch), XMConvertToRadians(_yaw), XMConvertToRadians(_roll));
		XMStoreFloat4x4(&result, rotateMatrix);
		return result;
	}

	inline XMFLOAT4X4 LookAtLH(const XMFLOAT3& _eyePosition, const XMFLOAT3& _lookAtPosition, const XMFLOAT3& _upDirection) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixLookAtLH(XMLoadFloat3(&_eyePosition), XMLoadFloat3(&_lookAtPosition), XMLoadFloat3(&_upDirection)));
		return(result);
	}
	inline XMFLOAT4X4 PerspectiveFovLH(float _fovAngleY, float _aspectRatio, float _nearZ, float _farZ) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(_fovAngleY, _aspectRatio, _nearZ, _farZ));
		return result;
	}
	inline XMFLOAT4X4 ScaleTransform(const XMFLOAT3& scale) {
		XMFLOAT4X4 result;
		XMStoreFloat4x4(&result, XMMatrixScaling(scale.x, scale.y, scale.z));
		return result;
	}
}

