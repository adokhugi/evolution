#ifndef _GAME_H_
#define _GAME_H_

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

#include "NeHeGL.h"
#include "NeHe_Window.h"
#include "Bitmap.h"
#include "bass.h"

class Position
{
public:
	int x, y;
};

enum EnemyType
{
	Kraken,
	Seastar,
	Jellyfish,
	Cancer,
	EnemyBullet,
	CancerBullet,
	Lion,
	Worm
};

class Enemy
{
public:
	Position position;
	EnemyType type;
	int width, height;
	bool moving;
	Position whereToGo;
	int variable [10];
};

class Game
{
public:
	bool Initialize (GL_Window *window, Keys *keys, DWORD tickCount);
	void Deinitialize ();
	void Update (DWORD tickCount, DWORD lastTickCount);
	void Draw (DWORD tickCount);

private:
	void NewGame (DWORD tickCount);
	void UpdateLives (bool generate);
	void NewGame_Sub (DWORD tickCount);
	void EnterDinosaur (DWORD tickCount);

	enum GameStates
	{
		ActualGame,
		GameOver,
		TitlePicture,
		ActualGame_Dinosaur,
		GameCompleted
	} gameState;

	GL_Window *g_window;
	Keys *g_keys;
 	bool spacePressed;
	int spacePressedSince;
	bool escapePressed;
	bool keyPressedUpDown;
	bool keyPressedLeftRight;
	int keyPressedUpDownSince;
	int keyPressedLeftRightSince;
	int blinkingDuration;
	Bitmap bitmap [20];
	GLuint texture [20];
	Position playerPosition;
	Position bulletPosition [10];
	bool bulletActive [10];
	int lastBulletMove;
	Enemy enemy [20];
	int lastEnemyAppeared;
	bool enemyActive [20];
	int startTime;
	int lives;
	Bitmap bitmap_lives;
	GLuint texture_lives;
	Bitmap bitmap_titlescreen;
	GLuint texture_titlescreen;
	Bitmap bitmap_gameover;
	GLuint texture_gameover;
	Position platformPosition [15];
	bool platformActive [15];
	bool dinosaurMoving;
	int jumping;
	int falling;
	int movingswitchedSince;
	bool movingDirection;
	bool gap;
	int rightmost;
	bool bulletDirection [10];
	int completed;
	Bitmap bitmap_gamecompleted;
	GLuint texture_gamecompleted;
	bool scrolling;
	int upward;
	int lastVerticalMove;
	HSTREAM soundeffect [6];
};

#endif