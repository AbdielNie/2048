#include "MyDirectX.h"
#include <iostream>

using namespace std;

LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DSURFACE9 backbuffer = NULL;

LPDIRECTINPUT8 dinput = NULL;
LPDIRECTINPUTDEVICE8 dimouse = NULL;
LPDIRECTINPUTDEVICE8 dikeyboard = NULL;
DIMOUSESTATE mouse_state;
char keys[256];
XINPUT_GAMEPAD controllers[4];
LPD3DXSPRITE spriteobj;

bool Direct3D_Init(HWND window, int width, int height, bool fullscreen)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d) return false;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = (!fullscreen);
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.hDeviceWindow = window;

	d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (!d3ddev) return false;
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	D3DXCreateSprite(d3ddev, &spriteobj);
	return true;
}

void Direct3D_Shutdown()
{
	if (spriteobj)spriteobj->Release();
	if (d3ddev) d3ddev->Release();
	if (d3d) d3d->Release();
}

void DrawSurface(LPDIRECT3DSURFACE9 dest, float x, float y, LPDIRECT3DSURFACE9 source)
{
	D3DSURFACE_DESC desc;
	source->GetDesc(&desc);

	RECT source_rect = { 0,0,(long)desc.Width,(long)desc.Height };
	RECT dest_rect = { (long)x,(long)y,(long)x + desc.Width,(long)y + desc.Height };

	d3ddev->StretchRect(source, &source_rect, dest, &dest_rect, D3DTEXF_NONE);
}

LPDIRECT3DSURFACE9 LoadSurface(string filename)
{
	LPDIRECT3DSURFACE9 image = NULL;

	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK) return NULL;
	result = d3ddev->CreateOffscreenPlainSurface(info.Width, info.Height, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &image, NULL);
	if (result != D3D_OK) return NULL;
	result = D3DXLoadSurfaceFromFile(image, NULL, NULL, filename.c_str(), NULL, D3DX_DEFAULT, D3DCOLOR_XRGB(0, 0, 0), NULL);
	if (result != D3D_OK)return NULL;
	return image;
}

LPDIRECT3DTEXTURE9 LoadTexture(string filename,D3DCOLOR transcolor,int width,int height)
{
	LPDIRECT3DTEXTURE9 texture = NULL;

	D3DXIMAGE_INFO info;
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK) return NULL;
	if(width == 0 && height == 0)
	    D3DXCreateTextureFromFileEx(d3ddev, filename.c_str(), info.Width,info.Height,1,D3DPOOL_DEFAULT,D3DFMT_UNKNOWN,D3DPOOL_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,transcolor,&info,NULL,&texture);
	else
		D3DXCreateTextureFromFileEx(d3ddev, filename.c_str(), width,height, 1, D3DPOOL_DEFAULT, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, transcolor, &info, NULL, &texture);
	if (result != D3D_OK)return NULL;
	return texture;
}

D3DXVECTOR2 GetBitmapSize(string filename)
{
	D3DXIMAGE_INFO info;
	D3DXVECTOR2 size = D3DXVECTOR2(0.0f, 0.0f);
	HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
	if (result != D3D_OK)
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);
	else
		size = D3DXVECTOR2((float)info.Width, (float)info.Height);
		return size;
}

void Sprite_Draw_Frame(LPDIRECT3DTEXTURE9 texture,int destx,int desty,int framenum,int framew,int frameh,int columns)
{
	D3DXVECTOR3 position((float)destx, (float)desty, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	RECT rect;
	rect.left = (framenum%columns)*framew;
	rect.top = (framenum / columns)*frameh;
	rect.right = rect.left + framew;
	rect.bottom = rect.top + frameh;
	spriteobj->Draw(texture, &rect, NULL, &position, white);
}

bool Sprite_Animate(int & frame, int startframe, int endframe, int direction, int &starttime, int delay,bool repeat)
{
	if ((int)GetTickCount() > starttime + delay)
	{
		starttime = GetTickCount();
		frame += direction;
		if (frame > endframe)
		{
			if (repeat)
			{
				frame = startframe;
				return false;
			}
			return true;
		}
		if (frame < startframe) 
		{ 
			frame = endframe; 
			return false;
		}
	}
	return false;
}

bool DirectInput_Init(HWND hwnd)
{
	HRESULT result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);
	dinput->CreateDevice(GUID_SysKeyboard, &dikeyboard, NULL);
	dikeyboard->SetDataFormat(&c_dfDIKeyboard);
	dikeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dikeyboard->Acquire();

	dinput->CreateDevice(GUID_SysMouse, &dimouse, NULL);
	dimouse->SetDataFormat(&c_dfDIMouse);
	dimouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	dimouse->Acquire();

	d3ddev->ShowCursor(false);
	return true;
}

void DirectInput_Update()
{
	HRESULT result;
	result = dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);	
	if (result == DIERR_INPUTLOST)
	{
		result = dimouse->Acquire();
		dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);
	}
	if (result == DIERR_NOTACQUIRED)
	{
		DirectInput_ReAcquire();
	}
	dikeyboard->GetDeviceState(sizeof(keys), (LPVOID)&keys);
	for (int i = 0; i < 4; i++)
	{
		ZeroMemory(&controllers[i], sizeof(XINPUT_STATE));
		XINPUT_STATE state;
		DWORD result = XInputGetState(i, &state);
		if (result == 0) controllers[i] = state.Gamepad;
	}
}

int Mouse_X()
{
	return mouse_state.lX;
}

int Mouse_Y()
{
	return mouse_state.lY;
}

int Mouse_Button(int button)
{
	return mouse_state.rgbButtons[button] & 0x80;
}

int Key_Down(int key)
{
	return (keys[key] & 0x80);
}

void DirectInput_ReAcquire()
{ 
	dikeyboard->Acquire();
	dimouse->Acquire();
}

void DirectInput_Shutdown()
{
	if (dikeyboard)
	{
		dikeyboard->Unacquire();
		dikeyboard->Release();
		dikeyboard = NULL;
	}
	if (dimouse)
	{
		dimouse->Unacquire();
		dimouse->Release();
		dimouse = NULL;
	}
}

bool XInput_Controller_Found()
{
	XINPUT_CAPABILITIES caps;
	ZeroMemory(&caps, sizeof(XINPUT_CAPABILITIES));
	XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps);
	if (caps.Type != 0)return false;
	return true;
}

void XInput_Vibrate(int contNum, int amount)
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = amount;
	vibration.wRightMotorSpeed = amount;
	XInputSetState(contNum, &vibration);
}

LPD3DXFONT MakeFont(string name, int size)
{
	LPD3DXFONT font = NULL;
	D3DXFONT_DESC desc = { size,0,0,0,false,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"" };
	strcpy_s(desc.FaceName, name.c_str());
	D3DXCreateFontIndirect(d3ddev, &desc, &font);
	return font;
}

void FontPrint(LPD3DXFONT font, int x, int y, string text, D3DCOLOR color)
{
	RECT rect = { x,y,0,0 };
	font->DrawText(NULL, text.c_str(), text.length(), &rect, DT_CALCRECT, color);
	font->DrawText(spriteobj, text.c_str(), text.length(), &rect, DT_LEFT, color);
}
