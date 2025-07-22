#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "DeviceContext.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "InputLayout.h"
#include "ShaderProgram.h"
#include "Buffer.h"
#include "MeshComponent.h"
#include "BlendState.h"
#include "DepthStencilState.h"

class
BaseApp {
public:
	BaseApp()  = default;
	~BaseApp() = default;

	HRESULT
	init();
	
	void 
	update();
	
	void 
	render();
	
	void 
	destroy();

	int 
	run(HINSTANCE hInstance, 
			HINSTANCE hPrevInstance, 
			LPWSTR lpCmdLine, 
			int nCmdShow, 
			WNDPROC wndproc);

private:
	Window m_window;
};