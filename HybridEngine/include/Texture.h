#pragma once
#include "Prerequisites.h"

class 
Texture {
public:
	Texture()  = default;
	~Texture() = default;

	void
	init();
	
	void 
	update();
	
	void 
	render();
	
	void 
	destroy();

public:
	// This variable is in charge of handle a texture resource as data
	ID3D11Texture2D* m_texture = nullptr;
	// This variable is in charge of handle a texture resource as image data
	ID3D11ShaderResourceView* m_textureFromImg;
};