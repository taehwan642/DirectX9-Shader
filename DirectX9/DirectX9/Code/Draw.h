#ifndef __draw_h__
#define __draw_h__

#include "stdafx.h"
#include <d3d9.h>
#include <d3dx9.h>

namespace DrawTriangle
{
	HRESULT Initialize(LPDIRECT3DDEVICE9 device);
	void Update(LPDIRECT3DDEVICE9 device);
	void Close(LPDIRECT3DDEVICE9 device);
}

namespace DrawTexture
{
	HRESULT Initialize(LPDIRECT3DDEVICE9 device);
	void Update(LPDIRECT3DDEVICE9 device);
	void Close(LPDIRECT3DDEVICE9 device);
}

#endif __draw_h__