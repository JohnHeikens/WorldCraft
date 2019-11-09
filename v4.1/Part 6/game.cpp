#include "game.h"

form* activeForm;

GraphicsObject graphics = GraphicsObject();//the graphics of the window

// Global Windows/Drawing variables
HBITMAP hbmp = NULL;
HWND hwnd = NULL;
HDC hdcMem = NULL;
// The window's DC
HDC wndDC = NULL;
HBITMAP hbmOld = NULL;
// Pointer to colors (will automatically have space allocated by CreateDIBSection

uint* PlaneIndices = NULL;
POINT MousePos = POINT();
POINT LastPoint;
//mobs
human* player;
std::list<mob*> mobs;

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif
constexpr fp minZplane = 0.01;
fp ScreenProportion;
fp PlayerHeight = 1.8;

gamemode mode = gamemode::creative;

constexpr int populationMargin = 1;//population margin: 1 chunk
constexpr int SaveDiametre = 0x10;//0x42;//0x10;//the range in which chunks remain in work memory
constexpr int RenderDistance = 0x6,//0x20,//0x6,
#if SaveDiametre < ((RenderDistance + populationMargin) * 2)
#error "SaveDiametre has to be more than (RenderDistance + populationMargin) * 2"
#endif
	RenderDistance2 = RenderDistance * RenderDistance;//render distance in chunks
constexpr int LoadBlocks = SaveDiametre * ChunkScaleHor;//render distance in blocks
constexpr int LoadChunksMin = SaveDiametre - 1, SaveDiametre2 = SaveDiametre * SaveDiametre, HalfLoadBlocks = LoadBlocks / -2;//a
constexpr fp MaxDistance = (RenderDistance - 1) * ChunkScaleHor;//INFINITY;
fp FogDistance;
fp LastYaw = 0;

//constexpr fp MaxDistance2 = MaxDistance * MaxDistance;
block SelectedBlock;
int lastblockx[3], lastblocky[3], lastblockz[3];
int SelectedBlockX, SelectedBlockY, SelectedBlockZ;
int AdjacentBlockX, AdjacentBlockY, AdjacentBlockZ;

seconds activeticktime = NormalTickTime;

int DesktopWidth, DesktopHeight;
int WindowX = 0, WindowY = 0;

bool DrawControls = true;
bool fog = true;
bool f5mode = false;//the player can see himself in f5 mode

int Focus = -1;

bool releasedSlash = true;
bool releasedG = true;
bool releasedM = true;
bool releasedI = true;


vec3 EyePosition;
vec3 HeadDirection;
vec3 firstraycastintersection;//the exact first intersection point of the ray cast
vec3 CameraPosition;//normally the cameraposition = eyeposition, except for f5 mode.
vec3 CameraDirection;
//mat4 TransForm; // make sure to initialize matrix to identity matrix first
//vec3 Position;
//transforms
mat4x4 projection, view, matrix;
//looking over positive x -axis
ViewFrustum ActiveFrustum;
fp
FOV = 90, Pitch, Yaw,
EyeHeight = 1.62f,
FistRange = 0x8;
color AirColor((fp).66, (fp).78, (fp)1.0);
color WaterColor(0.5, (fp).16, (fp).22, (fp).71);
color LavaColor(153, 26, 0);
//render settings
miliseconds msElapsed;
seconds LastTime, LastUpdateTime;

//uint OutWaterArrays[RenderDistance2 * RenderDistance2];
//uint WaterArrays[RenderDistance2 * RenderDistance2];
//fp smoothness = 0x40;
int LayerCount = 4;
int PlusX, PlusY, PlusChunkY, PlusChunkX;
bool IsVisible[SaveDiametre2];//determines wether a chunk is visible
Chunk* ChunkTiles[SaveDiametre2];//chunks formatted as a 2d array
std::list<Chunk*> Chunk1d[SaveDiametre2 * 2];//list with chunks unformatted
bool LeftButtonDown = false, RightButtonDown = false;
TextBox* commandline = NULL;
PictureBox* crosshair = NULL;
biomeeditor* editor = NULL;
Texture* BlockTextures;
byte ActiveItem;
itemtype ActiveItemType = itemtype::tblock;
//vec3 Speed;
//settings
bool LockMouse = true;//wether the mouse is reset to the mid of the screen

letter* ScreenShotPath = (letter*)0;
//static letter* ScreenShotPath = (letter*)"";//not const because of internal linkage
fp MouseSensitivity = 0.1;

void UpdateCursor()
{
	POINT p;
	if (GetCursorPos(&p))
	{
		//cursor position now in p.x and p.y
		if (ScreenToClient(hwnd, &p))
		{
			MousePos = p;
			//p.x and p.y are now relative to hwnd's client area
		}
	}
}
void ProcessCursor()
{
	UpdateCursor();
	if (Pressed(LOCKMOUSE_KEY)) {
		if (releasedM) {
			LockMouse = !LockMouse;
			if (LockMouse)
			{
				HideMouse();
			}
			else
			{
				ShowMouse();
			}
			releasedM = false;
		}
	}
	else
	{
		releasedM = true;
	}
	f5mode = pressed(VK_F5);
	player->visible = f5mode;

	if (Pressed(INVENTORY_KEY)) {
		if (releasedI) 
		{
			//inventory
			releasedI = false;
		}
	}
	else {
		releasedM = true;
	}
	if (Focus != commandline->index)
	{
		if (Pressed(DIG_KEY)) {
			LeftButtonDown = true;
		}
		if (Pressed(PLACE_KEY)) {
			RightButtonDown = true;
		}
	}
	POINT p = MousePos;
	int
		MouseDX = p.x - LastPoint.x,
		MouseDY = p.y - LastPoint.y;
	Yaw += MouseDX * MouseSensitivity;
	Pitch -= MouseDY * MouseSensitivity;
	if (LockMouse)
	{
		LastPoint.x = graphics.width / 2;
		LastPoint.y = graphics.height / 2;
		POINT scr = LastPoint;
		if (ClientToScreen(hwnd, &scr))
		{
			SetCursorPos(scr.x, scr.y);
		}
	}
	else
	{
		LastPoint = p;
	}
	if (IsRaycastVisible[SelectedBlock])
	{
		if (LeftButtonDown)Dig();
		if (RightButtonDown)Place();
	}
	LeftButtonDown = RightButtonDown = false;
}


