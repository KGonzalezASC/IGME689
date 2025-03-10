#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <string>
#include <vector>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib") //includes library that isnt normally in project
#pragma comment(lib, "dxgi.lib") //can also be added via Linker -> input -> Additional Dependencies
#include "SharedBuffers.h"




namespace Graphics
{
	// --- GLOBAL VARS ---

	// Primary D3D11 API objects
	//simlarly to webgpu where we need device and render context
	inline Microsoft::WRL::ComPtr<ID3D11Device1> Device;
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext1> Context11_1;
	//a swap chain is: a buffer that holds the image that is being displayed on the screen
	//handles double buffering
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;

	// Rendering buffers
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRTV;
	inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthBufferDSV;

	// --- FUNCTIONS ---

	// Getters
	bool VsyncState();
	std::wstring APIName();

	// General functions
	HRESULT Initialize(unsigned int windowWidth, unsigned int windowHeight, HWND windowHandle, bool vsyncIfPossible);
	void ShutDown();
	void ResizeBuffers(unsigned int width, unsigned int height);
	void UpdateInstanceBuffer(const std::vector<InstanceData>& instances);
	// Debug Layer
	void PrintDebugMessages();
}
