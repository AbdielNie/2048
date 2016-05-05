#include "MyDirectX.h"

const int SCREENW = 450;
const int SCREENH = 510;

LPD3DXFONT GameFont = NULL;
LPD3DXFONT GameSmallFont = NULL;
LPDIRECT3DSURFACE9 background = NULL;

const unsigned int TILE_SIZE = 4;
const unsigned int MAX = 2048;
const string APPTITLE = "2048 for DirectX";
unsigned int score = 0;
unsigned int maxscore = 0;

unsigned int Dealtime = 0;
unsigned int FlushTime = 0;
unsigned short Dealtime_note = 0;
unsigned int Endtime = 0;
unsigned int MoveData[8][7];
unsigned short ShowData[8][2];
unsigned int Showtime = 0;
unsigned int Movetime = 0;

bool win = false;
bool gameend = false;
short direction = 0;
bool canmove = false;
bool sw = true;
bool ccon = false;
bool newblock = false;
bool keepplay = false;

unsigned int block[TILE_SIZE][TILE_SIZE];
bool block_show[TILE_SIZE][TILE_SIZE];

bool Game_EndInfo(bool,HWND);
bool IsEmpty();
void ShowBlock();
void RandBlock();
bool DealBlock(unsigned int);
void GetBlockColor(int, int, int &, int &, int &);
RECT GetBlockRect(int, int);
void ShowBlockNum(int , int , RECT );
short TestBlock();
void DataInit();
void Animate_ShowBlock();
void Animate_MoveBlock();
bool LogBlockShow(int id);
bool LogBlockMove(int, int);
int NumConvertId(int num1, int num2);
void IdConvertNum(int id, int &, int &);
bool ShowInfoBar(bool order,bool);
void AddScore(unsigned int);
bool canplace();
bool TestMoveBlock(int,int);
bool ReadMaxScore();
bool WriteMaxScore();

bool Game_Init(HWND window)
{
	if (!Direct3D_Init(window, SCREENW, SCREENH, false))
	{
		MessageBox(window, "Initlize Direct3D failed.", "error", MB_OK);
		return false;
	}
	if (!DirectInput_Init(window))
	{
		MessageBox(window, "Initlize DirectInput failed.", "error", MB_OK);
		return false;
	}
	GameFont = MakeFont("微软雅黑", 48);
	GameSmallFont = MakeFont("微软雅黑", 36);
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

	srand((unsigned int)time(NULL));
	if (!ReadMaxScore())
		maxscore = 0; 
	DataInit();
	return true; 
} 

void Game_Run(HWND window) 
{ 
	if (!d3ddev) return; 
	DirectInput_Update(); 

	if (d3ddev->BeginScene()) 
	{ 	
		d3ddev->ColorFill(backbuffer, NULL, D3DCOLOR_XRGB(160,160,160));
		//背景色
		spriteobj->Begin(D3DXSPRITE_ALPHABLEND);
		{
			RECT rect = { 10,70,110,170 };
			int r = 220, g = 220, b = 220;

			if (IsEmpty() || canmove && TestMoveBlock(-1,-1)) RandBlock();
			//初始化产生
			ShowBlock();
			for (int m = 0; m < TILE_SIZE; m++)
			{
				for (int i = 0; i < TILE_SIZE; i++)
				{
					if (TestMoveBlock(m,i))
					{
						d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(190,190,190));
					}
					else
					{
						GetBlockColor(m, i, r, g, b);
						d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(r, g, b));
						ShowBlockNum(m, i, rect);
					}
					rect.left += 110;
					rect.right += 110;
				}
				rect.left = 10;
				rect.right = 110;
				rect.top += 110;
				rect.bottom += 110;
			}
		}
		//绘制方块与数字，颜色

		Animate_MoveBlock();
		Animate_ShowBlock();
		//绘制动画
		{
			short returnval = TestBlock();
			if (returnval != 0)
			{
				gameend = true;
				if (returnval == 1) win = true;
				if (returnval == -1) win = false;	
			}
			//胜败判断
		}
		if (gameend)
		{
			if (ShowInfoBar(true,sw))
			{
				sw = false;
				if (Game_EndInfo(win, window))
				{
					if (ShowInfoBar(false, sw))
					{
						if (!keepplay || !win)
							DataInit();
						else
						{
							gameend = false;
							ccon = false;
						}
						sw = true;
					}
				}
			}
		}
		//胜败消息显示
		spriteobj->End(); 
		d3ddev->EndScene(); 
		d3ddev->Present(NULL, NULL, NULL, NULL); 
	} 

	if (!gameend)
	{
		if (Key_Down(DIK_LEFT) || Key_Down(DIK_A))
			DealBlock(4);
		if (Key_Down(DIK_RIGHT) || Key_Down(DIK_D))
			DealBlock(6);
		if (Key_Down(DIK_UP) || Key_Down(DIK_W))
			DealBlock(8);
		if (Key_Down(DIK_DOWN) || Key_Down(DIK_S))
			DealBlock(2);
	}
	if (Key_Down(DIK_ESCAPE)) 
		gameover = true; 
	//键盘控制
} 