void MakeSurface(HWND hwnd) {
	/* Use CreateDIBSection to make a HBITMAP which can be quickly
	 * blitted to a surface while giving 100% fast access to colors
	 * before blit.
	 */
	 // Desired bitmap properties
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);//sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth = graphics.width;
	bmi.bmiHeader.biHeight = -graphics.height; // Order colors from top to bottom
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32; // last byte not used, 32 bit for alignment
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;// width* height * 4;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiColors[0].rgbBlue = 0;
	bmi.bmiColors[0].rgbGreen = 0;
	bmi.bmiColors[0].rgbRed = 0;
	bmi.bmiColors[0].rgbReserved = 0;
	HDC hdc = GetDC(hwnd);
	graphics.colors = nullptr;
	// Create DIB section to always give direct access to colors
	hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)& graphics.colors, NULL, 0);

	DeleteDC(hdc);
	// Give plenty of time for main thread to finish setting up
	Sleep(50);//time??? without sleep, it finishes
	ShowWindow(hwnd, SW_SHOW);
	// Retrieve the window's DC
	wndDC = GetDC(hwnd);
	// Create DC with shared colors to variable 'colors'
	hdcMem = CreateCompatibleDC(wndDC);//HDC must be wndDC!! :)
	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmp);

	graphics.depthbuffer = (fp*)calloc(graphics.width * graphics.height, sizeof(fp));
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		MakeSurface(hwnd);
		break;
	case WM_MOUSEMOVE:
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// Draw colors to window when window needs repainting
		BitBlt(hdc, 0, 0, graphics.width, graphics.height, hdcMem, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		SelectObject(hdcMem, hbmOld);
		DeleteDC(wndDC);
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		LeftButtonDown = true;
		break;
	case WM_RBUTTONDOWN:
		RightButtonDown = true;
		break;
	case WM_CHAR://https://stackoverflow.com/questions/13596797/concatenation-of-pressed-wm-letter-key-values-win32api

		if (Focus == commandline->index)
		{
			switch (wParam)
			{
				// First, handle non-displayable characters by beeping.
			case 0x08:  // backspace.
				commandline->text = commandline->text.substr(0, commandline->text.size() - 1);
				break;
			case 0x09:  // tab.
			{
				std::wstring clip;
				if (GetClipboardText(clip))
				{
					commandline->text += clip;
				}
				MessageBeep((UINT)-1);
				break; 
			}
			case 0x0A:  // linefeed.
			case 0x0D:  // carriage return.
			case 0x1B:  // escape.
				MessageBeep((UINT)-1);
				break;
				// Next, handle displayable characters by appending them to our wstring.
			default:
				commandline->text += (letter)wParam;
			}
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	//get global vars
	GetDesktopResolution(graphics.width, graphics.height);
	activeForm = new form(0, 0, graphics.width, graphics.height);
	//graphics.width = 600;
	//graphics.height = 400;
	WNDCLASSEX wc;
	//MSG msg;
	// Init wc
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = CreateSolidBrush(0);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"animation_class";
	wc.lpszMenuName = NULL;
	wc.style = 0;
	// Register wc
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Failed to register window class.", L"Error", MB_OK);
		return 0;
	}
	// Make window
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		L"animation_class",
		L"Animation",
		WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP | WS_CAPTION,
		300, 200, graphics.width, graphics.height,
		NULL, NULL, hInstance, NULL);
	RECT rcClient, rcWindow;
	POINT ptDiff;
	// Get window and client sizes
	GetClientRect(hwnd, &rcClient);
	GetWindowRect(hwnd, &rcWindow);
	// Find offset between window size and client size
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	// Resize client
	MoveWindow(hwnd, rcWindow.left, rcWindow.top, graphics.width + ptDiff.x, graphics.height + ptDiff.y, false);
	UpdateWindow(hwnd);
	Run();
	/*while (GetMessage(&msg, 0, 0, NULL) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/
	return 0;
}

void Initialize()
{
	rendersettings::checkopacity = true;
	GenerateControls();
	//Image* img = 
	BlockTextures = Image::FromFile(L"HD3.bmp", true);
	//delete img;
	ScreenShotPath = new letter[1]{ 0 };
	mobs.push_back(player = new human(vec3(0, 0, 10)));
	//Position = vec3(0, 0, 10);//Maxheight);
	//unsigned int buffer;
	ScreenProportion = (fp)graphics.width / (fp)graphics.height;
	//generate buffer object
	const uint MAX_VERTICES = 0x10000 * 6;//786 thousand
	uint* indices = new uint[MAX_VERTICES];
	uint* endptr = indices + MAX_VERTICES;
	uint* ptr = indices;
	uint off = 0;
	while (ptr < endptr)
	{
		*ptr++ = off;		//0
		*ptr++ = off + 1;	//1
		*ptr++ = off + 2;	//2
		*ptr++ = off + 3;	//3
		*ptr++ = off + 2;	//2
		*ptr++ = off + 1;	//1
		off += 4;
	}
	PlaneIndices = indices;
	//destroy array
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//MouseSensitivity = 0.1;
	ActiveFrustum = ViewFrustum();

	Seed = time(NULL);
	Power = 5;
	MinE = 0;
	MaxE = 1;
	MinM = 0;
	MaxM = 1;

	InitializeTerrain();
	UpdateCursor();
	LastPoint = MousePos;
	SetWindowPos(hwnd, HWND_TOP, 0, 0, graphics.width, graphics.height, 0);
	HideMouse();
}

inline bool Pressed(int key)
{
	return GetKeyState(key) & 0x8000;
}


void Place() {
	RightButtonDown = false;
	lastblockx[2] = lastblockx[1];
	lastblocky[2] = lastblocky[1];
	lastblockz[2] = lastblockz[1];
	lastblockx[1] = lastblockx[0];
	lastblocky[1] = lastblocky[0];
	lastblockz[1] = lastblockz[0];
	lastblockx[0] = AdjacentBlockX;
	lastblocky[0] = AdjacentBlockY;
	lastblockz[0] = AdjacentBlockZ;
	switch (ActiveItemType)
	{
	case tblock:
		SetBlock(AdjacentBlockX, AdjacentBlockY, AdjacentBlockZ, (block)ActiveItem, true);
		break;
	case ttree:
		Chunk::PlaceArtifact(AdjacentBlockX, AdjacentBlockY, AdjacentBlockZ, (Artifact)ActiveItem, true);
		break;
	default:
		break;
	}
}
constexpr bool sound = false;

void Dig()
{
	LeftButtonDown = false;
	lastblockx[2] = lastblockx[1];
	lastblocky[2] = lastblocky[1];
	lastblockz[2] = lastblockz[1];
	lastblockx[1] = lastblockx[0];
	lastblocky[1] = lastblocky[0];
	lastblockz[1] = lastblockz[0];
	lastblockx[0] = SelectedBlockX;
	lastblocky[0] = SelectedBlockY;
	lastblockz[0] = SelectedBlockZ;
	if (sound && SelectedBlock)
	{
		stopsound();
		switch (blockaudio[SelectedBlock])
		{
		case auSand:
			startsound(((L"data/sounds/sand" + std::to_wstring((rand() % 4 + 1)) + L".wav")).c_str());
			break;
		case auWood:
			startsound(((L"data/sounds/wood" + std::to_wstring((rand() % 4 + 1)) + L".wav")).c_str());
			break;
		case auStone:
			startsound(((L"data/sounds/stone" + std::to_wstring((rand() % 4 + 1)) + L".wav")).c_str());
			break;
		case auGravel:
			startsound(((L"data/sounds/gravel" + std::to_wstring((rand() % 4 + 1)) + L".wav")).c_str());
			break;
		case auGrass:
			startsound(((L"data/sounds/grass" + std::to_wstring((rand() % 4 + 1)) + L".wav")).c_str());
			break;
		}

	}

	SetBlock(SelectedBlockX, SelectedBlockY, SelectedBlockZ, Air, true);
}
//determines per chunk if it will be drawn, crops the chunks to view
void CropView() {
	ActiveFrustum.update(matrix);
	int index = 0;
	fp px = (player->position.x / ChunkScaleHor) - PlusChunkX;
	fp py = (player->position.y / ChunkScaleHor) - PlusChunkY;
	vec3 ChunkSize = vec3(ChunkScaleHor, ChunkScaleHor, ChunkScaleVert);
	for (int j = 0, ChunkY = PlusY; j < SaveDiametre; j++, ChunkY += ChunkScaleHor)
	{
		fp dy0 = j - py, dy1 = dy0 + 1;
		dy0 *= dy0;
		dy1 *= dy1;
		fp dy = dy0 < dy1 ? dy0 : dy1;//squared distance on the y-axis to the camera
		for (int i = 0, ChunkX = PlusX; i < SaveDiametre; i++, index++, ChunkX += ChunkScaleHor)
		{
			fp dx0 = i - px, dx1 = dx0 + 1;
			dx0 *= dx0;
			dx1 *= dx1;
			fp dx = dx0 < dx1 ? dx0 : dx1;//squared distance on the x-axis to the camera

			fp DistanceToPlayer2 = dx + dy;
			if (DistanceToPlayer2 > RenderDistance2)
			{
				IsVisible[index] = false;
				continue;
			}
			//frustum culling
			IsVisible[index] = ActiveFrustum.isBoxInFrustum(vec3(PlusX + i * ChunkScaleHor, PlusY + j * ChunkScaleHor, 0), ChunkSize);
		}
	}
}
void SetDirection() {
	if (Pitch > 89.0f)
		Pitch = 89.0;
	if (Pitch < -89.0f)
		Pitch = -89.0;
	fp RadiansPitch = Pitch * DegToRad;
	fp RadiansYaw = -Yaw * DegToRad;
	fp CosYaw = cos(RadiansYaw);
	fp SinYaw = sin(RadiansYaw);
	fp CosPitch = cos(RadiansPitch);
	fp SinPitch = sin(RadiansPitch);
	HeadDirection = vec3(CosPitch * CosYaw, CosPitch * SinYaw, SinPitch);
	//fp l = LookDirection.length();
	vec3 eyepos = vec3(0.5, 1, 0.5);//the middle of the front of the head
	//transform must be recalculated
	bodypart* b = &player->body;
	b->rotations[0].angle = RadiansYaw - M_PI * 0.5;
	b->CalculateTransform();
	player->head->rotations[0].angle = -RadiansPitch;
	//player->head->rotations[1].angle = RadiansYaw - M_PI * 0.5;
	player->head->CalculateTransform();
	mat4x4 bodytransform = combine({ player->head->scalecentre, player->head->applied, b->applied });
	EyePosition = bodytransform.multPointMatrix(eyepos);//the position of the nose of the player
	walk_grid(EyePosition, EyePosition + HeadDirection * FistRange, SelectedBlockX, SelectedBlockY, SelectedBlockZ, AdjacentBlockX, AdjacentBlockY, AdjacentBlockZ, SelectedBlock);
	if (f5mode)
	{
		CameraDirection = -HeadDirection;
		CameraPosition = firstraycastintersection;
		CameraPosition -= HeadDirection;
	}
	else {
		CameraDirection = HeadDirection;
		CameraPosition = EyePosition;
	}
}
bool walk_grid(vec3 p0, vec3 p1, int& selx, int& sely, int& selz, int& adjx, int& adjy, int& adjz, block& hit)
{
	//https://www.redblobgames.com/grids/line-drawing.html
	//http://www.cse.yorku.ca/~amana/research/grid.pdf
	vec3 d0to1 = p1 - p0;//dx
	int sign[3] = { d0to1.x > 0 ? 1 : -1,  d0to1.y > 0 ? 1 : -1,  d0to1.z > 0 ? 1 : -1 };//sign_x
	//not dx * sign_x, because you can get -0 and -inf
	vec3 absd0to1 = d0to1.absolute();//nx
	int pos[3] = { (int)floor(p0.x),(int)floor(p0.y),(int)floor(p0.z) };//px
	int dimension = -1;
	vec3 delta = { 1 / absd0to1.x,1 / absd0to1.y,1 / absd0to1.z };//deltax
	//vx
	vec3 progress = {
	d0to1.x > 0 ? pos[0] + 1 - p0.x : p0.x - pos[0],
	d0to1.y > 0 ? pos[1] + 1 - p0.y : p0.y - pos[1],
	d0to1.z > 0 ? pos[2] + 1 - p0.z : p0.z - pos[2]
	};
	progress.x *= delta.x;//'divide' by total length
	progress.y *= delta.y;
	progress.z *= delta.z;
	int count = 0;
	bool looping;
	do {
		//points.push(new Point(p.x, p.y));
		//check point
		hit = GetBlock(pos[0], pos[1], pos[2]);
		if (IsRaycastVisible[hit])
		{
			break;
		}
		if (progress.x < progress.y && progress.x < progress.z) {
			// next step is x
			dimension = 0;
		}
		else if (progress.y < progress.z) {
			// next step is y
			dimension = 1;
		}
		else {
			// next step is z
			dimension = 2;
		}
		pos[dimension] += sign[dimension];//step in the active dimenstion
		looping = progress.axis[dimension] <= 1;
		progress.axis[dimension] += delta.axis[dimension];//update progress
	} while (looping);
	selx = pos[0];
	sely = pos[1];
	selz = pos[2];
	adjx = pos[0];
	adjy = pos[1];
	adjz = pos[2];
	hit = GetBlock(selx, sely, selz);
	bool visible = IsRaycastVisible[hit];
	if (visible)
	{
		if (dimension == -1)
		{
			firstraycastintersection = p0;
		}
		else {
			//calculate the exact intersection point.
			firstraycastintersection.axis[dimension] = pos[dimension];
			if (sign[dimension] == -1)
			{
				++firstraycastintersection.axis[dimension];//this axis has to be increased because the ray hit on the plus side of the block.
			}
			fp distance = abs(firstraycastintersection.axis[dimension] - p0.axis[dimension]);
			int otheraxis0, otheraxis1;
			if (dimension == 0) {//last dimension: x
				adjx = selx - sign[0];
				otheraxis0 = 1;
				otheraxis1 = 2;
			}
			else if (dimension == 1) {//last dimension: y
				adjy = sely - sign[1];
				otheraxis0 = 0;
				otheraxis1 = 2;
			}
			else if (dimension == 2) {//last dimension: z
				adjz = selz - sign[2];
				otheraxis0 = 0;
				otheraxis1 = 1;
			}
			fp part = distance / absd0to1.axis[dimension];//the part of the ray that has been marched
			fp exact0 = p0.axis[otheraxis0] + part * d0to1.axis[otheraxis0];
			fp exact1 = p0.axis[otheraxis1] + part * d0to1.axis[otheraxis1];
			firstraycastintersection.axis[otheraxis0] = exact0;
			firstraycastintersection.axis[otheraxis1] = exact1;
		}
	}
	else
	{
		firstraycastintersection = p1;
	}
	return visible;
}
void ReloadTerrain() {
	fp px = player->position.x, py = player->position.y, pz = player->position.z;
	int intposx = floor(player->position.x / ChunkScaleHor);
	int intposy = floor(player->position.y / ChunkScaleHor);
	int NewPlusX = intposx * ChunkScaleHor + HalfLoadBlocks;
	int NewPlusY = intposy * ChunkScaleHor + HalfLoadBlocks;
	bool changed = PlusX != NewPlusX || PlusY != NewPlusY;
	PlusX = NewPlusX;
	PlusY = NewPlusY;
	PlusChunkX = PlusX / ChunkScaleHor;
	PlusChunkY = PlusY / ChunkScaleHor;
	if (changed)
	{
		RemoveOutBounds();
		Documentate();
	}
}
void RemoveOutBounds() {
	//int count = Chunk1d->size();
	auto i = Chunk1d->begin();
	while (i != Chunk1d->end())
	{
		Chunk* chunk = *i;
		int
			cx = chunk->xPos - PlusChunkX,
			cy = chunk->yPos - PlusChunkY;
		if (cx < 0 || cx >= SaveDiametre || cy < 0 || cy >= SaveDiametre) //out of bounds
		{
			//chunk->Delete();
			delete chunk;
			i = Chunk1d->erase(i);
		}
		else {
			i++;
		}
	}
}
void PopulateNearestNullChunk()
{
	int ix = 0, iy = 0;
	//fp max = LoadChunks / 2 - .5;
	//max *= max;
	fp NearestDistance = INFINITY;
	//int bytesize = LoadChunks2 * sizeof(Chunk);
	fp px = (player->position.x / ChunkScaleHor) - PlusChunkX;
	fp py = (player->position.y / ChunkScaleHor) - PlusChunkY;
	for (int j = populationMargin; j < SaveDiametre - populationMargin; j++)
	{
		fp yoff = j - py;
		yoff *= yoff;
		for (int i = populationMargin; i < SaveDiametre - populationMargin; i++)
		{
			int index = i + j * SaveDiametre;
			if ((ChunkTiles[index] == NULL || !ChunkTiles[index]->Populated) && IsVisible[index])
			{
				fp xoff = i - px;
				fp d = xoff * xoff + yoff;
				if (d < NearestDistance)
				{
					NearestDistance = d;
					ix = i;
					iy = j;
				}
			}
		}
	}
	if (NearestDistance < INFINITY)
	{
		//check if surrounding chunks can populate
		int MinX = ix - populationMargin;
		int MinY = iy - populationMargin;
		int MaxX = ix + populationMargin;
		int MaxY = iy + populationMargin;
		for (int ActiveY = MinY; ActiveY <= MaxY; ActiveY++)
		{
			for (int ActiveX = MinX; ActiveX <= MaxX; ActiveX++)
			{
				int index = ActiveY * SaveDiametre + ActiveX;
				Chunk* ActiveChunk = ChunkTiles[index];
				if (ActiveChunk == NULL) {//generate chunk blocks
					ActiveChunk = DBG_NEW Chunk(ActiveX + PlusChunkX, ActiveY + PlusChunkY);
					Chunk1d->push_back(ActiveChunk);
					ChunkTiles[index] = ActiveChunk;
				}
				if (ActiveX == ix || ActiveY == iy)
				{
					ActiveChunk->Changed = true;//update mesh
				}
			}
		}
		Chunk* ChunkToPopulate = ChunkTiles[ix + iy * SaveDiametre];
		ChunkToPopulate->Populate();
	}
}
//fill tile array with loaded chunks
void Documentate() {
	for (int i = 0; i < SaveDiametre2; i++) {
		ChunkTiles[i] = NULL;
	}

	//Chunk** ptr = ChunkTiles;//reset chunk array
	//Chunk** End = ptr + LoadChunks2;
	//while (ptr<End)
	//{
	//	*ptr++ = NULL;
	//}
	for (auto i = Chunk1d->begin(); i != Chunk1d->end(); i++) {
		Chunk* chunk = *i;
		ChunkTiles[chunk->xPos - PlusChunkX + (chunk->yPos - PlusChunkY) * SaveDiametre] = chunk;
	}

}
//creates the render window, returns wether success
bool CreateRenderWindow(int Width, int Height, const letter* Title)
{
	return true;
}
void GenerateControls() {
	//Image* imgCrossHair = &Image::FromFile("Data/Crosshair.png");
	//Image* imgInventorySlot = &Image::FromFile("data/inventoryslot.png");
	//Crosshair = new Control((graphics.width - imgCrossHair->Width) / 2, (graphics.height - imgCrossHair->Height) / 2, &Image::FromFile("Data/Crosshair.png"));
	//Inventory = new Control((graphics.width - imgInventorySlot->Width) / 2, (graphics.height - imgInventorySlot->Height) / 2, &Image::FromFile("data/inventoryslot.png"));
	Image img = *Image::FromFile(L"Data/Fonts/Font.bmp", true);
	int h = img.Height;
	int charh = h / 0x10;
	squaretex* tex = new squaretex(img.colors, h);
	Font* f = new Font(tex);
	commandline = new TextBox(0, (graphics.height - charh) / 2, graphics.width, charh, f);
	activeForm->childs.push_back(commandline);
	Image* cross = Image::FromFile(L"Data/crosshair.bmp", true);
	h = cross->Height;
	crosshair = new PictureBox((graphics.width - h) / 2, (graphics.height - h) / 2, h, h, cross);
	activeForm->childs.push_back(crosshair);
	editor = new biomeeditor(0, 0, graphics.width, graphics.height);
	editor->visible = false;
	activeForm->childs.push_back(editor);
}
void DeleteChunks()
{
	Chunk1d->clear();
	for (int i = 0; i < SaveDiametre2; i++) {
		delete ChunkTiles[i];
		ChunkTiles[i] = NULL;
	}
	//cout << "deleted all loaded chunks and actions" << endl;
}
#define cast reinterpret_cast<const char*>
void Save(const letter* path) {
	std::ofstream stream;//output file stream
	stream.open(path, std::ios::binary);
	if (stream.good()) {//succesfully opened
		stream.seekp(0, stream.beg);//go to start
		const int fs = sizeof(fp);
		//world options
		//int(4 bytes)
		stream.write(cast(&version), sizeof(int));
		stream.write(cast(&Seed), sizeof(int));
		stream.write(cast(&SeaLevel), sizeof(int));
		stream.write(cast(&Power), sizeof(int));
		//fp(varies)
		stream.write(cast(&MinE), sizeof(fp));//BE AWARE THAT THE SIZE OF FP CAN CHANGE
		stream.write(cast(&MaxE), sizeof(fp));
		stream.write(cast(&MinM), sizeof(fp));
		stream.write(cast(&MaxM), sizeof(fp));
		//graphical data
		stream.write(cast(&AirColor), sizeof(color));
		stream.write(cast(&WaterColor), sizeof(color));

		//player data
		stream.write(cast(&Pitch), sizeof(fp));
		stream.write(cast(&Yaw), sizeof(fp));
		stream.write(cast(&LastYaw), sizeof(fp));
		stream.write(cast(&FOV), sizeof(fp));

		stream.write(cast(&player->sneaking), sizeof(bool));
		stream.write(cast(&player->sinking), sizeof(bool));
		stream.write(cast(&player->gravity), sizeof(bool));

		stream.write(cast(&player->position), sizeof(vec3));
		stream.write(cast(&player->speed), sizeof(vec3));

		//events
		const int count = GetActionContainerCount();
		stream.write(cast(&count), sizeof(int));
		for (int i = 0; i < count; i++) {
			ActionContainer* ActiveContainer = getActionContainerPointer(i);
			stream.write(cast(&ActiveContainer->posX), sizeof(int));
			stream.write(cast(&ActiveContainer->posY), sizeof(int));
			std::list<Action*>* actions = ActiveContainer->actions;
			int size = (int)actions->size();
			stream.write(cast(&size), sizeof(int));
			for (auto j = actions->begin(); j != actions->end(); j++) {
				Action* action = *j;
				stream.write(cast(&action->b), sizeof(block));
				stream.write(cast(&action->index), sizeof(BlockIndex));
			}
		}

	}
	else {
		//cout << "Faillure. Filename was bad" << endl;
	}
	stream.close();
}
void Open(const letter* path) {
	std::ifstream stream;//input file stream
	stream.open(path, std::ios::binary);
	if (stream.good()) {
		stream.seekg(0, stream.beg);
		const int fs = sizeof(fp);
		//world options
		//int(4 bytes)
		int fileversion;//the version of the file
		stream.read((char*)& fileversion, sizeof(int));
		if (fileversion == version ||
			MessageBox(hwnd, L"This file has another version than your version. do you still want to open it?", L"WorldCraft", MB_YESNO) == IDYES) {
			try
			{
				stream.read((char*)& Seed, sizeof(int));
				stream.read((char*)& SeaLevel, sizeof(int));
				stream.read((char*)& Power, sizeof(int));
				//fp(varies)
				stream.read((char*)& MinE, sizeof(fp));//BE AWARE THAT THE SIZE OF FP CAN CHANGE
				stream.read((char*)& MaxE, sizeof(fp));
				stream.read((char*)& MinM, sizeof(fp));
				stream.read((char*)& MaxM, sizeof(fp));

				//graphical data
				stream.read((char*)& AirColor, sizeof(color));
				stream.read((char*)& WaterColor, sizeof(color));

				//player data
				stream.read((char*)& Pitch, sizeof(fp));
				stream.read((char*)& Yaw, sizeof(fp));
				stream.read((char*)& LastYaw, sizeof(fp));
				stream.read((char*)& FOV, sizeof(fp));

				stream.read((char*)& player->sneaking, sizeof(bool));
				stream.read((char*)& player->sinking, sizeof(bool));
				stream.read((char*)& player->gravity, sizeof(bool));

				stream.read((char*)& player->position, sizeof(vec3));
				stream.read((char*)& player->speed, sizeof(vec3));


				//initialize terrain generator by seed
				InitializeTerrain();
				//events
				DeleteChunks();
				ClearActionContainers();
				int count;
				stream.read((char*)& count, sizeof(int));
				for (int i = 0; i < count; i++) {
					int posX, posY;
					stream.read((char*)& posX, sizeof(int));
					stream.read((char*)& posY, sizeof(int));
					ActionContainer* ActiveContainer = new ActionContainer(posX, posY);
					std::list<Action*>* actions = ActiveContainer->actions;
					int count;
					stream.read((char*)& count, sizeof(int));
					for (int j = 0; j < count; j++) {
						BlockIndex index;
						block b;
						stream.read((char*)& b, sizeof(b));
						stream.read((char*)& index, sizeof(BlockIndex));
						actions->push_back(new Action(index, b));
					}
				}
			}
			catch (const std::exception&)
			{
				MessageBox(hwnd, L"an error has occurred while opening the file.", L"WorldCraft", MB_OK);
			}
		}
	}
	else {
		std::cout << "Faillure. Filename was bad" << std::endl;
	}

	stream.close();
}
bool close;

//update tick
void Update()
{
	for (mob* m : mobs)
	{
		m->Update();
		m->Physics();
	}
}

bool Run() {
	Initialize();
	close = false;
	LastTime = getseconds(getmiliseconds());


	/*const int w = 200, h = 200;
	//create noise map
	Image* image = new Image(200, 200);
	//color* NoiseMap = new color[w * h];
	color* ptr = image->colors;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			Biome ActiveBiome = GetBiome(x, y);
			*ptr++ = BiomeColor[ActiveBiome];
		}
	}
	PictureBox* control = new PictureBox(0, graphics.height - h, w, h, image);
	*/
	LastUpdateTime = getseconds(getmiliseconds());
	while (!close && DoEvents())
	{
		ReloadTerrain();
		//generate nearest chunk
		PopulateNearestNullChunk();
		//sleep
		seconds NewTime = getseconds(getmiliseconds());
		if (DrawControls) {
			if (Pressed(COMMAND_KEY)) {
				if (releasedSlash) {
					if (Focus == commandline->index)
					{
						Focus = -1;
						//clear text
						commandline->text = L"";
					}
					else
					{
						Focus = commandline->index;
						commandline->text = L"/";
					}
					releasedSlash = false;
				}
			}
			else
			{
				releasedSlash = true;
			}
			if (pressed('l'))
			{
				editor->visible = !editor->visible;
			}
		}
		if (Pressed(VK_RETURN) && Focus == commandline->index)
		{
			Focus = -1;
			if (commandline->text.length() > 0)
			{
				//execute
				Execute(commandline->text);
				commandline->text = L"";
			}
		}
		for (int i = 0; LastUpdateTime < NewTime; i++)
		{
			if (i > 10) {
				LastUpdateTime = NewTime;
				break;
			}
			ProcessCursor();
			// input
			if (Focus < 0)
			{
				//
				Update();
				//max 10 frames at once
			}
			LastUpdateTime += activeticktime;
		}
		msElapsed = (NewTime - LastTime) * 1000;
		int milisecSleep = (int)(MSFrame - msElapsed);
		std::this_thread::sleep_for(std::chrono::milliseconds(milisecSleep));
		LastTime = getseconds(getmiliseconds());
		SetDirection();
		//if (focused) 
		//{
		Draw();
		//}
		RightButtonDown = LeftButtonDown = false;
		// Draw colors to window
		BitBlt(wndDC, 0, 0, graphics.width, graphics.height, hdcMem, 0, 0, SRCCOPY);
	}
	return false;
}
void Delete() {
	// optional: de-allocate all resources once they've outlived their purpose:
// ------------------------------------------------------------------------
	for (int i = 0; i < SaveDiametre2; i++) {
		delete ChunkTiles[i];
	}
	Chunk1d->clear();
	delete BlockTextures;
}

