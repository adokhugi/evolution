#include "game.h"

bool Game::Initialize (GL_Window* window, Keys* keys, DWORD tickCount)	
{
  int i;

  g_window			= window;
  g_keys	  		= keys;

  glEnable (GL_POINT_SMOOTH);
  glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
  glEnable (GL_LINE_SMOOTH);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

  glClearColor (0, 0.0, 0.3f, 1);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluOrtho2D (0.0, 640.0, 0.0, 480.0);

  glAlphaFunc (GL_GREATER, 0.1f);
  glEnable (GL_ALPHA_TEST);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE);

  GLuint color [3] = {0, 0, 0};

  if (!bitmap [0].LoadFile ("fish.bmp", true, color)) return false;
  if (!bitmap [1].LoadFile ("bullet.bmp", true, color)) return false;
  if (!bitmap [2].LoadFile ("enemy_bullet.bmp", true, color)) return false;
  if (!bitmap [3].LoadFile ("enemy_cancer.bmp", true, color)) return false;
  if (!bitmap [4].LoadFile ("enemy_cancerbullet.bmp", true, color)) return false;
  if (!bitmap [5].LoadFile ("enemy_jellyfish.bmp", true, color)) return false;
  if (!bitmap [6].LoadFile ("enemy_kraken.bmp", true, color)) return false;
  if (!bitmap [7].LoadFile ("enemy_seastar.bmp", true, color)) return false;
  if (!bitmap [8].LoadFile ("platform.bmp", true, color)) return false;
  if (!bitmap [9].LoadFile ("dino_stand.bmp", true, color)) return false;
  if (!bitmap [10].LoadFile ("dino_move.bmp", true, color)) return false;
  if (!bitmap [11].LoadFile ("dino_bullet.bmp", true, color)) return false;
  if (!bitmap [12].LoadFile ("enemy_bulletdino.bmp", true, color)) return false;
  if (!bitmap [13].LoadFile ("enemy_lion.bmp", true, color)) return false;
  if (!bitmap [14].LoadFile ("enemy_worm.bmp", true, color)) return false;
  if (!bitmap_titlescreen.LoadFile ("titlescreen.bmp", true, color)) return false;
  if (!bitmap_gameover.LoadFile ("gameover.bmp", true, color)) return false;
  if (!bitmap_gamecompleted.LoadFile ("gamecompleted.bmp", true, color)) return false;
  if (!bitmap_lives.New (64, 16)) return false;

  bitmap_titlescreen.GenerateTexture (texture_titlescreen);
  bitmap_gameover.GenerateTexture (texture_gameover);
  bitmap_gamecompleted.GenerateTexture (texture_gamecompleted);
  for (i = 0; i < NUMBITMAPS; i++)
    bitmap [i].GenerateTexture (texture [i]);

  BASS_Init (-1, 44100, 0, NULL, NULL);
  soundeffect [0] = BASS_StreamCreateFile (FALSE, "enemy_shoot.wav", 0, 0, 0);
  soundeffect [1] = BASS_StreamCreateFile (FALSE, "gameover.wav", 0, 0, 0);
  soundeffect [2] = BASS_StreamCreateFile (FALSE, "hit.wav", 0, 0, 0);
  soundeffect [3] = BASS_StreamCreateFile (FALSE, "jump.wav", 0, 0, 0);
  soundeffect [4] = BASS_StreamCreateFile (FALSE, "loselife.wav", 0, 0, 0);
  soundeffect [5] = BASS_StreamCreateFile (FALSE, "shoot.wav", 0, 0, 0);

  gameState = TitlePicture;
  for (i = 0; i < MAXNUMPLAYERS; i++)
      player[0] = Player();
  for (i = 0; i < MAXNUMBULLETS; i++)
      bullet[i] = Bullet();
  for (i = 0; i < MAXNUMPLATFORMS; i++)
      platform[i] = Platform();

  srand (tickCount);
  spacePressed = false;

  return true;
}

void Game::NewGame (DWORD tickCount)
{
  NewGame_Sub (tickCount);
  glClearColor (0, 0, 0.3f, 1);
  gameState = ActualGame;
  player[0] = Player();
  player[0].position.x = 20;
  player[0].position.y = 200;
  for (int i = 0; i < MAXNUMBULLETS; i++)
      bullet[i] = Bullet();
  lives = 5;
  UpdateLives (true);
}

