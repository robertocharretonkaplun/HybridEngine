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
#include "UserInterface.h"
#include "ModelLoader.h"
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
	Device m_device;
	SwapChain m_swapChain;
	DeviceContext m_deviceContext;
	Texture m_backBuffer;
	RenderTargetView m_renderTargetView;
	Texture m_depthStencil;
	DepthStencilView m_depthStencilView;
	Viewport m_viewport;
	ShaderProgram m_shaderProgram;
	ShaderProgram m_shaderShadow;
	BlendState m_shadowBlendState;
	DepthStencilState m_shadowDepthStencilState;
	ModelLoader m_modelLoader;
	// Camera Buffers
	Buffer m_neverChanges;
	Buffer m_changeOnResize;

	// Cube Buffers
	Buffer m_vertexBuffer;
	Buffer m_indexBuffer;
	Buffer m_changeEveryFrame;

	// Cube Shadow Buffers
	Buffer m_constShadow;

	// Plane Buffers
	Buffer m_planeVertexBuffer;
	Buffer m_planeIndexBuffer;
	Buffer m_constPlane;

	// Variable global para el constant buffer de la luz puntual
	ID3D11ShaderResourceView* m_pTextureRV = NULL;
	ID3D11SamplerState* m_pSamplerLinear = NULL;
	XMMATRIX                            m_World;         // Para el cubo
	XMMATRIX                            m_PlaneWorld;    // Para el plano
	XMMATRIX                            m_View;
	XMMATRIX                            m_Projection;
	XMFLOAT4                            m_vMeshColor;

	//----- Variables agregadas para el plano y sombras -----//
	UINT                                m_planeIndexCount = 0;
	XMFLOAT4                            m_LightPos; // Posición de la luz(2.0f, 4.0f, -2.0f, 1.0f)

	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	//MeshComponent cubeMesh;
	MeshComponent DrakePistol;
	MeshComponent planeMesh;
	CBNeverChanges cbNeverChanges;
	CBChangeOnResize cbChangesOnResize;
	CBChangesEveryFrame cbPlane;
	CBChangesEveryFrame cb;
	CBChangesEveryFrame cbShadow;
	UserInterface												m_userInterface;



};