bool Game_EndInfo(bool win, HWND window)
{
	RECT rect = { 0,180,450,330 };
	POINT p;

	if (ccon)
		return true;

	GetCursorPos(&p);
	ScreenToClient(window, &p);

	GameFont->Release();
	GameSmallFont->Release();
	GameFont = MakeFont("微软雅黑", 72);
	GameSmallFont = MakeFont("微软雅黑", 24);

	d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(0, 191, 255));
	if (win)
	{
		FontPrint(GameFont, 110, 190, "You Win!", D3DCOLOR_XRGB(0, 0, 0));
		FontPrint(GameSmallFont, 255, 280, "New Game", D3DCOLOR_XRGB(0, 0, 0));
		FontPrint(GameSmallFont, 105, 280, "Continue", D3DCOLOR_XRGB(0, 0, 0));
	}
	else
	{
		FontPrint(GameFont, 80, 190, "Game Over", D3DCOLOR_XRGB(0, 0, 0));
		FontPrint(GameSmallFont, 180, 280, "Try Again", D3DCOLOR_XRGB(0, 0, 0));
	}
	if (score > maxscore)
	{
		maxscore = score;
		if (!WriteMaxScore())
			MessageBox(NULL, "Error:Write File failed.", "Error", MB_OK | MB_ICONERROR);
	}

	if (win)
	{
		RECT rect2;
		rect.left = 85;
		rect.top = 275;
		rect.right = 200;
		rect.bottom = 310;
		rect2.left = 245;
		rect2.top = 275;
		rect2.right = 360;
		rect2.bottom = 310;
		if (p.x > 97 && p.x < 176 && p.y > 253 && p.y < 285)
		{
			d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(200, 200, 200));
			if (Mouse_Button(0) & 0x80)
			{
				keepplay = true;
				ccon = true;
				win = false;
			}
		}
		else
			d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(156, 156, 156));
		if (p.x > 246 && p.x < 339 && p.y > 253 && p.y < 285)
		{
			d3ddev->ColorFill(backbuffer, &rect2, D3DCOLOR_XRGB(200, 200, 200));
			if (Mouse_Button(0) & 0x80)
			{
				ccon = true;
			}
		}
		else
			d3ddev->ColorFill(backbuffer, &rect2, D3DCOLOR_XRGB(156, 156, 156));
	}
	else
	{
		rect.left = 170;
		rect.top = 275;
		rect.right = 275;
		rect.bottom = 310;
		if (p.x > 164 && p.x < 265 && p.y > 253 && p.y < 285)
		{
			d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(200, 200, 200));
			if (Mouse_Button(0) & 0x80)
			{
				ccon = true;
			}
		}
		else
			d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(156, 156, 156));
	}

	GameFont->Release();
	GameSmallFont->Release();
	GameFont = MakeFont("微软雅黑", 48);
	GameSmallFont = MakeFont("微软雅黑", 36);
	return false;
} 

void ShowBlock() 
{ 
	string sscore = "Score:", smscore = "MaxScore:";
	char Cscore[5]; 
	sprintf_s(Cscore, "%u", score); 
	sscore += Cscore; 
	FontPrint(GameSmallFont, 10, 10, sscore, D3DCOLOR_XRGB(255, 255, 255)); 
	if (maxscore >= score)
	{
		sprintf_s(Cscore, "%u", maxscore);
		smscore += Cscore;
	}
	else
		smscore += Cscore;
	FontPrint(GameSmallFont, 240, 10, smscore, D3DCOLOR_XRGB(187, 255, 255));
}

