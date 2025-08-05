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

  // Set Drake Pistol Actor
  m_ADrakePistol = EngineUtilities::MakeShared<Actor>(m_device);

  if (!m_ADrakePistol.isNull()) {
    // Crear vertex buffer y index buffer para el pistol
    DrakePistol = m_modelLoader.LoadOBJModel("Models/drakefire_pistol_low.obj");

    // Cargar la textura
    hr = m_drakePistolTexture.init(m_device, "Textures/GunAlbedo", DDS);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }
    std::vector<MeshComponent> DrakePistolMeshes;
		DrakePistolMeshes.push_back(DrakePistol);
    std::vector<Texture> DrakePistolTextures;
		DrakePistolTextures.push_back(m_drakePistolTexture);
    m_ADrakePistol->setMesh(m_device, DrakePistolMeshes);
    m_ADrakePistol->setTextures(DrakePistolTextures);

		m_actors.push_back(m_ADrakePistol);
    m_ADrakePistol->getComponent<Transform>()->setPosition(EngineUtilities::Vector3(0.0f, 0.0f, 0.0f));
    m_ADrakePistol->getComponent<Transform>()->setRotation(EngineUtilities::Vector3(0.0f, 0.0f, 0.0f));
    m_ADrakePistol->getComponent<Transform>()->setScale(EngineUtilities::Vector3(1.0f, 1.0f, 1.0f));
  } else {
    ERROR("Main", "InitDevice", "Failed to create Drake Pistol Actor.");
		return E_FAIL;
  }

  // Set Plane Actor
  m_APlane = EngineUtilities::MakeShared<Actor>(m_device);
  
  if (!m_APlane.isNull()) {
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

    planeMesh.m_numVertex = 4;
		planeMesh.m_numIndex = 6;

    // Cargar la textura
    hr = m_PlaneTexture.init(m_device, "Textures/Default", DDS);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize DrakePistol Texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }
    std::vector<MeshComponent> PlaneMeshes;
    PlaneMeshes.push_back(planeMesh);
    std::vector<Texture> PlaneTextures;
    PlaneTextures.push_back(m_PlaneTexture);
    m_APlane->setMesh(m_device, PlaneMeshes);
    m_APlane->setTextures(PlaneTextures);

    m_APlane->getComponent<Transform>()->setPosition(EngineUtilities::Vector3(0.0f, -5.0f, 0.0f));
    m_APlane->getComponent<Transform>()->setRotation(EngineUtilities::Vector3(0.0f, 0.0f, 0.0f));
    m_APlane->getComponent<Transform>()->setScale(EngineUtilities::Vector3(1.0f, 1.0f, 1.0f));
    m_actors.push_back(m_APlane);
  }
  else {
    ERROR("Main", "InitDevice", "Failed to create Plane Actor.");
    return E_FAIL;
  }

  // Crear los constant buffers (Camera)
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
	//ImGui::ShowDemoWindow(&show_demo_window);
  m_userInterface.inspectorGeneral(m_actors[0]);

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

	// Update Drake Pistol Actor
  for (auto& actor : m_actors) {
    actor->update(0, m_deviceContext);
  }

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

  //------------- Renderizar Plano-------------//
  // Asignar buffers constantes
  m_neverChanges.render(m_deviceContext, 0, 1);
  m_changeOnResize.render(m_deviceContext, 1, 1);

  //------------- Renderizar la Pistola-------------//
  for(auto& actor : m_actors) {
    actor->render(m_deviceContext);
	}

  //------------- Renderizar la sombra del cubo -------------//
  m_shaderShadow.render(m_deviceContext, PIXEL_SHADER);

  m_shadowBlendState.render(m_deviceContext, blendFactor, 0xffffffff);
  m_shadowDepthStencilState.render(m_deviceContext, 0);

  // Asignar buffers Vertex e Index
  m_vertexBuffer.render(m_deviceContext, 0, 1);
  m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

  // Asignar buffers constantes
  m_constShadow.render(m_deviceContext, 2, 1, true);

  //m_deviceContext.m_deviceContext->DrawIndexed(DrakePistol.m_index.size(), 0, 0);

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

  if (m_pSamplerLinear) m_pSamplerLinear->Release();

  m_neverChanges.destroy();
  m_changeOnResize.destroy();

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