void Game::EnterDinosaur (DWORD tickCount)
{
  int i;

  NewGame_Sub (tickCount);
  glClearColor (0, 0, 0.6f, 1);
  gameState = ActualGame_Dinosaur;
  for (i = 0; i < MINNUMPLATFORMS; i++)
  {
    platform[i] = Platform();
    platform[i].position.x = i * 64;
    platform[i].position.y = 0;
    platform[i].active = true;
  }
  for (; i < MAXNUMPLATFORMS; i++)
    platform[i].active = false;
  player[0] = Player();
  player[0].position.x = 20;
  player[0].position.y = 16;
  for (i = 0; i < MAXNUMBULLETS; i++)
      bullet[i] = Bullet();
  dinosaurMoving = false;
  jumping = 0;
  falling = 0;
  movingswitchedSince = 0;
  movingDirection = true;
  rightmost = 9;
  gap = false;
  completed = 0;
  scrolling = false;
  upward = 0;
  lastVerticalMove = 0;
}

void Game::NewGame_Sub (DWORD tickCount)
{
  int i;

  for (i = 0; i < MAXNUMBULLETS; i++)
    bullet[i].active = false;
  for (i = 0; i < MAXNUMENEMIES; i++)
    enemy[i].active = false;
  lastBulletMove = 0;
  keyPressedLeftRightSince = 0;
  keyPressedUpDownSince = 0;
  spacePressedSince = 0;
  spacePressed = false;
  escapePressed = false;
  keyPressedLeftRight = false;
  keyPressedUpDown = false;
  lastEnemyAppeared = 0;
  startTime = tickCount;
}

void Game::Deinitialize ()
{
}

