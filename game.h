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

class Game
{
public:
	bool Initialize (GL_Window *window, Keys *keys, DWORD tickCount);
	void Deinitialize ();
	void Update (DWORD tickCount, DWORD lastTickCount);
	void Draw (DWORD tickCount);

	inline static const int MAXNUMPLAYERS = 1;
	inline static const int MAXNUMBULLETS = 10;
	inline static const int MAXNUMPLATFORMS = 15;
	inline static const int MINNUMPLATFORMS = 10;
	inline static const int MINNUMENEMIES = 10;
	inline static const int MAXNUMENEMIES = 20;
	inline static const int NUMBITMAPS = 20;

	inline static enum GameStates
	{
		ActualGame,
		GameOver,
		TitlePicture,
		ActualGame_Dinosaur,
		GameCompleted
	} gameState;

	inline static GLuint texture[20];

	class Position
	{
	public:
		int x, y;
	};

	class Size
	{
	public:
		int width, height;
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

	class Player
	{
	public:
		GLuint textureDefault;
		GLuint textureMoving;
		Position position;
		Size size;

		Player()
		{
  			switch (gameState)
			{
			case TitlePicture:
			case ActualGame:
				textureDefault = texture[0];
				size.width = 64;
				size.height = 32;
				break;

			case ActualGame_Dinosaur:
				textureDefault = texture[9];
				textureMoving = texture[10];
				size.width = 64;
				size.height = 128;
				break;
			}
		}
	};

	class Enemy
	{
	public:
		GLuint textureDefault;
		Position position;
		Size size;
		EnemyType type;
		bool moving;
		Position whereToGo;
		bool active;
		int variable[10];
	};

	class Bullet
	{
	public:
		GLuint textureDefault;
		Position position;
		Size size;
		bool active;
		bool direction;

		Bullet()
		{
			switch (gameState)
			{
			case TitlePicture:
			case ActualGame:
				textureDefault = texture[1];
				break;

			case ActualGame_Dinosaur:
				textureDefault = texture[11];
				break;
			}

			size.width = 32;
			size.height = 32;
		}
	};

	class Platform
	{
	public:
		GLuint textureDefault;
		Position position;
		Size size;
		bool active;

		Platform()
		{
			textureDefault = texture[8];
			size.width = 64;
			size.height = 16;
		}
	};

private:
	void NewGame (DWORD tickCount);
	void UpdateLives (bool generate);
	void NewGame_Sub (DWORD tickCount);
	void EnterDinosaur (DWORD tickCount);

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
	Bullet bullet [10];
	int lastBulletMove;
	Player player [1];
	Enemy enemy [20];
	int lastEnemyAppeared;
	int startTime;
	int lives;
	Bitmap bitmap_lives;
	GLuint texture_lives;
	Bitmap bitmap_titlescreen;
	GLuint texture_titlescreen;
	Bitmap bitmap_gameover;
	GLuint texture_gameover;
	Platform platform[15];
	bool dinosaurMoving;
	int jumping;
	int falling;
	int movingswitchedSince;
	bool movingDirection;
	bool gap;
	int rightmost;
	int completed;
	Bitmap bitmap_gamecompleted;
	GLuint texture_gamecompleted;
	bool scrolling;
	int upward;
	int lastVerticalMove;
	HSTREAM soundeffect [6];
};

#endif