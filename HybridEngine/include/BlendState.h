#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

class 
BlendState {
public:
	BlendState()  = default;
	~BlendState() = default;

	HRESULT
	init(Device& device);

	void 
		update() {};

	void 
	render(DeviceContext& deviceContext, 
				 float* blendFactor = nullptr, 
				 unsigned int sampleMask = 0xffffffff,
				 bool reset = false);

	void 
	destroy();

private:
	ID3D11BlendState* m_blendState = nullptr;
};