void Game::Update (DWORD tickCount, DWORD lastTickCount)
{
  int i, j;

  if (escapePressed && !g_keys->keyDown [VK_ESCAPE])
  {
    if (gameState == TitlePicture)
      exit (1);
    else if (gameState == ActualGame)
    {
      glClearColor (0, 0, 0.3f, 1);
      escapePressed = false;
      spacePressed = false;
      gameState = GameOver;
      BASS_ChannelPlay (soundeffect [1], TRUE);
    }
    else
    {
      glClearColor (0, 0, 0.3f, 1);
      escapePressed = false;
      spacePressed = false;
      gameState = TitlePicture;
    }
  }
  else if (!escapePressed && g_keys->keyDown [VK_ESCAPE])
    escapePressed = true;

  if (gameState == GameOver)
  {
    if (!spacePressed && g_keys->keyDown [VK_SPACE])
      spacePressed = true;

    if (spacePressed && !g_keys->keyDown [VK_SPACE])
    {
      glClearColor (0, 0, 0.3f, 1);
      spacePressed = false;
      gameState = TitlePicture;
      return;
    }
  }

  if (gameState == TitlePicture)
  {
    if (!spacePressed && g_keys->keyDown [VK_SPACE])
      spacePressed = true;

    if (spacePressed && !g_keys->keyDown [VK_SPACE])
    {
      NewGame (tickCount);
      return;
    }

    // cheat: 300 lives
    if (g_keys->keyDown [VK_F9])
    {
      NewGame (tickCount);
      lives = 300;
      UpdateLives (false);
      return;
    }

    // cheat to skip fish mode
    if (g_keys->keyDown [VK_F7])
    {
      lives = 5;
      UpdateLives (true);
      EnterDinosaur (tickCount);
      return;
    }
  }

  if (gameState == ActualGame)
  {
    if (tickCount - startTime > 300000)
    {
      EnterDinosaur (tickCount);
      return;
    }

    if (g_keys->keyDown [VK_DOWN])
    {
      if (!keyPressedUpDown || keyPressedUpDownSince + 10 < tickCount)
      {
        keyPressedUpDown = true;
        keyPressedUpDownSince = tickCount;
        if (player[0].position.y > 70)
          player[0].position.y -= 3;
      }
    }
    else if (g_keys->keyDown [VK_UP])
    {
      if (!keyPressedUpDown || keyPressedUpDownSince + 10 < tickCount)
      {
        keyPressedUpDown = true;
        keyPressedUpDownSince = tickCount;
        if (player[0].position.y < 416)
          player[0].position.y += 3;
      }
    }
    else if (!g_keys->keyDown [VK_DOWN] && !g_keys->keyDown [VK_UP])
      keyPressedUpDown = false;

    if (g_keys->keyDown [VK_LEFT])
    {
      if (!keyPressedLeftRight || keyPressedLeftRightSince + 10 < tickCount)
      {
        keyPressedLeftRight = true;
        keyPressedLeftRightSince = tickCount;
        if (player[0].position.x > 2)
          player[0].position.x -= 3;
      }
    }
    else if (g_keys->keyDown [VK_RIGHT])
    {
      if (!keyPressedLeftRight || keyPressedLeftRightSince + 10 < tickCount)
      {
        keyPressedLeftRight = true;
        keyPressedLeftRightSince = tickCount;
        if (player[0].position.x < 400)
          player[0].position.x += 3;
      }
    }
    else if (!g_keys->keyDown [VK_LEFT] && !g_keys->keyDown [VK_RIGHT])
      keyPressedLeftRight = false;

    if (g_keys->keyDown [VK_SPACE])
    {
      if (spacePressedSince + 800 < tickCount)
      {
        spacePressed = true;
        spacePressedSince = tickCount;
        for (i = 0; i < MAXNUMBULLETS && bullet[i].active; i++);
        if (i < MAXNUMBULLETS)
        {
          bullet[i].position.x = player[0].position.x + 64;
          bullet[i].position.y = player[0].position.y;
          bullet[i].active = true;
          BASS_ChannelPlay (soundeffect [5], TRUE);
        }
      }
    }
    else
      spacePressed = false;

    if (lastBulletMove + 5 < tickCount)
    {
      lastBulletMove = tickCount;
      for (i = 0; i < MAXNUMBULLETS; i++)
        if (bullet[i].active)
        {
          bullet[i].position.x += 5;
          if (bullet[i].position.x >= 640)
            bullet[i].active = false;
        }

      for (i = 0; i < MAXNUMENEMIES; i++)
        if (enemy[i].active)
        {
          if (player[0].position.x + 64 >= enemy [i].position.x
            && player[0].position.x <= enemy [i].position.x + enemy [i].size.width
            && player[0].position.y + 32 >= enemy [i].position.y
            && player[0].position.y <= enemy [i].position.y + enemy [i].size.height)
          {
            lives--;
            UpdateLives (false);
            enemy[i].active = false;
            BASS_ChannelPlay (soundeffect [4], TRUE);
          }

          for (j = 0; j < 10 && enemy[i].active; j++)
              if (bullet[j].active
                && bullet[j].position.x + 64 >= enemy [i].position.x
                && bullet[j].position.x <= enemy [i].position.x + enemy [i].size.width
                && bullet[j].position.y + 32 >= enemy [i].position.y
                && bullet[j].position.y <= enemy [i].position.y + enemy [i].size.height)
              {
                bullet[j].active = false;

                if (enemy [i].type == Jellyfish || enemy [i].type == Kraken 
                  || enemy [i].type == EnemyBullet || enemy [i].type == CancerBullet)
                {
                  BASS_ChannelPlay (soundeffect [2], TRUE);
                  enemy[i].active = false;
                }
              }

          if (enemy[i].active)
          {
            enemy [i].position.x -= 3;
            if (enemy [i].position.x < -enemy [i].size.width)
              enemy[i].active = false;
            else
            {
              switch (enemy [i].type)
              {
              case Kraken:
                if (enemy [i].moving)
                {
                  if (enemy [i].position.y < enemy [i].whereToGo.y)
                    enemy [i].position.y++;
                  else if (enemy [i].position.y > enemy [i].whereToGo.y)
                    enemy [i].position.y--;

                  if (enemy [i].position.y == enemy [i].whereToGo.y)
                    enemy [i].moving = false;
                }
                else
                {
                  enemy [i].moving = true;
                  if (enemy [i].variable [0] == 0)
                  {
                    enemy [i].whereToGo.y = enemy [i].position.y - 20;
                    enemy [i].variable [0] = 1;
                  }
                  else
                  {
                    enemy [i].whereToGo.y = enemy [i].position.y + 20;
                    enemy [i].variable [0] = 0;
                  }
                }

                if (enemy [i].variable [1] + 1000 < tickCount)
                {
                  enemy [i].variable [1] = tickCount;
                  if (rand () % 2)
                  {
                    for (j = 0; j < 20 && enemy[j].active; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy[j].type = EnemyBullet;
                      enemy[j].textureDefault = texture[2];
                      enemy[j].size.width = 64;
                      enemy[j].size.height = 32;
                      enemy[j].position.x = enemy[i].position.x - 64 - 1;
                      enemy[j].position.y = enemy[i].position.y + 48;
                      enemy[j].active = true;
                    }
                  }
                }
                break;

              case EnemyBullet:
                enemy [i].position.x -= 5;
                for (j = 0; j < 20 && enemy[i].active; j++)
                  if (enemy[j].active && j != i
                    && enemy [i].position.x + enemy [i].size.width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].size.width
                    && enemy [i].position.y + enemy [i].size.height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].size.height)
                    enemy[i].active = false;
                break;

              case Jellyfish:
                if (enemy [i].moving)
                {
                  if (enemy [i].position.y < enemy [i].whereToGo.y)
                  {
                    enemy [i].position.y += 4;
                    if (enemy [i].position.y > enemy [i].whereToGo.y)
                      enemy [i].position.y = enemy [i].whereToGo.y;
                  }
                  else if (enemy [i].position.y > enemy [i].whereToGo.y)
                  {
                    enemy [i].position.y -= 4;
                    if (enemy [i].position.y < enemy [i].whereToGo.y)
                      enemy [i].position.y = enemy [i].whereToGo.y;
                  }

                  if (enemy [i].position.y == enemy [i].whereToGo.y)
                    enemy [i].moving = false;
                }
                else
                {
                  enemy [i].moving = true;
                  if (enemy [i].variable [0] == 0)
                  {
                    enemy [i].whereToGo.y = enemy [i].position.y - 100 - rand () % 200;
                    if (enemy [i].whereToGo.y < enemy [i].size.height)
                      enemy [i].whereToGo.y = enemy [i].size.height;
                    enemy [i].variable [0] = 1;
                  }
                  else
                  {
                    enemy [i].whereToGo.y = enemy [i].position.y + 100 - rand () % 200;
                    if (enemy [i].whereToGo.y > 480 - enemy [i].size.height)
                      enemy [i].whereToGo.y = 480 - enemy [i].size.height;
                    enemy [i].variable [0] = 0;
                  }
                }
              break;

              case Cancer:
                if (enemy [i].variable [1] + 800 < tickCount)
                {
                  enemy [i].variable [1] = tickCount;
                  if (rand () % 2)
                  {
                    for (j = 0; j < 20 && enemy[j].active; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy[j].type = CancerBullet;
                      enemy[j].textureDefault = texture[4];
                      enemy[j].size.width = 32;
                      enemy [j].size.height = 32;
                      enemy[j].position.x = enemy[i].position.x + 48;
                      enemy[j].position.y = enemy[i].position.y + 64 + 1;
                      enemy[j].whereToGo.x = 1 + rand () % 3;
                      if (rand () % 2)
                        enemy[j].whereToGo.x = -enemy[j].whereToGo.x;
                      enemy[j].active = true;
                    }
                  }
                }
                break;

              case CancerBullet:
                enemy [i].position.x -= enemy [i].whereToGo.x;
                enemy [i].position.y += 5;
                for (j = 0; j < 20 && enemy[i].active; j++)
                  if (enemy[j].active && j != i
                    && enemy [i].position.x + enemy [i].size.width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].size.width
                    && enemy [i].position.y + enemy [i].size.height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].size.height)
                    enemy[i].active = false;
                break;

              }
            }
          }
        }
    }

    if ((lastEnemyAppeared + 3000 < tickCount)
      || (lastEnemyAppeared + 750 - (tickCount - startTime) / 1000 < tickCount && rand () % 2))
    {
      for (i = 0; i < MAXNUMENEMIES && enemy[i].active; i++);
      if (i < MAXNUMENEMIES)
      {
        for (j = 0; j < MINNUMENEMIES; j++)
          enemy[i].variable[j] = 0;
        switch (rand () % 10)
        {
        case 0:
        case 1:
          enemy[i].type = Kraken;
          enemy[i].textureDefault = texture[6];
          enemy[i].size.width = 128;
          enemy[i].size.height = 128;
          enemy[i].position.y = 128 + rand () % (480 - 128 * 2);
          break;

        case 2:
        case 3:
        case 4:
          enemy[i].type = Jellyfish;
          enemy[i].textureDefault = texture[5];
          enemy[i].size.width = 64;
          enemy[i].size.height = 128;
          enemy[i].position.y = 128 + rand () % (480 - 128 * 2);
          enemy[i].variable [0] = rand () % 2;
          break;

        case 5:
          enemy[i].type = Cancer;
          enemy[i].textureDefault = texture[3];
          enemy[i].size.width = 128;
          enemy[i].size.height = 64;
          enemy[i].position.y = 0;
          break;

        default:
          enemy[i].type = Seastar;
          enemy[i].textureDefault = texture[7];
          enemy[i].size.width = 64;
          enemy[i].size.height = 64;
          enemy[i].position.y = 64 + rand () % (480 - 64 * 2);
          break;
        }
        enemy[i].moving = false;
        enemy[i].position.x = 640;
        enemy[i].active = true;
        lastEnemyAppeared = tickCount;
      }
    }
  }

  if (gameState == ActualGame_Dinosaur)
  {
    if (completed == 800)
    {
      glClearColor (0, 0, 0.3f, 1);
      gameState = GameCompleted;
      return;
    }

    if (tickCount > lastVerticalMove + 10)
    {
      lastVerticalMove = tickCount;
      if (jumping)
      {
        player[0].position.y += jumping;
        jumping--;
        if (!jumping)
          falling = 1;
      }
      else
      {
        keyPressedUpDown = true;

        if (!falling)
          falling = 1;

        for (i = 0; i < MAXNUMPLATFORMS && falling; i++)
          if (platform[i].active 
            && platform[i].position.x <= player[0].position.x + 64
            && platform[i].position.x + 64 >= player[0].position.x + 10
            && platform[i].position.y <= player[0].position.y + 80
            && platform[i].position.y + 16 >= player[0].position.y)
            {
              player[0].position.y = platform[i].position.y + 16;
              falling = 0;
              keyPressedUpDown = false;
            }

        if (falling)
        {
          if (player[0].position.y < -128)
          {
            int leftmost = 0;
            lives--;
            UpdateLives (false);
            BASS_ChannelPlay (soundeffect [4], TRUE);
            for (i = 0; i < MAXNUMPLATFORMS && falling; i++)
            {
              if (platform[i].position.x > 0
                && (platform[i].position.x < platform[leftmost].position.x
                  || platform[leftmost].position.x < 0))
                leftmost = i;
              if (platform[i].active 
                && platform[i].position.x <= player[0].position.x + 64
                && platform[i].position.x + 64 >= player[0].position.x + 10)
                {
                  player[0].position.y = platform[i].position.y + 16;
                  falling = 0;
                }
            }
            if (falling)
            {
              player[0].position.x = platform[leftmost].position.x;
              player[0].position.y = platform[leftmost].position.y + 16;
            }
          }
          player[0].position.y -= falling;
          falling++;
        }
      }
    }

    if (g_keys->keyDown [VK_UP])
    {
      if (!keyPressedUpDown)
      {
        keyPressedUpDown = true;
        jumping = 20;
        falling = 0;
        BASS_ChannelPlay (soundeffect [3], TRUE);
      }
    }
    else if (keyPressedUpDown && !falling)
      jumping = 1;
    scrolling = false;
    if (g_keys->keyDown [VK_LEFT])
    {
      if (!keyPressedLeftRight || keyPressedLeftRightSince + 10 < tickCount)
      {
        keyPressedLeftRight = true;
        keyPressedLeftRightSince = tickCount;
        if (player[0].position.x > 2)
          player[0].position.x -= 3;
        if (movingswitchedSince + 60 < tickCount)
        {
          movingswitchedSince = tickCount;
          dinosaurMoving = !dinosaurMoving;
        }
        movingDirection = false;
      }
    }
    else if (g_keys->keyDown [VK_RIGHT])
    {
      if (!keyPressedLeftRight || keyPressedLeftRightSince + 10 < tickCount)
      {
        keyPressedLeftRight = true;
        keyPressedLeftRightSince = tickCount;
        if (player[0].position.x < 320)
          player[0].position.x += 3;
        else
        {
          scrolling = true;
          j = 15;
          for (i = 0; i < MAXNUMPLATFORMS; i++)
          {
            if (platform[i].active)
            {
              platform[i].position.x -= 3;
              if (platform[i].position.x < -64)
              {
                platform[i].active = false;
                completed++;
              }
            }
            else
              j = i;
          }

          if (j < 15)
          {
            if (!gap && !(rand () % 6))
              gap = true;
            else
            {
              platform[j].position.x = platform[rightmost].position.x + 64;
              if (gap)
                platform[j].position.x += 100;
              platform[j].position.y = platform[rightmost].position.y;
              upward++;
              if (!(rand () % 4))
              {
                int temp = rand () % 120 + 100;
                if (rand () % 2)
                  temp = -temp;
                while (platform[j].position.y + temp < 0 || platform[j].position.y + temp > 360)
                {
                  if (platform[j].position.y - temp < 0 || platform[j].position.y - temp > 360)
                  {
                    temp /= 4;
                    temp *= 3;
                  }
                  else
                    temp = -temp;
                }
                if (temp < 0)
                  upward = 0;
                platform[j].position.y += temp;
              }
              platform[j].active = true;
              rightmost = j;
              gap = false;
            }
          }
        }

        if (movingswitchedSince + 60 < tickCount)
        {
          movingswitchedSince = tickCount;
          dinosaurMoving = !dinosaurMoving;
        }
        movingDirection = true;
      }
    }
    else if (!g_keys->keyDown [VK_LEFT] && !g_keys->keyDown [VK_RIGHT])
      keyPressedLeftRight = false;

    if (g_keys->keyDown [VK_SPACE])
    {
      if (spacePressedSince + 800 < tickCount)
      {
        spacePressed = true;
        spacePressedSince = tickCount;
        for (i = 0; i < MAXNUMBULLETS && bullet[i].active; i++);
        if (i < MAXNUMBULLETS)
        {
          if (movingDirection)
            bullet[i].position.x = player[0].position.x + 64;
          else
            bullet[i].position.x = player[0].position.x - 32;
          bullet[i].position.y = player[0].position.y + 60;
          bullet[i].direction = movingDirection;
          bullet[i].active = true;
          BASS_ChannelPlay (soundeffect [5], TRUE);
        }
      }
    }
    else
      spacePressed = false;

    if (lastBulletMove + 5 < tickCount)
    {
      lastBulletMove = tickCount;
      for (i = 0; i < MAXNUMBULLETS; i++)
        if (bullet[i].active)
        {
          if (bullet[i].direction)
            bullet[i].position.x += 5;
          else
            bullet[i].position.x -= 5;
          bullet[i].position.y--;
          if (bullet[i].position.x >= 640 || bullet[i].position.x < 0 || bullet[i].position.y < 0)
            bullet[i].active = false;
        }

      for (i = 0; i < MAXNUMENEMIES; i++)
        if (enemy[i].active)
        {
          if (player[0].position.x + 64 >= enemy [i].position.x
            && player[0].position.x <= enemy [i].position.x + enemy [i].size.width
            && player[0].position.y + 80 >= enemy [i].position.y
            && player[0].position.y <= enemy [i].position.y + enemy [i].size.height)
          {
            lives--;
            UpdateLives (false);
            enemy[i].active = false;
            BASS_ChannelPlay (soundeffect [4], TRUE);
          }
          for (j = 0; j < 10 && enemy[i].active; j++)
              if (bullet[j].active
                && bullet[j].position.x + 32 >= enemy [i].position.x
                && bullet[j].position.x <= enemy [i].position.x + enemy [i].size.width
                && bullet[j].position.y + 32 >= enemy [i].position.y
                && bullet[j].position.y <= enemy [i].position.y + enemy [i].size.height)
              {
                BASS_ChannelPlay (soundeffect [2], TRUE);
                bullet[j].active = false;
                enemy[i].active = false;
              }

          if (enemy[i].active)
          {
            if (scrolling)
              enemy [i].position.x -= 3;
            if (enemy [i].position.x < -enemy [i].size.width)
              enemy[i].active = false;
            else
            {
              switch (enemy [i].type)
              {
                case Worm:
                if (enemy [i].variable [1] + 1600 < tickCount)
                {
                  enemy [i].variable [1] = tickCount;
                  if (rand () % 3)
                  {
                    for (j = 0; j < 20 && enemy[j].active; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy[j].type = EnemyBullet;
                      enemy[j].textureDefault = texture[12];
                      enemy[j].size.width = 32;
                      enemy[j].size.height = 32;
                      enemy[j].position.x = enemy[i].position.x - 32;
                      enemy[j].position.y = enemy[i].position.y + 80;
                      enemy[j].whereToGo.y = 0;
                      enemy[j].active = true;
                    }
                  }
                }
                break;

                case Lion:
                if (enemy [i].variable [1] + 800 < tickCount)
                {
                  enemy [i].variable [1] = tickCount;
                  if (rand () % 2)
                  {
                    for (j = 0; j < 20 && enemy[j].active; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy[j].type = EnemyBullet;
                      enemy[j].textureDefault = texture[12];
                      enemy[j].size.width = 32;
                      enemy[j].size.height = 32;
                      enemy[j].position.x = enemy[i].position.x - 32;
                      enemy[j].position.y = enemy[i].position.y + 80;
                      enemy[j].whereToGo.y = 1 + rand () % 3;
                      if (rand () % 2)
                        enemy[j].whereToGo.y = -enemy[j].whereToGo.y;
                      enemy[j].active = true;
                    }
                  }
                }
                break;

              case EnemyBullet:
                enemy [i].position.x -= 5;
                enemy [i].position.y += enemy [i].whereToGo.y;
                for (j = 0; j < 20 && enemy[i].active; j++)
                  if (enemy[j].active && j != i
                    && enemy [i].position.x + enemy [i].size.width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].size.width
                    && enemy [i].position.y + enemy [i].size.height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].size.height)
                    enemy[i].active = false;
                break;
              }
            }
          }
        }

        for (i = 0; i < MAXNUMPLATFORMS; i++)
          if (platform[i].active)
            for (j = 0; j < MAXNUMBULLETS; j++)
              if (bullet[j].active
                && bullet[j].position.x + 32 >= platform[i].position.x
                && bullet[j].position.x <= platform[i].position.x + 64
                && bullet[j].position.y + 32 >= platform[i].position.y
                && bullet[j].position.y <= platform[i].position.y + 16)
                bullet[j].active = false;
    }

    if (upward > 2
      && lastEnemyAppeared + 2 < completed
      && ((lastEnemyAppeared + 20 < completed)
      || (lastEnemyAppeared + 5 - completed / 100 < completed && rand () % 2)))
    {
      for (i = 0; i < MAXNUMENEMIES && enemy[i].active; i++);
      if (i < MAXNUMENEMIES)
      {
        for (j = 0; j < MINNUMENEMIES; j++)
          enemy [i].variable [j] = 0;
        switch (rand () % 10)
        {
        case 0:
        case 1:
        case 2:
          enemy[i].type = Lion;
          enemy[i].textureDefault = texture[13];
          enemy[i].size.width = 128;
          enemy[i].size.height = 118;
          enemy[i].position.x = platform[rightmost].position.x;
          enemy[i].position.y = platform[rightmost].position.y + 16;
          break;

        default:
          enemy[i].type = Worm;
          enemy[i].textureDefault = texture[14];
          enemy[i].size.width = 64;
          enemy[i].size.height = 98;
          enemy[i].position.x = platform[rightmost].position.x;
          enemy[i].position.y = platform[rightmost].position.y + 16;
          break;
        }
        enemy[i].moving = false;
        enemy[i].active = true;
        lastEnemyAppeared = completed;
      }
    }
  }
}

