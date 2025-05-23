#pragma once
#include "Prerequisites.h"

class 
DeviceContext {
public:
	DeviceContext()  = default;
	~DeviceContext() = default;

	void
	init();
	
	void 
	update();
	
	void 
	render();
	
	void 
	destroy();

public:
	ID3D11DeviceContext* m_deviceContext = nullptr;
};