// Do stuff with colors
void Draw() {
	// render
	// ------
	//first 3d
	Render3D();
	//then 2d overlay
	if (DrawControls)
	{
		activeForm->Draw(graphics);
	}
	std::wostringstream strs;
	fp fps = 1000.0 / msElapsed;
	strs << fps;
	std::wstring str = strs.str();
	std::wstring wstr = std::wstring(str.begin(), str.end());
	//then caption
	SetWindowText(hwnd, wstr.c_str());
}
void Render3D() {
	color FogColor;
	if (GetBlock(CameraPosition) == Water) {
		FogDistance = 50.0;
		FogColor = WaterColor;
	}
	else if (GetBlock(CameraPosition) == Lava) {
		FogDistance = FistRange;
		FogColor = LavaColor;
	}
	else
	{
		FogDistance = MaxDistance;
		FogColor = AirColor;
	}
	if (FogDistance > MaxDistance)
	{
		FogDistance = MaxDistance;
	}
	//if (!fog)
	graphics.ClearColor(FogColor);//if there is no water borders and fog, then clearing is unnessesary.
	graphics.ClearDepthBuffer(FogDistance);

	// projection matrix
	projection = mat4x4::perspectiveFov(DegToRad * FOV, graphics.width, graphics.height, minZplane, FogDistance);
	// camera/view transformation
	view = mat4x4::lookat(CameraPosition, CameraPosition + CameraDirection, vec3(0, 0, 1));
	matrix = mat4x4::cross(projection, view);
	CropView();//optimisation
	rendersettings::s3d::mindistance = minZplane;
	rendersettings::s3d::maxdistance = FogDistance;
	rendersettings::s3d::cullbackfaces = true;
	//draw mobs
	for (mob* m : mobs) {
		m->Draw(&graphics, matrix, CameraPosition, CameraDirection);
	}
	/*bodypart b = bodypart(NULL, firstraycastintersection, 0.2,vec3(0.1));
	color cintersect = color(0, 0, 255);
	b.Draw(&graphics, matrix, &cintersect, CameraPosition, CameraDirection);
	color ceye = color(255, 0, 0);
	b.translate = EyePosition;
	b.changed = true;
	b.Draw(&graphics, matrix, &ceye, CameraPosition, CameraDirection);*/
	//draw chunks culled
	for (int i = 0; i < SaveDiametre2; i++)
	{
		if (ChunkTiles[i] != NULL && IsVisible[i] && ChunkTiles[i]->Populated)
		{
			Chunk* ActiveChunk = ChunkTiles[i];
			if (ActiveChunk->Changed)ChunkTiles[i]->GenerateMesh();
			if (ActiveChunk->CulledSolidSize > 0)
			{
				graphics.DrawTrianglesTex(ActiveChunk->CulledSolidVertices, PlaneIndices, ActiveChunk->CulledSolidSize, ActiveChunk->CulledSolidSize * 2, CameraPosition, matrix, *BlockTextures, CameraDirection);
			}
		}
	}
	//draw chunks unculled
	rendersettings::s3d::cullbackfaces = false;
	for (int i = 0; i < SaveDiametre2; i++)
	{
		if (ChunkTiles[i] != NULL && IsVisible[i] && ChunkTiles[i]->Populated)
		{
			Chunk* ActiveChunk = ChunkTiles[i];
			if (ActiveChunk->UnCulledSolidSize > 0)
			{
				graphics.DrawTrianglesTex(ActiveChunk->UnCulledSolidVertices, PlaneIndices, ActiveChunk->UnCulledSolidSize, ActiveChunk->UnCulledSolidSize * 2, CameraPosition, matrix, *BlockTextures, CameraDirection);
			}
		}
	}

	bufferobject<color> bcolors = bufferobject<color>(&WaterColor, 1, 0, 12);
	for (int i = 0; i < SaveDiametre2; i++)
	{
		if (ChunkTiles[i] != NULL && IsVisible[i] && ChunkTiles[i]->Populated)
		{
			Chunk* ActiveChunk = ChunkTiles[i];
			if (ActiveChunk->FluidSize > 0)
			{
				//elementcount?
				bufferobject<fp> b = bufferobject<fp>(ActiveChunk->FluidVertices, ActiveChunk->FluidSize * 2 * (3 + 3), (3 + 3), ActiveChunk->FluidSize * 2);
				bufferobject<uint> bindices = bufferobject<uint>(PlaneIndices, 3 * ActiveChunk->FluidSize, 3, ActiveChunk->FluidSize);
				graphics.DrawTrianglesPlain(&b, &bindices, &bcolors, CameraPosition, matrix, CameraDirection);
			}
		}
	}
	if (fog)graphics.Fog(FogColor, 1.0 / FogDistance);
}
block GetBlock(vec3 pos) {
	return GetBlock(floor(pos.x), floor(pos.y), floor(pos.z));
}
block GetBlock(int x, int y, int z) {
	int ChunkIndex;
	if (Chunk * chunk = MapOnChunk(x, y, z, ChunkIndex))
	{
		BlockIndex index = x + y * ChunkScaleHor + z * ChunkScale2;
		block b = *(chunk->data + index);
		return b;
	}
	else
	{
		return Air;
	}
}
vec3 GetLightLevel(int x, int y, int z)
{
	const int LoadScaleHor = ChunkScaleHor * SaveDiametre;
	x -= PlusX;
	y -= PlusY;
	if (x < 0 || y < 0 || z < 0 || x >= LoadScaleHor || y >= LoadScaleHor || z >= ChunkScaleVert)return vec3(1);
	int ChunkX = x / ChunkScaleHor, ChunkY = y / ChunkScaleHor;
	Chunk* chunk = ChunkTiles[ChunkX + ChunkY * SaveDiametre];
	if (chunk == NULL)return vec3(1);
	x = x % ChunkScaleHor;
	y = y % ChunkScaleHor;
	//z = z + MarginZBottom;
	BlockIndex index = x + (y * StrideY) + (z * StrideZ);
	fp brightness = *(chunk->LightMap + index) * BrightnessValueToFp;
	return vec3(brightness);
}
std::list<vec3> GravityCheckPoints = std::list<vec3>({
	zup::left,zup::right,
	zup::forward,zup::back,
	zup::down
	}
);