void GetBlockColor(int num1,int num2, int & r, int & g, int & b)
{
	int temp = 0;
	if (num1 < 0)
	{
		temp = num2;
		if (num2 == 0 || num2 < 0)
		{
			MessageBox(NULL, "num2 error", "info", MB_OK);
		}
	}
	else
		temp = block[num1][num2];
	if (temp == 2)
	{
		r = 255;
		g = 228;
		b = 255;
	}
	else if (temp == 4)
	{
		r = 255;
		g = 255;
		b = 240;
	}
	else if (temp == 8)
	{
		r = 255;
		g = 228;
		b = 181;
	}
	else if (temp == 16)
	{
		r = 255;
		g = 218;
		b = 185;
	}
	else if (temp == 32)
	{
		r = 255;
		g = 160;
		b = 122;
	}
	else if (temp == 64)
	{
		r = 255;
		g = 128;
		b = 114;
	}
	else if (temp == 128)
	{
		r = 255;
		g = 99;
		b = 71;
	}
	else if (temp == 256)
	{
		r = 178;
		g = 34;
		b = 34;
	}
	else if (temp == 512)
	{
		r = 255;
		g = 69;
		b = 0;
	}
	else if (temp == 1024)
	{
		r = 255;
		g = 30;
		b = 0;
	}
	else if (temp == 2048)
	{
		r = 255;
		g = 0;
		b = 0;
	}
	else if (temp == 0)
	{
		r = g = b = 190;
	}
}

void ShowBlockNum(int num1,int num2,RECT rect)
{
	if (num2 != -1)
	{
		if (block[num1][num2] != 0)
		{
			char str[5];
			sprintf_s(str, "%u", block[num1][num2]);
			string s = str;
			if (num1 == 1 && gameend)
				return;
			GameSmallFont->DrawText(spriteobj, s.c_str(), s.length(), &rect, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0, 0, 0));
		}
	}
	else
	{
		char str[5];
		sprintf_s(str, "%u", MoveData[num1][5]);
		string s = str;
		GameSmallFont->DrawText(spriteobj, s.c_str(), s.length(), &rect, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0, 0, 0));
	}
}

bool ShowInfoBar(bool order,bool switches)
{
	if (order != switches)
		return true;
	if (Dealtime_note == 75)
	{
		Dealtime_note = 0;
		Dealtime = 0;
		return true;
	}
	RECT rect;

	if (order)
	{
		rect.left = 0;
		rect.top = 255 - Dealtime_note;
		rect.right = 450;
		rect.bottom = 255 + Dealtime_note;
	}
	else
	{
		rect.left = 0;
		rect.top = 180 + Dealtime_note;
		rect.right = 450;
		rect.bottom = 330 - Dealtime_note;
	}
	d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(0, 191, 255));
	if ((int)timeGetTime() < Dealtime + 1)
		return false;
	else
	{
		Dealtime = timeGetTime();
		Dealtime_note += 5;
	}
	return false;
}

void Game_End()
{
	if (GameFont)GameFont->Release();
	if (GameSmallFont)GameSmallFont->Release();
	if (background)background->Release();
	DirectInput_Shutdown();
	Direct3D_Shutdown();
}

//////////////////////////////////////////////////////////
//游戏动画部分
//////////////////////////////////////////////////////////

bool LogBlockShow(int id)
{
	int i = 0;
	for (; i < 8; i++)
	{
		if (ShowData[i][0] == id)
			break;
	}
	//检测是否已登记
	if (i == 8)
	{
		for (int s = 0; s < 8; s++)
		{
			if (ShowData[s][0] == 0)
			{
				ShowData[s][0] = id;
				return true;
			}
		}
	}
	//若没有，则登记
	return false;
}

