#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

class 
DepthStencilState {
public:
	DepthStencilState()  = default;
	~DepthStencilState() = default;

	HRESULT 
	init(Device& device, bool enableDepth = true, bool enableStencil = false);

	void 
	update(); 

	void 
	render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);

	void 
	destroy();

private:
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
};