//Flood - fill(node, target - color, replacement - color) :
//https://en.wikipedia.org/wiki/Flood_fill
void FloodFill(int x, int y, int z, block fill, std::list<vec3> PointsCheckFor)
{
	block floodFrom = GetBlock(x, y, z);
	//	1. If target is equal to replacement, return.
	if (floodFrom == fill)return;
	//3. Set the node to replacement.
	SetBlock(x, y, z, fill, true);
	//4. Set Q to the empty queue.
	std::list<vec3> Q = std::list<vec3>();
	//5. Add node to the end of Q.
	Q.push_back(vec3(x, y, z));
	//6. While Q is not empty:
	while (Q.size() > 0)
	{
		//7. Set n equal to the first element of Q.
		vec3 n = *Q.begin();
		//8.     Remove first element from Q.
		Q.pop_front();
		for (vec3 off : PointsCheckFor)
		{
			//9.     If the type of a node next to n is target,
			vec3 next = n + off;
			int ix = (int)next.x, iy = (int)next.y, iz = (int)next.z, chunkindex;
			if (Chunk * chunk = MapOnChunk(ix, iy, iz, chunkindex))
			{
				//DO NOT USE ix, iy and iz. they are transformed to chunk coordinates.
				if (GetBlock(next) == floodFrom)
				{
					//set the type of that node to replacement - type and add that node to the end of Q.
					SetBlock(next, fill, true);
					Q.push_back(next);
				}
			}
			//10. Continue looping until Q is exhausted.
		}
	}
	//11. Return.
}
void SetBlockRange(int x0, int y0, int z0, int x1, int y1, int z1, block value, const bool addaction)
{
	for (int k = z0; k < z1; k++)
	{
		for (int j = y0; j < y1; j++)
		{
			for (int i = x0; i < x1; i++)
			{
				SetBlock(i, j, k, value, addaction);
			}
		}
	}
}
void SetBlockImgSize(vec3 v00, vec3 v10, vec3 v11, Image* image, std::vector<color> colors, std::vector<block> blocks, const bool addaction) 
{
	vec3 xstep = v10 - v00;
	vec3 ystep = v11 - v10;
	xstep /= image->Width;
	ystep /= image->Height;
	SetBlockImgStep(v00, xstep, ystep, image, colors, blocks, addaction);
}
void SetBlockImgStep(vec3 start, vec3 xstep, vec3 ystep, Image* image, std::vector<color> colors,std::vector<block> blocks, const bool addaction)
{
	vec3 activey = start;
	for (int iy = 0; iy < image->Height; iy++) 
	{
		vec3 activex = activey;
		for (int ix = 0; ix < image->Width; ix++)
		{
			color c = image->GetColor(ix, iy);
			for (int index = 0; index < colors.size(); index++) 
			{
				if (colors[index] == c) 
				{
					SetBlock(activex, blocks[index], addaction);
				}
			}
			activex += xstep;
		}
		activey += ystep;
	}

}
void SetBlockRangeFromTo(int x0, int y0, int z0, int x1, int y1, int z1, block value, const bool addaction)
{
	int minx, miny, minz, maxx, maxy, maxz;
	if (x0 < x1)
	{
		minx = x0; maxx = x1;
	}
	else
	{
		minx = x1; maxx = x0;
	}
	if (y0 < y1)
	{
		miny = y0; maxy = y1;
	}
	else
	{
		miny = y1; maxy = y0;
	}
	if (z0 < z1)
	{
		minz = z0; maxz = z1;
	}
	else
	{
		minz = z1; maxz = z0;
	}
	for (int k = minz; k <= maxz; k++)
	{
		for (int j = miny; j <= maxy; j++)
		{
			for (int i = minx; i <= maxx; i++)
			{
				SetBlock(i, j, k, value, addaction);
			}
		}
	}
}
Chunk* MapOnChunk(int& x, int& y, const int& z, int& ChunkIndex)
{
	const int LoadScaleHor = ChunkScaleHor * SaveDiametre;
	x -= PlusX;
	y -= PlusY;
	if (x < 0 || y < 0 || z < 0 || x >= LoadScaleHor || y >= LoadScaleHor || z >= ChunkScaleVert)return NULL;
	int ChunkX = x / ChunkScaleHor, ChunkY = y / ChunkScaleHor;
	ChunkIndex = ChunkX + ChunkY * SaveDiametre;
	Chunk* chunk = ChunkTiles[ChunkIndex];
	if (chunk != NULL)
	{
		//x and y in chunk
		x %= ChunkScaleHor;
		y %= ChunkScaleHor;
	}
	return chunk;
}

