#include "BaseApp.h"

HRESULT BaseApp::init()
{
  HRESULT hr = S_OK;

  hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize SwpaChian. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Crear textura de depth stencil.
  hr = m_depthStencil.init(m_device,
    m_window.m_width,
    m_window.m_height,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    0);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Crear el depth stencil view
  hr = m_depthStencilView.init(m_device,
    m_depthStencil,
    DXGI_FORMAT_D24_UNORM_S8_UINT);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Crear el m_viewport
  hr = m_viewport.init(m_window);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Definir el layout de entrada
  std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;

  D3D11_INPUT_ELEMENT_DESC position;
  position.SemanticName = "POSITION";
  position.SemanticIndex = 0;
  position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
  position.InputSlot = 0;
  position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
  position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  position.InstanceDataStepRate = 0;
  Layout.push_back(position);

  D3D11_INPUT_ELEMENT_DESC texcoord;
  texcoord.SemanticName = "TEXCOORD";
  texcoord.SemanticIndex = 0;
  texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
  texcoord.InputSlot = 0;
  texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*12*/;
  texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  texcoord.InstanceDataStepRate = 0;
  Layout.push_back(texcoord);

  // Create the Shader Program
  hr = m_shaderProgram.init(m_device, "HybridEngine.fx", Layout);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Crear vertex buffer y index buffer para el pistol
  DrakePistol = m_modelLoader.LoadOBJModel("drakefire_pistol_low.obj");


  hr = m_vertexBuffer.init(m_device, DrakePistol, D3D11_BIND_VERTEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize VertexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_indexBuffer.init(m_device, DrakePistol, D3D11_BIND_INDEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize IndexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Establecer topología primitiva
  m_deviceContext.m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Crear los constant buffers
  hr = m_neverChanges.init(m_device, sizeof(CBNeverChanges));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_changeOnResize.init(m_device, sizeof(CBChangeOnResize));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ChangeOnResize Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_changeEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ChangesEveryFrame Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }


  // Cargar la textura
  hr = D3DX11CreateShaderResourceViewFromFile(m_device.m_device, "GunAlbedo.dds", NULL, NULL, &m_pTextureRV, NULL);
  if (FAILED(hr))
    return hr;

  // Crear el sampler state
  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = m_device.CreateSamplerState(&sampDesc, &m_pSamplerLinear);
  if (FAILED(hr))
    return hr;

  // Inicializar las matrices de mundo, vista y proyección
  XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  m_View = XMMatrixLookAtLH(Eye, At, Up);

  // Actualizar la matriz de proyección
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

  //------- CREACIÓN DE GEOMETRÍA DEL PLANO (suelo) -------//
  SimpleVertex planeVertices[] =
  {
      { XMFLOAT3(-20.0f, 0.0f, -20.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(20.0f, 0.0f, -20.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(20.0f, 0.0f,  20.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-20.0f, 0.0f,  20.0f), XMFLOAT2(0.0f, 1.0f) },
  };

  WORD planeIndices[] =
  {
      0, 2, 1,
      0, 3, 2
  };

  m_planeIndexCount = 6;

  // Store the vertex data
  for (int i = 0; i < 4; i++) {
    planeMesh.m_vertex.push_back(planeVertices[i]);
  }
  // Store the index data
  for (int i = 0; i < 6; i++) {
    planeMesh.m_index.push_back(planeIndices[i]);
  }

  // Crear el vertex buffer y index buffer para el plano
  hr = m_planeVertexBuffer.init(m_device, planeMesh, D3D11_BIND_VERTEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize PlaneVertexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_planeIndexBuffer.init(m_device, planeMesh, D3D11_BIND_INDEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize PlaneIndexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_constPlane.init(m_device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Plane Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //------- COMPILAR SHADER DE SOMBRA -------//
  hr = m_shaderShadow.CreateShader(m_device, PIXEL_SHADER, "HybridEngine.fx");

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Shadow Shader. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_constShadow.init(m_device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Shadow Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }
  //------- CREAR ESTADOS DE BLENDING Y DEPTH STENCIL PARA LAS SOMBRAS -------//
  hr = m_shadowBlendState.init(m_device);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Shadow Blend State. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_shadowDepthStencilState.init(m_device, true, false);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Depth Stencil State. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }


	m_LightPos = XMFLOAT4(2.0f, 4.0f, -2.0f, 1.0f); // Posición de la luz
	// Initialize User Interface
	
  m_userInterface.init(m_window.m_hWnd,
		                   m_device.m_device,
		                   m_deviceContext.m_deviceContext);
  return S_OK;
}

void 
BaseApp::update() {
	m_userInterface.update();
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

  // Actualizar tiempo (mismo que antes)
  static float t = 0.0f;
  if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
  {
    t += (float)XM_PI * 0.0125f;
  }
  else
  {
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0)
      dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;
  }

  // Actualizar la matriz de proyección y vista
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_neverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
  m_changeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);


  // --- Transformación del cubo ---
  // Parámetros del cubo:
  float cubePosX = 0.0f, cubePosY = 2.0f, cubePosZ = 0.0f;  // Ubicado 2 unidades arriba
  float cubeScale = 1.0f;                                    // Escala uniforme
  float cubeAngleX = 0.0f, cubeAngleY = t, cubeAngleZ = 0.0f;  // Rotación dinámica en Y

  // Crear las matrices individuales
  XMMATRIX cubeScaleMat = XMMatrixScaling(cubeScale, cubeScale, cubeScale);
  XMMATRIX cubeRotMat = XMMatrixRotationX(cubeAngleX) *
    XMMatrixRotationY(cubeAngleY) *
    XMMatrixRotationZ(cubeAngleZ);
  XMMATRIX cubeTransMat = XMMatrixTranslation(cubePosX, cubePosY, cubePosZ);

  // Combinar: primero escala, luego rota y por último traslada
  m_World = cubeTransMat * cubeRotMat * cubeScaleMat;

  // Actualizar el color animado del cub

  // --- Transformación del plano ---
  // Parámetros para el plano:
  float planePosX = 0.0f, planePosY = -5.0f, planePosZ = 0.0f;
  // Aunque los vértices ya definen un plano extenso (-20 a 20), aquí puedes ajustar el escalado adicional
  float planeScaleFactor = 1.0f; // Puedes modificar este factor para agrandar o reducir el plano
  float planeAngleX = 0.0f, planeAngleY = 0.0f, planeAngleZ = 0.0f; // Sin rotación por defecto

  XMMATRIX planeScaleMat = XMMatrixScaling(planeScaleFactor, planeScaleFactor, planeScaleFactor);
  XMMATRIX planeRotMat = XMMatrixRotationX(planeAngleX) *
    XMMatrixRotationY(planeAngleY) *
    XMMatrixRotationZ(planeAngleZ);
  XMMATRIX planeTransMat = XMMatrixTranslation(planePosX, planePosY, planePosZ);

  // Combinar transformaciones para el plano
  m_PlaneWorld = planeTransMat * planeRotMat * planeScaleMat;


  // Update Plane
  cbPlane.mWorld = XMMatrixTranspose(m_PlaneWorld);
  cbPlane.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  m_constPlane.update(m_deviceContext, nullptr, 0, nullptr, &cbPlane, 0, 0);

  // Update cube
  m_vMeshColor = XMFLOAT4(1,1,1,1);
  cb.mWorld = XMMatrixTranspose(m_World);
  cb.vMeshColor = m_vMeshColor;
  m_changeEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);

  // Update Shadow cube
  float dot = m_LightPos.y;
  XMMATRIX shadowMatrix = XMMATRIX(
    dot, -m_LightPos.x, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, -m_LightPos.z, dot, 0.0f,
    0.0f, -1.0f, 0.0f, dot
  );
  XMMATRIX shadowWorld = m_World * shadowMatrix;
  cbShadow.mWorld = XMMatrixTranspose(shadowWorld);
  cbShadow.vMeshColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
  m_constShadow.update(m_deviceContext, nullptr, 0, nullptr, &cbShadow, 0, 0);


}

void 
BaseApp::render() {
  // Limpiar el back buffer y el depth buffer
  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

  // Set Viewport
  m_viewport.render(m_deviceContext);

  m_depthStencilView.render(m_deviceContext);

  // Configurar los buffers y shaders para el pipeline
  m_shaderProgram.render(m_deviceContext);

  // Asignar buffers constantes
  m_neverChanges.render(m_deviceContext, 0, 1);
  m_changeOnResize.render(m_deviceContext, 1, 1);
  //------------- Renderizar el plano (suelo) -------------//
  // Asignar buffers Vertex e Index
  m_planeVertexBuffer.render(m_deviceContext, 0, 1);
  m_planeIndexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

  // Asignar buffers constantes
  m_constPlane.render(m_deviceContext, 2, 1);
  m_constPlane.render(m_deviceContext, 2, 1, true);

  m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
  m_deviceContext.m_deviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
  m_deviceContext.m_deviceContext->DrawIndexed(planeMesh.m_index.size(), 0, 0);

  //------------- Renderizar el cubo (normal) -------------//
  // Asignar buffers Vertex e Index
  m_vertexBuffer.render(m_deviceContext, 0, 1);
  m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

  // Asignar buffers constantes
  m_changeEveryFrame.render(m_deviceContext, 2, 1);
  m_changeEveryFrame.render(m_deviceContext, 2, 1, true);

  m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
  m_deviceContext.m_deviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
  m_deviceContext.m_deviceContext->DrawIndexed(DrakePistol.m_index.size(), 0, 0);

  //------------- Renderizar la sombra del cubo -------------//
  m_shaderShadow.render(m_deviceContext, PIXEL_SHADER);

  m_shadowBlendState.render(m_deviceContext, blendFactor, 0xffffffff);
  m_shadowDepthStencilState.render(m_deviceContext, 0);

  // Asignar buffers Vertex e Index
  m_vertexBuffer.render(m_deviceContext, 0, 1);
  m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

  // Asignar buffers constantes
  m_constShadow.render(m_deviceContext, 2, 1, true);

  m_deviceContext.m_deviceContext->DrawIndexed(DrakePistol.m_index.size(), 0, 0);

  m_shadowBlendState.render(m_deviceContext, blendFactor, 0xffffffff, true);
  m_shadowDepthStencilState.render(m_deviceContext, 0, true);

	// Renderizar la interfaz de usuario y mostrar la imagen
	m_userInterface.render();

  // Presentar el back buffer al front buffer
  m_swapChain.present();
}

void 
BaseApp::destroy() {
	m_userInterface.destroy();
  if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

  m_shadowBlendState.destroy();
  m_shadowDepthStencilState.destroy();
  m_shaderShadow.destroy();

  m_planeVertexBuffer.destroy();
  m_planeIndexBuffer.destroy();

  if (m_pSamplerLinear) m_pSamplerLinear->Release();
  if (m_pTextureRV) m_pTextureRV->Release();

  m_neverChanges.destroy();
  m_changeOnResize.destroy();
  m_changeEveryFrame.destroy();
  m_constPlane.destroy();
  m_constShadow.destroy();

  m_vertexBuffer.destroy();
  m_indexBuffer.destroy();
  m_shaderProgram.destroy();
  m_depthStencil.destroy();
  m_depthStencilView.destroy();
  m_renderTargetView.destroy();
  m_swapChain.destroy();
  if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->Release();
  if (m_device.m_device) m_device.m_device->Release();
}

int
BaseApp::run(HINSTANCE hInstance,
						 HINSTANCE hPrevInstance,
						 LPWSTR lpCmdLine,
						 int nCmdShow,
						 WNDPROC wndproc) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(m_window.init(hInstance, nCmdShow, wndproc))) {
		return 0;

	}

	if (FAILED(init()))	{
		destroy();
		return 0;
	}

	// Bucle principal de mensajes
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Actualiza la lógica de la escena
			update();
			// Renderiza la escena
			render();
		}
	}

	destroy();

	return (int)msg.wParam;
}