void Game::Draw (DWORD tickCount)
{
  int i;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  switch (gameState)
  {
  case TitlePicture:
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, texture_titlescreen);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 0, 0);
      glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 512, 0);
      glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 512, 512);
      glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 0, 512);
    glEnd ();
    glDisable (GL_TEXTURE_2D);
    break;

  case GameOver:
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, texture_gameover);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 0, 0);
      glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 512, 0);
      glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 512, 512);
      glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 0, 512);
    glEnd ();
    glDisable (GL_TEXTURE_2D);
    break;

  case GameCompleted:
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, texture_gamecompleted);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 0, 0);
      glTexCoord2f (1.0f, 0.0f); glVertex2f ((640 - 512) / 2 + 512, 0);
      glTexCoord2f (1.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 512, 512);
      glTexCoord2f (0.0f, 1.0f); glVertex2f ((640 - 512) / 2 + 0, 512);
    glEnd ();
    glDisable (GL_TEXTURE_2D);
    break;

  case ActualGame:
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, texture_lives);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f (0, 480 - 16);
      glTexCoord2f (1.0f, 0.0f); glVertex2f (64, 480 - 16);
      glTexCoord2f (1.0f, 1.0f); glVertex2f (64, 480);
      glTexCoord2f (0.0f, 1.0f); glVertex2f (0, 480);
    glEnd ();
    glDisable (GL_TEXTURE_2D);

    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, player[0].textureDefault);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f (player[0].position.x, player[0].position.y);
      glTexCoord2f (1.0f, 0.0f); glVertex2f (player[0].position.x + player[0].size.width, player[0].position.y);
      glTexCoord2f (1.0f, 1.0f); glVertex2f (player[0].position.x + player[0].size.width, player[0].position.y + player[0].size.height);
      glTexCoord2f (0.0f, 1.0f); glVertex2f (player[0].position.x, player[0].position.y + player[0].size.height);
    glEnd ();
    glDisable (GL_TEXTURE_2D);

    for (i = 0; i < MAXNUMBULLETS; i++)
      if (bullet[i].active)
      {
        glEnable (GL_TEXTURE_2D);
        glBindTexture (GL_TEXTURE_2D, bullet[i].textureDefault);
        glColor3f (1.0f, 1.0f, 1.0f);
        glBegin (GL_QUADS);
          glTexCoord2f (0.0f, 0.0f); glVertex2f (bullet[i].position.x, bullet[i].position.y);
          glTexCoord2f (1.0f, 0.0f); glVertex2f (bullet[i].position.x + bullet[i].size.width, bullet[i].position.y);
          glTexCoord2f (1.0f, 1.0f); glVertex2f (bullet[i].position.x + bullet[i].size.width, bullet[i].position.y + bullet[i].size.height);
          glTexCoord2f (0.0f, 1.0f); glVertex2f (bullet[i].position.x, bullet[i].position.y + bullet[i].size.height);
        glEnd ();
        glDisable (GL_TEXTURE_2D);
      }

    for (i = 0; i < MAXNUMENEMIES; i++)
      if (enemy[i].active)
      {
        glEnable (GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, enemy[i].textureDefault);
        glColor3f (1.0f, 1.0f, 1.0f);
        glBegin (GL_QUADS);
        glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
        glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + enemy[i].size.width, enemy[i].position.y);
        glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + enemy[i].size.width, enemy [i].position.y + enemy[i].size.height);
        glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + enemy[i].size.height);
        glEnd ();
        glDisable (GL_TEXTURE_2D);
      }
      break;

    case ActualGame_Dinosaur:
      glEnable (GL_TEXTURE_2D);
      glBindTexture (GL_TEXTURE_2D, texture_lives);
      glColor3f (1.0f, 1.0f, 1.0f);
      glBegin (GL_QUADS);
        glTexCoord2f (0.0f, 0.0f); glVertex2f (0, 480 - 16);
        glTexCoord2f (1.0f, 0.0f); glVertex2f (64, 480 - 16);
        glTexCoord2f (1.0f, 1.0f); glVertex2f (64, 480);
        glTexCoord2f (0.0f, 1.0f); glVertex2f (0, 480);
      glEnd ();
      glDisable (GL_TEXTURE_2D);

      glEnable (GL_TEXTURE_2D);
      if (dinosaurMoving)
        glBindTexture (GL_TEXTURE_2D, player[0].textureMoving);
      else
        glBindTexture (GL_TEXTURE_2D, player[0].textureDefault);
      glColor3f (1.0f, 1.0f, 1.0f);
      glBegin (GL_QUADS);
        glTexCoord2f (0.0f, 0.0f); glVertex2f (player[0].position.x, player[0].position.y);
        glTexCoord2f (1.0f, 0.0f); glVertex2f (player[0].position.x + player[0].size.width, player[0].position.y);
        glTexCoord2f (1.0f, 1.0f); glVertex2f (player[0].position.x + player[0].size.width, player[0].position.y + player[0].size.height);
        glTexCoord2f (0.0f, 1.0f); glVertex2f (player[0].position.x, player[0].position.y + player[0].size.height);
      glEnd ();
      glDisable (GL_TEXTURE_2D);

      for (i = 0; i < MAXNUMPLATFORMS; i++)
        if (platform[i].active)
        {
          glEnable (GL_TEXTURE_2D);
          glBindTexture (GL_TEXTURE_2D, platform[i].textureDefault);
          glColor3f (1.0f, 1.0f, 1.0f);
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (platform[i].position.x, platform[i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (platform[i].position.x + platform[i].size.width, platform[i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (platform[i].position.x + platform[i].size.width, platform[i].position.y + platform[i].size.height);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (platform[i].position.x, platform[i].position.y + platform[i].size.height);
          glEnd ();
          glDisable (GL_TEXTURE_2D);
        }

      for (i = 0; i < MAXNUMBULLETS; i++)
        if (bullet[i].active)
        {
          glEnable (GL_TEXTURE_2D);
          glBindTexture (GL_TEXTURE_2D, bullet[i].textureDefault);
          glColor3f (1.0f, 1.0f, 1.0f);
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (bullet[i].position.x, bullet[i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (bullet[i].position.x + bullet[i].size.width, bullet[i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (bullet[i].position.x + bullet[i].size.width, bullet[i].position.y + bullet[i].size.height);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (bullet[i].position.x, bullet[i].position.y + bullet[i].size.height);
          glEnd ();
          glDisable (GL_TEXTURE_2D);
        }

      for (i = 0; i < MAXNUMENEMIES; i++)
        if (enemy[i].active)
        {
          glEnable (GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, enemy[i].textureDefault);
          glColor3f (1.0f, 1.0f, 1.0f);
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + enemy[i].size.width, enemy[i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + enemy[i].size.width, enemy [i].position.y + enemy[i].size.height);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + enemy[i].size.height);
          glEnd ();
        }
      break;
  }
}

void Game::UpdateLives (bool generate = false)
{
  char text [9], temp [4];
  int color [3] = {0, 0, 64};
  TextObject textObject;
  if (lives < 1)
  {
    glClearColor (0, 0, 0.3f, 1);
    gameState = GameOver;
    BASS_ChannelPlay (soundeffect [1], TRUE);
    return;
  }
  strcpy (text, "Lives: ");
  strcat (text, itoa (lives, temp, 10));
  textObject.SetText (text);
  bitmap_lives.Clear (color);
  bitmap_lives.SetRenderTextFont ("Arial", 16);
  bitmap_lives.SetRenderTextColor (0xffff00);
  bitmap_lives.RenderText (&textObject, Align_Left);
  if (generate)
    bitmap_lives.GenerateTexture (texture_lives);
  else
    bitmap_lives.UpdateTexture (texture_lives);
}