void SetBlock(vec3 pos, block b, const bool addaction)
{
	SetBlock(floor(pos.x), floor(pos.y), floor(pos.z), b, addaction);
}
void SetBlock(int x, int y, int z, block b, const bool addaction) {
	int ChunkIndex;
	if (Chunk * chunk = MapOnChunk(x, y, z, ChunkIndex)) {
		if (x == 0) {
			if (Chunk * mx = ChunkTiles[ChunkIndex - 1]) 
			{
				mx->Changed = true;
			}
		}
		else if (x == ChunkScaleHorMin)
		{
			if (Chunk * px = ChunkTiles[ChunkIndex + 1]) {
				px->Changed = true;
			}
		}
		if (y == 0) {
			if (Chunk * my = ChunkTiles[ChunkIndex - SaveDiametre]) {
				my->Changed = true;
			}
		}
		else if (y == ChunkScaleHorMin) {
			if (Chunk * py = ChunkTiles[ChunkIndex + SaveDiametre]) {
				py->Changed = true;
			}
		}
		BlockIndex index = x + y * ChunkScaleHor + z * ChunkScale2;
		if (addaction)
		{
			AddAction(chunk->xPos, chunk->yPos, index, b);
		}
		*(chunk->data + index) = b;
		chunk->Changed = true;
	}
}
//bool flying = true;//indicates wether player has nothing below the feet
//bool FluidFeet = false;//indicates wether player has a fluid below the feet
//bool lastflying = true;
//bool sneak = false;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void SetActivePath(letter* path)
{
	delete[] ScreenShotPath;
	ScreenShotPath = path;
}
void ShowMouse()
{
	while (ShowCursor(true) <= 0);
}
void HideMouse()
{
	while (ShowCursor(false) >= 0);
}
void human::Update()
{
	fp RadiansPitch = Pitch * DegToRad;
	fp RadiansYaw = -Yaw * DegToRad;
	fp CosYaw = cos(RadiansYaw);
	fp SinYaw = sin(RadiansYaw);
	fp DegreesPerFrame = MSFrame * .2;
	//if player is on ground:
	if (!player->falling)
	{
		//rotate speed vector with camera, and substract friction by rotation
		fp dYaw = LastYaw - Yaw;
		fp friction = pow(0.5f, abs(dYaw / 180));//0.5 speed multiplier when turned 180 degrees
		player->speed = player->speed.rotatez(dYaw / 180 * PI) * friction;
	}
	if (Pressed(GRAVITY_KEY) && !hasgravity[mode]) {
		if (releasedG) {
			player->gravity = !player->gravity;
			releasedG = false;
		}
	}
	else {
		releasedG = true;
	}

	bool Sprintkey = Pressed(SPRINT_KEY);
	if (Sprintkey) {
		FOV = (FOV * 5 + 110) / 6.0;
	}
	else
	{
		FOV = (FOV * 5 + 90) / 6.0;
	}
	fp MovementSpeed;
	fp step;//the power of this step
	if (player->gravity) {
		fp Jumpheight = 1.252203;// vanilla mc: 1.25
		const fp Jumpspeed = //0.23;
			0.42;//to jump 1.252203 blocks high
		const fp SwimUpspeed = GravityForcePerTick + (0.5f * NormalTickTime * (1 - FluidFriction));//0.2;
		const fp standheight = 1.8;
		PlayerHeight = standheight;

		if (Sprintkey) {
			MovementSpeed = SprintingSpeed;
		}
		else {
			MovementSpeed = WalkingSpeed;
		}
		if (player->falling)
		{
			if (player->sinking) {
				bool jump = Pressed(JUMP_KEY);
				if (jump)
				{
					player->speed.z += SwimUpspeed;
				}
			}
			step = MovementSpeed * NormalTickTime * (player->falling ? AirFrictionMultiplier : GroundFrictionMultiplier);
		}
		else
		{
			bool jump = Pressed(JUMP_KEY) && !player->falling;
			player->sneaking = Pressed(SNEAK_KEY) && !jump;
			if (jump)
			{
				player->speed.z += Jumpspeed;
				step = 0.18;//horizontal jump power, .17 is too few
			}
			else {
				if (player->sneaking) {
					//sneak = true;
					const fp sneakheight = 1.65;
					PlayerHeight = sneakheight;
					MovementSpeed = SneakingSpeed;
					//Speed.z -= vz;
				}
				step = MovementSpeed * NormalTickTime * (player->falling ? AirFrictionMultiplier : GroundFrictionMultiplier);
			}
		}
	}
	else {
		if (Sprintkey) {
			MovementSpeed = SprintFlyingSpeed;
		}
		else
		{
			MovementSpeed = FlyingSpeed;
		}
		step = MovementSpeed * NormalTickTime * AirFrictionMultiplier;
		if (Pressed(JUMP_KEY)) {
			player->speed.z += step;
		}
		if (Pressed(SNEAK_KEY)) {
			player->speed.z -= step;
		}

	}
	EyeHeight = PlayerHeight - .18;
	fp vx, vy;
	fp TwoKeyMult = 0.70710678118654752440084436210485;
	vx = CosYaw * step;
	vy = SinYaw * step;
	if (Pressed(JUMP_TO_BUILDPOS_KEY))
	{
		int ContainerCount = GetActionContainerCount();
		if (ContainerCount > 0) {
			int ContainerIndex = rand() % ContainerCount;
			ActionContainer* container = getActionContainerPointer(ContainerIndex);
			std::list<Action*>* actions = container->actions;
			int ActionIndex = rand() % actions->size();
			auto item = actions->begin();
			std::advance(item, ActionIndex);
			Action* action = *item;
			int aIndex = action->index;
			int aX = aIndex % ChunkScaleHor;
			int aY = (aIndex / StrideY) % ChunkScaleHor;
			int aZ = aIndex / StrideZ;
			player->position.x = (fp)container->posX * ChunkScaleHor + aX;
			player->position.y = (fp)container->posY * ChunkScaleHor + aY;
			player->position.z = aZ;
		}
	}
	if (Pressed(GUI_KEY))
	{
		DrawControls = !DrawControls;
	}
	//https://stackoverflow.com/questions/5844858/how-to-take-screenshot-in-opengl
	if (Pressed(SCREENSHOT_KEY))
	{
		Image img = Image(graphics.colors, graphics.width, graphics.height, sizeof(color));
		img.Save(L"screenshot.bmp");
		//ALWAYS the DOUBLE SLASH
		// Convert to FreeImage format & save to file
	}
	//vec3 speed
	/*
	//jump to random build places
	if (Pressed(JUMP_TO_BUILDPOS_KEY))
	{
		wstring sx = to_string(Position.x);
		wstring sy = to_string(Position.y);
		wstring sz = to_string(Position.z);
		MessageBoxA(NULL, "x: " + sx.c_str() + " y: " + sy.c_str() + " z: " + sz.c_str(), "WorldCraft", MB_OK);
	}*/
	if (Pressed(EXIT_KEY))
	{
		ShowMouse();//make sure the mouse is showm
		switch (MessageBox(hwnd, L"Do you want to save changes?", L"WorldCraft", MB_YESNOCANCEL))
		{
		case IDYES:
		{
			const letter* file = getopenfilename(filedialogmode::save, L"*.world", L"select a path");
			if (file)
			{
				Save(file);
			}
			delete[] file;
			break;
		}
		// save
		case IDNO:
			// close
			close = true;
			break;
		case IDCANCEL:
			// cancel
			break;
		}
		if (LockMouse)HideMouse();
	}
	bool left = Pressed(LEFT_KEY);
	bool right = Pressed(RIGHT_KEY);
	bool forward = Pressed(FORWARD_KEY);
	bool backward = Pressed(BACKWARD_KEY);
	if (left ^ right && forward ^ backward) //else the player can cheat walking diagonal
	{
		vx *= TwoKeyMult;
		vy *= TwoKeyMult;
	}
	if (forward) {
		player->speed.y += vy;
		player->speed.x += vx;
	}
	if (backward) {
		player->speed.y -= vy;
		player->speed.x -= vx;
	}
	if (left) {
		player->speed.y += vx;
		player->speed.x -= vy;
	}
	if (right) {
		player->speed.y -= vx;
		player->speed.x += vy;
	}
	if (Pressed(LOOK_LEFT_KEY)) {
		Yaw -= DegreesPerFrame;
	}
	if (Pressed(LOOK_RIGHT_KEY)) {
		Yaw += DegreesPerFrame;
	}
	if (Pressed(LOOK_UP_KEY)) {
		Pitch += DegreesPerFrame;
	}
	if (Pressed(LOOK_DOWN_KEY)) {
		Pitch -= DegreesPerFrame;
	}
	if (Pressed(BRAKE_KEY)) {
		player->speed = vec3();
	}
	LastYaw = Yaw;
	//rotate feet
	if (gravity && !falling) {
		fp l = player->speed.length();
		if (backward) {
			traveleddistance -= l;
		}
		if (forward) {
			traveleddistance += l;
		}
		const fp swingspermeter = 5;
		const fp swingradians = 0.5;
		leg0->rotations[0].angle = leg1->rotations[0].angle = sin(traveleddistance * swingspermeter) * swingradians;
		leg0->changed = leg1->changed = true;
	}
}