void Animate_ShowBlock()
{
	if (timeGetTime() < Showtime + 10)
		return;
	else
	{
		Showtime = timeGetTime();
		for (int n = 0; n < 8; n++)
		{
			if (ShowData[n][0] != 0)
			{
				if (ShowData[n][1] > 60)
				{
					ShowData[n][0] = 0;
					ShowData[n][1] = 0;
				}
				else
					ShowData[n][1] += 5;
			}
		}
	}

	RECT rect;
	int num1, num2, r, g, b;

	for (int i = 0; i < 8; i++)
	{
		if (ShowData[i][0] != 0)
		{
			bool pre_return = false;
			IdConvertNum(ShowData[i][0], num1, num2);
			for (int s = 0; s < 8; s++)
			{
				if (MoveData[s][0] == ShowData[i][0] && MoveData[s][1] < MoveData[s][2] * 110 - 50)
					pre_return = true;
			}
			if (pre_return)
				continue;
			GetBlockColor(num1, num2, r, g, b);
			rect = GetBlockRect(num1, num2);
			rect = { rect.left + 50,rect.top + 50,rect.right - 50,rect.bottom - 50 };
			if (ShowData[i][1] <= 55)
			{
				rect.left -= ShowData[i][1];
				rect.top -= ShowData[i][1];
				rect.right += ShowData[i][1];
				rect.bottom += ShowData[i][1];
			}
			else if (ShowData[i][1] <= 60)
			{
				rect = { rect.left - 55,rect.top - 55,rect.right + 55,rect.bottom + 55 };
				int x = 5 - (60 - ShowData[i][1]);
				rect.left += x;
				rect.top += x;
				rect.right -= x;
				rect.bottom -= x;
			}
			if (ShowData[i][1] < 60)
				d3ddev->ColorFill(backbuffer, &GetBlockRect(num1, num2), D3DCOLOR_XRGB(190, 190, 190));
			d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(r, g, b));
		}
	}
	//方块动画效果
}

RECT GetBlockRect(int num1,int num2)
{
	RECT rect;
	rect.left = 110 * num2 + 10;
	rect.top = num1 * 110 + 70;
	rect.bottom = rect.top + 100;
	rect.right = rect.left + 100;
	return rect;
}

int NumConvertId(int num1, int num2)
{
	return num1 * 4 + num2 + 1;
}

void IdConvertNum(int id,int &num1, int &num2)
{
	if (id <= 4)
		num1 = 0;
	if (id > 4 && id <= 8)
		num1 = 1;
	if (id > 8 && id <= 12)
		num1 = 2;
	if (id > 12 && id <= 16)
		num1 = 3;
	num2 = id - num1 * 4 - 1;
}

bool LogBlockMove(int sourc_id,int desc_id)
{
	int i = 0;
	for (; i < 8; i++)
	{
		if (MoveData[i][0] == sourc_id)
			break;
	}
	//检测是否已登记
	if (i == 8)
	{
		for (int s = 0; s < 8; s++)
		{
			if (MoveData[s][0] == 0)
			{
				int n1, n2, n3, n4;
				IdConvertNum(sourc_id, n1, n2);
				IdConvertNum(desc_id, n3, n4);
				MoveData[s][0] = sourc_id;
				MoveData[s][4] = desc_id;
				MoveData[s][5] = block[n1][n2];
				MoveData[s][6] = block[n3][n4];
				short diff = sourc_id - desc_id;
				if (diff <= 3 && diff >= -3)
					MoveData[s][2] = abs(desc_id - sourc_id);
				else
					MoveData[s][2] = abs((desc_id - sourc_id) / 4);
				if (diff <= 12 && diff >= 4)
					direction = 1; //up
				if (diff <= -4 && diff >= -12)
					direction = 2; //down
				if (diff <= 3 && diff >= 1)
					direction = 3; //left
				if (diff <= -1 && diff >= -3)
					direction = 4; //riight
				return true;
			}
		}
	}
	//若没有，则登记
	return false;
}

void Animate_MoveBlock()
{
	if (timeGetTime() < Movetime + 1)
		return;
	else
	{
		Movetime = timeGetTime();
		for (int n = 0; n < 8; n++)
		{
			if (MoveData[n][0] != 0)
			{
				int n1, n2, n3, n4;
				IdConvertNum(MoveData[n][0], n1, n2);
				IdConvertNum(MoveData[n][4], n3, n4);

				if (MoveData[n][1] >= MoveData[n][2] * 110)
				{
					for (int i = 0; i < 7; i++)
						MoveData[n][i] = 0;
				}
				else
					MoveData[n][1] += 30;
			}
			//初始化已使用的数据。
		}
	}

	RECT rect;
	int num1, num2, r = 0, g = 0, b = 0;

	for (int i = 0; i < 8; i++)
	{
		if (MoveData[i][0] != 0)
		{
			int num3, num4;
			IdConvertNum(MoveData[i][0], num1, num2);
			GetBlockColor(-(i+1), MoveData[i][5], r, g, b);
			rect = GetBlockRect(num1, num2);
			if (MoveData[i][1] <= MoveData[i][2] * 110)
			{
				if (direction == 1)
				{
					rect.top -= MoveData[i][1];
					rect.bottom -= MoveData[i][1];
				}
				if (direction == 2)
				{
					rect.top += MoveData[i][1];
					rect.bottom += MoveData[i][1];
				}
				if (direction == 3)
				{
					rect.left -= MoveData[i][1];
					rect.right -= MoveData[i][1];
				}
				if (direction == 4)
				{
					rect.left += MoveData[i][1];
					rect.right += MoveData[i][1];
				}
				d3ddev->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(r, g, b));
				ShowBlockNum(i, -1, rect);
			}
		}
	}
	//方块移动动画效果
}

