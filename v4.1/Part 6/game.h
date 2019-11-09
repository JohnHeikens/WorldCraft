#pragma once
#pragma comment(lib, "winmm.lib")

//default includes
#include <windows.h>
#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <crtdbg.h>
#include "wtypes.h"
#include <stdio.h>
#include <list>
#include <thread>
#include <sstream>
#include <mmsystem.h>
//project includes
#include "include.h"
#include "Chunk.h"
#include "mobs.h"
#include "biomeeditor.h"
//#include <FreeImage/FreeImage.h>
//using namespace std;
//defines
#define LOCKMOUSE_KEY 'M'
#define JUMP_TO_BUILDPOS_KEY 'K'
#define GRAVITY_KEY 'G'
#define EXIT_KEY VK_ESCAPE
#define SNEAK_KEY VK_LSHIFT
#define SPRINT_KEY VK_LCONTROL
#define JUMP_KEY VK_SPACE
#define LEFT_KEY 'A'//VK_LEFT
#define RIGHT_KEY 'D'//VK_RIGHT
#define FORWARD_KEY 'W'//VK_UP
#define BACKWARD_KEY 'S'//VK_DOWN
#define BRAKE_KEY 'R'
#define LOOK_LEFT_KEY VK_LEFT//VK_A
#define LOOK_RIGHT_KEY VK_RIGHT//VK_D
#define LOOK_UP_KEY VK_UP//VK_W
#define LOOK_DOWN_KEY VK_DOWN//VK_S
#define PLACE_KEY 'V'
#define DIG_KEY 'B'
#define DROP_KEY 'Q'
#define INVENTORY_KEY 'E'
#define DigButton VK_LBUTTON
#define PlaceButton VK_RBUTTON
#define GUI_KEY 'N'
#define COMMAND_KEY 191//forward slash key
#define SCREENSHOT_KEY 'H'
//https://minecraft.gamepedia.com/Player
#define FPS 30//60
#define MSFrame 1000/FPS
#define SneakingSpeed 1.31f //1.31 blocks per second
#define WalkingSpeed 4.317f //4.317 blocks per second
#define SprintingSpeed 5.612f //5.612 blocks per second
#define FlyingSpeed 10.92f //10.92 blocks per second
#define SprintFlyingSpeed 21.78f //21.78 blocks per second
#define MouseDown 1
#define JumpHeight 1.25219f//maximum height player can jump

//constants
constexpr fp PI = 3.14159265359f;
enum itemtype { tblock, ttree };

constexpr int version = 0;

//void GetDesktopResolution(int& horizontal, int& vertical);
//extern letter* ScreenShotPath;
//externs
extern fp MouseSensitivity;
extern int DesktopWidth, DesktopHeight;

//functions
//graphical functions in graphics.h
//blocks
bool walk_grid(vec3 p0, vec3 p1, int& selx, int& sely, int& selz, int& adjx, int& adjy, int& adjz, block& block);
void Dig();
void Place();
//chunk management
void ReloadTerrain();
void Documentate();
void RemoveOutBounds();
void DeleteChunks();
void PopulateNearestNullChunk();
//main functions
bool Run();
void Delete();
void Initialize();
void SetActivePath(letter* path);
void Save(const letter* path);
void Open(const letter* path);
//interaction
void UpdateCursor();
void ProcessCursor();
inline bool Pressed(int key);
void Execute(std::wstring line);
void HideMouse();
void ShowMouse();
void Render3D();
void Draw();
void GenerateControls();