block GetBlockByID(std::wstring ID)
{
	long id;
	if (strtol(ID, id,10))
	{
		return (block)id;
	}
	else
	{
		makelowercase(ID);
		for (int i = 0; i < BlockCount; i++)
		{
			if (ID == blockname[i])
			{
				return (block)i;
			}
		}
	}
}
gamemode GetGamemodeByID(std::wstring ID)
{
	long id;
	if (strtol(ID, id,10))
	{
		return (gamemode)id;
	}
	else
	{
		makelowercase(ID);
		for (int i = 0; i < gamemodecount; i++)
		{
			if (ID == gamemodename[i])
			{
				return (gamemode)i;
			}
		}
	}
}
std::vector<std::wstring> Split(std::wstring line)
{
	std::vector<std::wstring> result = std::vector<std::wstring>();
	int beginnextword = 0;
	//split the line in segments
	int depth = 0;//the depth in a string
	int i;
	for (i = 0; i < line.size(); i++) 
	{
		if (line[i] == L' ' && depth == 0) //cut
		{
			result.push_back(line.substr(beginnextword, i - beginnextword));
			beginnextword = i + 1;
		}
		else if (line[i] == L'"') //begin or end of string
		{
			if (depth) //end of string
			{
				depth--;
			}
			else //begin of string
			{
				depth++;
			}
		}
	}
	result.push_back(line.substr(beginnextword, i - beginnextword));
	return result;
}
void Execute(std::wstring line)
{
	line = line.substr(1, line.length() - 1);
	if (line == L"exit")
	{
		close = true;
	}
	else if (line == L"delete chunks")
	{
		DeleteChunks();
	}
	else if (line == L"delete actions")
	{
		ClearActionContainers();
	}
	else {
		std::vector<std::wstring> words = Split(line);
		int wordcount = words.size();

		if (wordcount > 0) {
			if (words[0] == L"gamemode" || words[0] == L"gm")
			{
				gamemode newmode = GetGamemodeByID(words[1]);
				if (newmode >= 0 && newmode != mode)
				{
					if (hasgravity[newmode])
					{
						player->gravity = true;
					}
				}
			}
			if (words[0] == L"open")
			{
				std::wstring filename = line.substr(5);//'open ' = 5 chars
				Open(filename.c_str());
			}
			else if (words[0] == L"screenshotpath")
			{
				SetActivePath((letter*)words[1].c_str());
				//delete[] ScreenShotPath;
				//ScreenShotPath = (letter*)words[1].c_str();
			}
			if (words[0] == L"tickrate")
			{
				fp rate = wcstof(words[1].c_str(), 0);
				if (rate > 0.05)
				{
					activeticktime = 1 / rate;
				}
			}
			else if (words[0] == L"fistrange")
			{
				FistRange = wcstof(words[1].c_str(), 0);
			}
			else if (words[0] == L"power")
			{
				Power = stoi(words[1]);
			}
			else if (words[0] == L"mine")
			{
				MinE = wcstof(words[1].c_str(), 0);
				InitializeTerrain();
			}
			else if (words[0] == L"maxe")
			{
				MaxE = wcstof(words[1].c_str(), 0);
				InitializeTerrain();
			}
			else if (words[0] == L"minm")
			{
				MinM = wcstof(words[1].c_str(), 0);
				InitializeTerrain();
			}
			else if (words[0] == L"maxm")
			{
				MaxM = wcstof(words[1].c_str(), 0);
				InitializeTerrain();
			}
			else if (words[0] == L"seed")
			{
				Seed = stoi(words[1]);
				InitializeTerrain();
			}
			else if (words[0] == L"save")//'save ' = 5 chars
			{
				std::wstring filename = line.substr(5);
				Save(filename.c_str());
			}
			else if (words[0] == L"mousesensitivity")
			{
				MouseSensitivity = wcstof(words[1].c_str(), 0);
			}
			else if (words[0] == L"sealevel")
			{
				SeaLevel = stoi(words[1]);
			}
			else if (words[0] == L"set")
			{
				if (wordcount > 4 && words[1] == L"img")
				{
					std::wstring path = words[2].substr(1, words[2].length() - 2);
					Image* img = Image::FromFile(path, false);
					std::vector<color> colors = std::vector<color>();
					std::vector<block> blocks = std::vector<block>();
					vec3 v0 = vec3(lastblockx[2], lastblocky[2], lastblockz[2]), v1 = vec3(lastblockx[1], lastblocky[1], lastblockz[1]), v2 = vec3(lastblockx[0], lastblocky[0], lastblockz[0]);
					int index = 3;
					bool step = false;
					if (words[index] == L"step") 
					{
						step = true;
						index++;
					}
					while (index + 1 < wordcount) 
					{
						colors.push_back(color::fromwstring(words[index]));
						blocks.push_back(GetBlockByID(words[index + 1]));
						index += 2;
					}
					if (step) 
					{
						SetBlockImgStep(v0, v1 - v0, v2 - v1, img, colors, blocks, true);
					}
					else 
					{
						SetBlockImgSize(v0, v1 - v0, v2 - v1, img, colors, blocks, true);
					}
					delete[] img->colors;
					delete img;
				}
				else {
					SetBlockRangeFromTo(lastblockx[0], lastblocky[0], lastblockz[0], lastblockx[1], lastblocky[1], lastblockz[1], (wordcount > 1 ? GetBlockByID(words[1]) : (block)ActiveItem), true);
				}
			}
			else if (words[0] == L"floodfill")
			{
				FloodFill(lastblockx[0], lastblocky[0], lastblockz[0], (wordcount > 1 ? GetBlockByID(words[1]) : (block)ActiveItem), GravityCheckPoints);
			}
			else if (words[0] == L"item")
			{
				ActiveItem = GetBlockByID(words[1]);
			}
			else if (words[0] == L"itemtype")
			{
				ActiveItemType = (itemtype)stoi(words[1]);
			}
			else if (words[0] == L"aircolor")
			{
				AirColor = color(
					(fp)wcstof(words[1].c_str(), 0),
					wcstof(words[2].c_str(), 0),
					wcstof(words[3].c_str(), 0));
			}
			else if (words[0] == L"watercolor")
			{
				WaterColor = color(
					(fp)wcstof(words[1].c_str(), 0),
					wcstof(words[2].c_str(), 0),
					wcstof(words[3].c_str(), 0));
			}
			else if (words[0] == L"tp")
			{
				vec3 pos;
				if (words[wordcount - 1] == L"@p")
				{
					pos = player->position;
				}
				else {
					pos = vec3(
						wcstof(words[wordcount - 3].c_str(), 0),
						wcstof(words[wordcount - 2].c_str(), 0),
						wcstof(words[wordcount - 1].c_str(), 0)
					);
				}
				if (words[1] == L"@a")
				{
					for (mob* m : mobs)
					{
						m->position = pos;
					}
				}
				else {
					//teleportate
					player->position = pos;
				}
			}
			else if (words[0] == L"spawn")
			{
				vec3 pos = vec3(SelectedBlockX + 0.5, SelectedBlockY + 0.5, SelectedBlockZ + 2 + 0.5);
				int count = (wordcount == 3) ? stoi(words[2]) : 1;//spawn count
				for (int j = 0; j < count; j++)
				{
					if (words[1] == L"pig")
					{
						mobs.push_back(new pig(pos));
					}
					if (words[1] == L"cod")
					{
						mobs.push_back(new cod(pos));
					}
					if (words[1] == L"human")
					{
						mobs.push_back(new pig(pos));
					}
					if (words[1] == L"parrot")
					{
						mobs.push_back(new parrot(pos));
					}
				}
			}
			else if (wordcount > 1)
			{
				if (words[1] == L"fog")
				{
					fog = words[0] == L"enable";
				}
			}
			else
			{
				MessageBox(hwnd, L"action is not recognised", L"Worldcraft", MB_OK);
			}
		}
	}
}