bool TestMoveBlock(int n1,int n2)
{
	if (n1 == n2 && n2 == -1)
	{
		if (IsEmpty())
			return false;
		for (int i = 0; i < 8; i++)
		{
			if (MoveData[i][0] != 0)
				return false;
		}
		return true;
	}
	for (int i = 0; i < 8; i++)
	{
		if (MoveData[i][0] != 0)
		{
			if (MoveData[i][0] == NumConvertId(n1, n2) || MoveData[i][4] == NumConvertId(n1, n2))
			{
				if (MoveData[i][5] != 0 || MoveData[i][6] != 0)
					return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////
//游戏逻辑部分
//////////////////////////////////////////////////////////

bool DealBlock(unsigned int num)
{
	if (timeGetTime() > Dealtime + 200)
		Dealtime = timeGetTime();
	else
		return false;
	if (num != 2 && num != 4 && num != 6 && num != 8)
		return false;
	if (num == 8)	// up
	{
		for (int s = 0; s < TILE_SIZE; s++)
		{
			for (int t = 0; t < TILE_SIZE; t++)
			{
				if (block[t][s] == 0)
				{
					for (int m = t + 1; m < TILE_SIZE; m++)
					{
						if (block[m][s] != 0)
						{
							LogBlockMove(NumConvertId(m, s), NumConvertId(t, s));
							block[t][s] = block[m][s];
							block[m][s] = 0;
							canmove = true;
							break;
						}
					}
				}
				if (block[t][s] != 0)
				{
					for (int m = t + 1; m < TILE_SIZE; m++)
					{
						if (block[m][s] == block[t][s])
						{
							LogBlockMove(NumConvertId(m, s), NumConvertId(t, s));
							block[t][s] += block[t][s];
							LogBlockShow(NumConvertId(t, s));
							block[m][s] = 0;
							AddScore(block[t][s]);
							canmove = true;
							break;
						}
						if (block[m][s] != 0)
							break;
					}
				}
			}
		}
	}
	if (num == 2)	// down
	{
		for (int s = 0; s < TILE_SIZE; s++)
		{
			for (int t = TILE_SIZE - 1; t >= 0; t--)
			{
				if (block[t][s] == 0)
				{
					for (int m = t - 1; m >= 0; m--)
					{
						if (block[m][s] != 0)
						{
							LogBlockMove(NumConvertId(m, s), NumConvertId(t, s));
							block[t][s] = block[m][s];
							block[m][s] = 0;
							canmove = true;
							break;
						}
					}
				}
				if (block[t][s] != 0)
				{
					for (int m = t - 1; m >= 0; m--)
					{
						if (block[m][s] == block[t][s])
						{
							LogBlockMove(NumConvertId(m, s), NumConvertId(t, s));
							block[t][s] += block[t][s];
							LogBlockShow(NumConvertId(t, s));
							block[m][s] = 0;
							AddScore(block[t][s]);
							canmove = true;
							break;
						}
						if (block[m][s] != 0)
							break;
					}
				}
			}
		}
	}
	if (num == 4)	// left
	{
		for (int s = 0; s < TILE_SIZE; s++)
		{
			for (int t = 0; t < TILE_SIZE; t++)
			{
				if (block[s][t] == 0)
				{
					for (int m = t + 1; m < TILE_SIZE; m++)
					{
						if (block[s][m] != 0)
						{
							block[s][t] = block[s][m];
							LogBlockMove(NumConvertId(s, m), NumConvertId(s, t));
							block[s][m] = 0;
							canmove = true;
							break;
						}
					}
				}
				if (block[s][t] != 0)
				{
					for (int m = t + 1; m < TILE_SIZE; m++)
					{
						if (block[s][m] == block[s][t])
						{
							block[s][t] += block[s][t];
							LogBlockMove(NumConvertId(s, m), NumConvertId(s, t));
							LogBlockShow(NumConvertId(s, t));
							block[s][m] = 0;
							AddScore(block[s][t]);
							canmove = true;
							break;
						}
						if (block[s][m] != 0)
							break;
					}
				}
			}
		}
	}
	if (num == 6)	// right
	{
		for (int s = 0; s < TILE_SIZE; s++)
		{
			for (int t = TILE_SIZE - 1; t >= 0; t--)
			{
				if (block[s][t] == 0)
				{
					for (int m = t - 1; m >= 0; m--)
					{
						if (block[s][m] != 0)
						{
							block[s][t] = block[s][m];
							LogBlockMove(NumConvertId(s, m), NumConvertId(s, t));
							block[s][m] = 0;
							canmove = true;
							break;
						}
					}
				}
				if (block[s][t] != 0)
				{
					for (int m = t - 1; m >= 0; m--)
					{
						if (block[s][m] == block[s][t])
						{
							block[s][t] += block[s][t];
							LogBlockMove(NumConvertId(s, m), NumConvertId(s, t));
							LogBlockShow(NumConvertId(s, t));
							block[s][m] = 0;
							AddScore(block[s][t]);
							canmove = true;
							break;
						}
						if (block[s][m] != 0)
							break;
					}
				}
			}
		}
	}
	return false;
}

bool canplace()
{
	for (int i = 0; i < TILE_SIZE; i++)
	{
		for (int m = 0; m < TILE_SIZE; m++)
		{
			if (!block_show[i][m])
				return false;
		}
	}
	return true;
}

//失败检测
short TestBlock()
{
	int m = 0;

	if (!keepplay)
	{
		for (int i = 0; i < TILE_SIZE; i++)
		{
			for (int m = 0; m < TILE_SIZE; m++)
			{
				if (block[i][m] == MAX)
					return 1;
			}
		}
	}
	// 检测是否获胜
	for (int n = 0; n < 4; n++)
	{
		for (int a = 0; a < TILE_SIZE; a++)
		{
			for (int b = 0; b < TILE_SIZE; b++)
			{			
				if (n == 0)		// up
				{
					if (a != 0)
					{
						if (block[a][b] == block[a - 1][b] || block[a][b] == 0)
							return 0;
					}
				}
				if (n == 1)		// down?				
				{
					if (a != 3)
					{
						if (block[a][b] == block[a + 1][b] || block[a][b] == 0)
							return 0;
					}
				}
				if (n == 2)		// left
				{
					if (b != 0)
					{
						if (block[a][b] == block[a][b - 1] || block[a][b] == 0)
							return 0;
					}
				}
				if (n == 3)		// right
				{
					if (b != 3)
					{
						if (block[a][b] == block[a][b + 1] || block[a][b] == 0)
							return 0;
					}
				}
			}
		}
	}
	//检测是否失败
	return -1;
}
bool IsEmpty()
{
	for (int i = 0; i < TILE_SIZE; i++)
	{
		for (int m = 0; m < TILE_SIZE; m++)
		{
			if (block[i][m] != 0)
				return false;
		}
	}
	return true;
}

void RandBlock()
{
	int a = 0, c = 0;
	while (true)
	{
		a = rand() % TILE_SIZE;
		c = rand() % TILE_SIZE;
		if (block[a][c] == 0)
		{
			if (rand() % 5 <= 3)
				block[a][c] = 2;
			else
				block[a][c] = 4;
			int id = NumConvertId(a, c);
			LogBlockShow(id);
			canmove = false;
			return;
		}
	}
}

void AddScore(unsigned int num)
{
	for (int i = 1; i <= 11; i++)
	{
		if (num == pow(2,i))
		{
			score += i;
			break;
		}
	}
}

void DataInit()
{
	score = 0;
	for (int i = 0; i < TILE_SIZE; i++)
	{
		for (int t = 0; t < TILE_SIZE; t++)
		{
			block[i][t] = 0; 
			block_show[i][t] = true;
		}
	}
	for (int i = 0; i < 8; i++)
	{
		for (int m = 0; m < 2; m++)
		{
			ShowData[i][m] = 0;
		}
		for (int m = 0; m < 7; m++)
		{
			MoveData[i][m] = 0;
		}
	}
	gameend = false;
	ccon = false;
	keepplay = false;
}
