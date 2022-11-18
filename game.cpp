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
  for (i = 0; i < 20; i++)
    bitmap [i].GenerateTexture (texture [i]);

  BASS_Init (-1, 44100, 0, NULL, NULL);
  soundeffect [0] = BASS_StreamCreateFile (FALSE, "enemy_shoot.wav", 0, 0, 0);
  soundeffect [1] = BASS_StreamCreateFile (FALSE, "gameover.wav", 0, 0, 0);
  soundeffect [2] = BASS_StreamCreateFile (FALSE, "hit.wav", 0, 0, 0);
  soundeffect [3] = BASS_StreamCreateFile (FALSE, "jump.wav", 0, 0, 0);
  soundeffect [4] = BASS_StreamCreateFile (FALSE, "loselife.wav", 0, 0, 0);
  soundeffect [5] = BASS_StreamCreateFile (FALSE, "shoot.wav", 0, 0, 0);

  srand (tickCount);
  spacePressed = false;
  gameState = TitlePicture;

  return true;
}

void Game::NewGame (DWORD tickCount)
{
  NewGame_Sub (tickCount);
  glClearColor (0, 0, 0.3f, 1);
  playerPosition.x = 20;
  playerPosition.y = 200;
  lives = 5;
  UpdateLives (true);
  gameState = ActualGame;
}

void Game::EnterDinosaur (DWORD tickCount)
{
  int i;

  NewGame_Sub (tickCount);
  glClearColor (0, 0, 0.6f, 1);
  for (i = 0; i < 10; i++)
  {
    platformPosition [i].x = i * 64;
    platformPosition [i].y = 0;
    platformActive [i] = true;
  }
  for (; i < 15; i++)
    platformActive [i] = false;
  playerPosition.x = 20;
  playerPosition.y = 16;
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
  gameState = ActualGame_Dinosaur;
}

void Game::NewGame_Sub (DWORD tickCount)
{
  int i;

  for (i = 0; i < 10; i++)
    bulletActive [i] = false;
  for (i = 0; i < 20; i++)
    enemyActive [i] = false;
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
        if (playerPosition.y > 70)
          playerPosition.y -= 3;
      }
    }
    else if (g_keys->keyDown [VK_UP])
    {
      if (!keyPressedUpDown || keyPressedUpDownSince + 10 < tickCount)
      {
        keyPressedUpDown = true;
        keyPressedUpDownSince = tickCount;
        if (playerPosition.y < 416)
          playerPosition.y += 3;
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
        if (playerPosition.x > 2)
          playerPosition.x -= 3;
      }
    }
    else if (g_keys->keyDown [VK_RIGHT])
    {
      if (!keyPressedLeftRight || keyPressedLeftRightSince + 10 < tickCount)
      {
        keyPressedLeftRight = true;
        keyPressedLeftRightSince = tickCount;
        if (playerPosition.x < 400)
          playerPosition.x += 3;
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
        for (i = 0; i < 10 && bulletActive [i]; i++);
        if (i < 10)
        {
          bulletPosition [i].x = playerPosition.x + 64;
          bulletPosition [i].y = playerPosition.y;
          bulletActive [i] = true;
          BASS_ChannelPlay (soundeffect [5], TRUE);
        }
      }
    }
    else
      spacePressed = false;

    if (lastBulletMove + 5 < tickCount)
    {
      lastBulletMove = tickCount;
      for (i = 0; i < 10; i++)
        if (bulletActive [i])
        {
          bulletPosition [i].x += 5;
          if (bulletPosition [i].x >= 640)
            bulletActive [i] = false;
        }

      for (i = 0; i < 20; i++)
        if (enemyActive [i])
        {
          if (playerPosition.x + 64 >= enemy [i].position.x
            && playerPosition.x <= enemy [i].position.x + enemy [i].width
            && playerPosition.y + 32 >= enemy [i].position.y
            && playerPosition.y <= enemy [i].position.y + enemy [i].height)
          {
            lives--;
            UpdateLives (false);
            enemyActive [i] = false;
            BASS_ChannelPlay (soundeffect [4], TRUE);
          }

          for (j = 0; j < 10 && enemyActive [i]; j++)
              if (bulletActive [j]
                && bulletPosition [j].x + 64 >= enemy [i].position.x
                && bulletPosition [j].x <= enemy [i].position.x + enemy [i].width
                && bulletPosition [j].y + 32 >= enemy [i].position.y
                && bulletPosition [j].y <= enemy [i].position.y + enemy [i].height)
              {
                bulletActive [j] = false;

                if (enemy [i].type == Jellyfish || enemy [i].type == Kraken 
                  || enemy [i].type == EnemyBullet || enemy [i].type == CancerBullet)
                {
                  BASS_ChannelPlay (soundeffect [2], TRUE);
                  enemyActive [i] = false;
                }
              }

          if (enemyActive [i])
          {
            enemy [i].position.x -= 3;
            if (enemy [i].position.x < -enemy [i].width)
              enemyActive [i] = false;
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
                    for (j = 0; j < 20 && enemyActive [j]; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy [j].type = EnemyBullet;
                      enemy [j].width = 64;
                      enemy [j].height = 32;
                      enemy [j].position.x = enemy [i].position.x - 64 - 1;
                      enemy [j].position.y = enemy [i].position.y + 48;
                      enemyActive [j] = true;
                    }
                  }
                }
                break;

              case EnemyBullet:
                enemy [i].position.x -= 5;
                for (j = 0; j < 20 && enemyActive [i]; j++)
                  if (enemyActive [j] && j != i
                    && enemy [i].position.x + enemy [i].width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].width
                    && enemy [i].position.y + enemy [i].height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].height)
                    enemyActive [i] = false;
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
                    if (enemy [i].whereToGo.y < enemy [i].height)
                      enemy [i].whereToGo.y = enemy [i].height;
                    enemy [i].variable [0] = 1;
                  }
                  else
                  {
                    enemy [i].whereToGo.y = enemy [i].position.y + 100 - rand () % 200;
                    if (enemy [i].whereToGo.y > 480 - enemy [i].height)
                      enemy [i].whereToGo.y = 480 - enemy [i].height;
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
                    for (j = 0; j < 20 && enemyActive [j]; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy [j].type = CancerBullet;
                      enemy [j].width = 32;
                      enemy [j].height = 32;
                      enemy [j].position.x = enemy [i].position.x + 48;
                      enemy [j].position.y = enemy [i].position.y + 64 + 1;
                      enemy [j].whereToGo.x = 1 + rand () % 3;
                      if (rand () % 2)
                        enemy [j].whereToGo.x = -enemy [j].whereToGo.x;
                      enemyActive [j] = true;
                    }
                  }
                }
                break;

              case CancerBullet:
                enemy [i].position.x -= enemy [i].whereToGo.x;
                enemy [i].position.y += 5;
                for (j = 0; j < 20 && enemyActive [i]; j++)
                  if (enemyActive [j] && j != i
                    && enemy [i].position.x + enemy [i].width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].width
                    && enemy [i].position.y + enemy [i].height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].height)
                    enemyActive [i] = false;
                break;

              }
            }
          }
        }
    }

    if ((lastEnemyAppeared + 3000 < tickCount)
      || (lastEnemyAppeared + 750 - (tickCount - startTime) / 1000 < tickCount && rand () % 2))
    {
      for (i = 0; i < 20 && enemyActive [i]; i++);
      if (i < 20)
      {
        for (j = 0; j < 10; j++)
          enemy [i].variable [j] = 0;
        switch (rand () % 10)
        {
        case 0:
        case 1:
          enemy [i].type = Kraken;
          enemy [i].width = 128;
          enemy [i].height = 128;
          enemy [i].position.y = 128 + rand () % (480 - 128 * 2);
          break;

        case 2:
        case 3:
        case 4:
          enemy [i].type = Jellyfish;
          enemy [i].width = 64;
          enemy [i].height = 128;
          enemy [i].position.y = 128 + rand () % (480 - 128 * 2);
          enemy [i].variable [0] = rand () % 2;
          break;

        case 5:
          enemy [i].type = Cancer;
          enemy [i].width = 128;
          enemy [i].height = 64;
          enemy [i].position.y = 0;
          break;

        default:
          enemy [i].type = Seastar;
          enemy [i].width = 64;
          enemy [i].height = 64;
          enemy [i].position.y = 64 + rand () % (480 - 64 * 2);
          break;
        }
        enemy [i].moving = false;
        enemy [i].position.x = 640;
        enemyActive [i] = true;
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
        playerPosition.y += jumping;
        jumping--;
        if (!jumping)
          falling = 1;
      }
      else
      {
        keyPressedUpDown = true;

        if (!falling)
          falling = 1;

        for (i = 0; i < 15 && falling; i++)
          if (platformActive [i] 
            && platformPosition [i].x <= playerPosition.x + 64
            && platformPosition [i].x + 64 >= playerPosition.x + 10
            && platformPosition [i].y <= playerPosition.y + 80
            && platformPosition [i].y + 16 >= playerPosition.y)
            {
              playerPosition.y = platformPosition [i].y + 16;
              falling = 0;
              keyPressedUpDown = false;
            }

        if (falling)
        {
          if (playerPosition.y < -128)
          {
            int leftmost = 0;
            lives--;
            UpdateLives (false);
            BASS_ChannelPlay (soundeffect [4], TRUE);
            for (i = 0; i < 15 && falling; i++)
            {
              if (platformPosition [i].x > 0
                && (platformPosition [i].x < platformPosition [leftmost].x
                  || platformPosition [leftmost].x < 0))
                leftmost = i;
              if (platformActive [i] 
                && platformPosition [i].x <= playerPosition.x + 64
                && platformPosition [i].x + 64 >= playerPosition.x + 10)
                {
                  playerPosition.y = platformPosition [i].y + 16;
                  falling = 0;
                }
            }
            if (falling)
            {
              playerPosition.x = platformPosition [leftmost].x;
              playerPosition.y = platformPosition [leftmost].y + 16;
            }
          }
          playerPosition.y -= falling;
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
        if (playerPosition.x > 2)
          playerPosition.x -= 3;
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
        if (playerPosition.x < 320)
          playerPosition.x += 3;
        else
        {
          scrolling = true;
          j = 15;
          for (i = 0; i < 15; i++)
          {
            if (platformActive [i])
            {
              platformPosition [i].x -= 3;
              if (platformPosition [i].x < -64)
              {
                platformActive [i] = false;
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
              platformPosition [j].x = platformPosition [rightmost].x + 64;
              if (gap)
                platformPosition [j].x += 100;
              platformPosition [j].y = platformPosition [rightmost].y;
              upward++;
              if (!(rand () % 4))
              {
                int temp = rand () % 120 + 100;
                if (rand () % 2)
                  temp = -temp;
                while (platformPosition [j].y + temp < 0 || platformPosition [j].y + temp > 360)
                {
                  if (platformPosition [j].y - temp < 0 || platformPosition [j].y - temp > 360)
                  {
                    temp /= 4;
                    temp *= 3;
                  }
                  else
                    temp = -temp;
                }
                if (temp < 0)
                  upward = 0;
                platformPosition [j].y += temp;
              }
              platformActive [j] = true;
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
        for (i = 0; i < 10 && bulletActive [i]; i++);
        if (i < 10)
        {
          if (movingDirection)
            bulletPosition [i].x = playerPosition.x + 64;
          else
            bulletPosition [i].x = playerPosition.x - 32;
          bulletPosition [i].y = playerPosition.y + 60;
          bulletDirection [i] = movingDirection;
          bulletActive [i] = true;
          BASS_ChannelPlay (soundeffect [5], TRUE);
        }
      }
    }
    else
      spacePressed = false;

    if (lastBulletMove + 5 < tickCount)
    {
      lastBulletMove = tickCount;
      for (i = 0; i < 10; i++)
        if (bulletActive [i])
        {
          if (bulletDirection [i])
            bulletPosition [i].x += 5;
          else
            bulletPosition [i].x -= 5;
          bulletPosition [i].y--;
          if (bulletPosition [i].x >= 640 || bulletPosition [i].x < 0 || bulletPosition [i].y < 0)
            bulletActive [i] = false;
        }

      for (i = 0; i < 20; i++)
        if (enemyActive [i])
        {
          if (playerPosition.x + 64 >= enemy [i].position.x
            && playerPosition.x <= enemy [i].position.x + enemy [i].width
            && playerPosition.y + 80 >= enemy [i].position.y
            && playerPosition.y <= enemy [i].position.y + enemy [i].height)
          {
            lives--;
            UpdateLives (false);
            enemyActive [i] = false;
            BASS_ChannelPlay (soundeffect [4], TRUE);
          }
          for (j = 0; j < 10 && enemyActive [i]; j++)
              if (bulletActive [j]
                && bulletPosition [j].x + 32 >= enemy [i].position.x
                && bulletPosition [j].x <= enemy [i].position.x + enemy [i].width
                && bulletPosition [j].y + 32 >= enemy [i].position.y
                && bulletPosition [j].y <= enemy [i].position.y + enemy [i].height)
              {
                BASS_ChannelPlay (soundeffect [2], TRUE);
                bulletActive [j] = false;
                enemyActive [i] = false;
              }

          if (enemyActive [i])
          {
            if (scrolling)
              enemy [i].position.x -= 3;
            if (enemy [i].position.x < -enemy [i].width)
              enemyActive [i] = false;
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
                    for (j = 0; j < 20 && enemyActive [j]; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy [j].type = EnemyBullet;
                      enemy [j].width = 32;
                      enemy [j].height = 32;
                      enemy [j].position.x = enemy [i].position.x - 32;
                      enemy [j].position.y = enemy [i].position.y + 80;
                      enemy [j].whereToGo.y = 0;
                      enemyActive [j] = true;
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
                    for (j = 0; j < 20 && enemyActive [j]; j++);
                    if (j < 20)
                    {
                      BASS_ChannelPlay (soundeffect [0], TRUE);
                      enemy [j].type = EnemyBullet;
                      enemy [j].width = 32;
                      enemy [j].height = 32;
                      enemy [j].position.x = enemy [i].position.x - 32;
                      enemy [j].position.y = enemy [i].position.y + 80;
                      enemy [j].whereToGo.y = 1 + rand () % 3;
                      if (rand () % 2)
                        enemy [j].whereToGo.y = -enemy [j].whereToGo.y;
                      enemyActive [j] = true;
                    }
                  }
                }
                break;

              case EnemyBullet:
                enemy [i].position.x -= 5;
                enemy [i].position.y += enemy [i].whereToGo.y;
                for (j = 0; j < 20 && enemyActive [i]; j++)
                  if (enemyActive [j] && j != i
                    && enemy [i].position.x + enemy [i].width >= enemy [j].position.x
                    && enemy [i].position.x <= enemy [j].position.x + enemy [j].width
                    && enemy [i].position.y + enemy [i].height >= enemy [j].position.y
                    && enemy [i].position.y <= enemy [j].position.y + enemy [j].height)
                    enemyActive [i] = false;
                break;
              }
            }
          }
        }

        for (i = 0; i < 15; i++)
          if (platformActive [i])
            for (j = 0; j < 10; j++)
              if (bulletActive [j]
                && bulletPosition [j].x + 32 >= platformPosition [i].x
                && bulletPosition [j].x <= platformPosition [i].x + 64
                && bulletPosition [j].y + 32 >= platformPosition [i].y
                && bulletPosition [j].y <= platformPosition [i].y + 16)
                bulletActive [j] = false;
    }

    if (upward > 2
      && lastEnemyAppeared + 2 < completed
      && ((lastEnemyAppeared + 20 < completed)
      || (lastEnemyAppeared + 5 - completed / 100 < completed && rand () % 2)))
    {
      for (i = 0; i < 20 && enemyActive [i]; i++);
      if (i < 20)
      {
        for (j = 0; j < 10; j++)
          enemy [i].variable [j] = 0;
        switch (rand () % 10)
        {
        case 0:
        case 1:
        case 2:
          enemy [i].type = Lion;
          enemy [i].width = 128;
          enemy [i].height = 118;
          enemy [i].position.x = platformPosition [rightmost].x;
          enemy [i].position.y = platformPosition [rightmost].y + 16;
          break;

        default:
          enemy [i].type = Worm;
          enemy [i].width = 64;
          enemy [i].height = 98;
          enemy [i].position.x = platformPosition [rightmost].x;
          enemy [i].position.y = platformPosition [rightmost].y + 16;
          break;
        }
        enemy [i].moving = false;
        enemyActive [i] = true;
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
    glBindTexture (GL_TEXTURE_2D, texture [0]);
    glColor3f (1.0f, 1.0f, 1.0f);
    glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f); glVertex2f (playerPosition.x, playerPosition.y);
      glTexCoord2f (1.0f, 0.0f); glVertex2f (playerPosition.x + 64, playerPosition.y);
      glTexCoord2f (1.0f, 1.0f); glVertex2f (playerPosition.x + 64, playerPosition.y + 32);
      glTexCoord2f (0.0f, 1.0f); glVertex2f (playerPosition.x, playerPosition.y + 32);
    glEnd ();
    glDisable (GL_TEXTURE_2D);

    for (i = 0; i < 10; i++)
      if (bulletActive [i])
      {
        glEnable (GL_TEXTURE_2D);
        glBindTexture (GL_TEXTURE_2D, texture [1]);
        glColor3f (1.0f, 1.0f, 1.0f);
        glBegin (GL_QUADS);
          glTexCoord2f (0.0f, 0.0f); glVertex2f (bulletPosition [i].x, bulletPosition [i].y);
          glTexCoord2f (1.0f, 0.0f); glVertex2f (bulletPosition [i].x + 64, bulletPosition [i].y);
          glTexCoord2f (1.0f, 1.0f); glVertex2f (bulletPosition [i].x + 64, bulletPosition [i].y + 32);
          glTexCoord2f (0.0f, 1.0f); glVertex2f (bulletPosition [i].x, bulletPosition [i].y + 32);
        glEnd ();
        glDisable (GL_TEXTURE_2D);
      }

    for (i = 0; i < 20; i++)
      if (enemyActive [i])
      {
        glEnable (GL_TEXTURE_2D);
        switch (enemy [i].type)
        {
        case Kraken:
          glBindTexture (GL_TEXTURE_2D, texture [6]);
          break;

        case Cancer:
          glBindTexture (GL_TEXTURE_2D, texture [3]);
          break;

        case Seastar:
          glBindTexture (GL_TEXTURE_2D, texture [7]);
          break;

        case Jellyfish:
          glBindTexture (GL_TEXTURE_2D, texture [5]);
          break;

        case EnemyBullet:
          glBindTexture (GL_TEXTURE_2D, texture [2]);
          break;

        case CancerBullet:
          glBindTexture (GL_TEXTURE_2D, texture [4]);
          break;
        }
        glColor3f (1.0f, 1.0f, 1.0f);
        switch (enemy [i].type)
        {
        case Kraken:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y 
            + 128);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 128);
          glEnd ();
          break;

        case Jellyfish:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y 
            + 128);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 128);
          glEnd ();
          break;

        case Seastar:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y + 64);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 64);
          glEnd ();
          break;

        case Cancer:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y 
            + 64);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 64);
          glEnd ();
          break;

        case EnemyBullet:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y + 32);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 32);
          glEnd ();
          break;

        case CancerBullet:
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 32, enemy [i].position.y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 32, enemy [i].position.y + 32);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 32);
          glEnd ();
          break;
        }
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
        glBindTexture (GL_TEXTURE_2D, texture [10]);
      else
        glBindTexture (GL_TEXTURE_2D, texture [9]);
      glColor3f (1.0f, 1.0f, 1.0f);
      glBegin (GL_QUADS);
        if (movingDirection)
        {
          glTexCoord2f (0.0f, 0.0f); glVertex2f (playerPosition.x, playerPosition.y);
          glTexCoord2f (1.0f, 0.0f); glVertex2f (playerPosition.x + 64, playerPosition.y);
          glTexCoord2f (1.0f, 1.0f); glVertex2f (playerPosition.x + 64, playerPosition.y + 128);
          glTexCoord2f (0.0f, 1.0f); glVertex2f (playerPosition.x, playerPosition.y + 128);
        }
        else
        {
          glTexCoord2f (0.0f, 0.0f); glVertex2f (playerPosition.x + 64, playerPosition.y);
          glTexCoord2f (1.0f, 0.0f); glVertex2f (playerPosition.x, playerPosition.y);
          glTexCoord2f (1.0f, 1.0f); glVertex2f (playerPosition.x, playerPosition.y + 128);
          glTexCoord2f (0.0f, 1.0f); glVertex2f (playerPosition.x + 64, playerPosition.y + 128);
        }
      glEnd ();
      glDisable (GL_TEXTURE_2D);

      for (i = 0; i < 15; i++)
        if (platformActive [i])
        {
          glEnable (GL_TEXTURE_2D);
          glBindTexture (GL_TEXTURE_2D, texture [8]);
          glColor3f (1.0f, 1.0f, 1.0f);
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (platformPosition [i].x, platformPosition [i].y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (platformPosition [i].x + 64, platformPosition [i].y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (platformPosition [i].x + 64, platformPosition [i].y 
            + 16);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (platformPosition [i].x, platformPosition [i].y + 16);
          glEnd ();
          glDisable (GL_TEXTURE_2D);
        }

      for (i = 0; i < 10; i++)
        if (bulletActive [i])
        {
          glEnable (GL_TEXTURE_2D);
          glBindTexture (GL_TEXTURE_2D, texture [11]);
          glColor3f (1.0f, 1.0f, 1.0f);
          glBegin (GL_QUADS);
            glTexCoord2f (0.0f, 0.0f); glVertex2f (bulletPosition [i].x, bulletPosition [i].y);
            glTexCoord2f (1.0f, 0.0f); glVertex2f (bulletPosition [i].x + 32, bulletPosition [i].y);
            glTexCoord2f (1.0f, 1.0f); glVertex2f (bulletPosition [i].x + 32, bulletPosition [i].y + 32);
            glTexCoord2f (0.0f, 1.0f); glVertex2f (bulletPosition [i].x, bulletPosition [i].y + 32);
          glEnd ();
          glDisable (GL_TEXTURE_2D);
        }

      for (i = 0; i < 20; i++)
        if (enemyActive [i])
        {
          glEnable (GL_TEXTURE_2D);
          switch (enemy [i].type)
          {
          case Lion:
            glBindTexture (GL_TEXTURE_2D, texture [13]);
            break;

          case Worm:
            glBindTexture (GL_TEXTURE_2D, texture [14]);
            break;

          case EnemyBullet:
            glBindTexture (GL_TEXTURE_2D, texture [12]);
            break;
          }
          glColor3f (1.0f, 1.0f, 1.0f);
          switch (enemy [i].type)
          {
          case Lion:
            glBegin (GL_QUADS);
              glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
              glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y);
              glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 128, enemy [i].position.y 
              + 128);
              glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 128);
            glEnd ();
            break;

          case Worm:
            glBegin (GL_QUADS);
              glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
              glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y);
              glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 64, enemy [i].position.y 
              + 128);
              glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 128);
            glEnd ();
            break;

          case EnemyBullet:
            glBegin (GL_QUADS);
              glTexCoord2f (0.0f, 0.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y);
              glTexCoord2f (1.0f, 0.0f); glVertex2f (enemy [i].position.x + 32, enemy [i].position.y);
              glTexCoord2f (1.0f, 1.0f); glVertex2f (enemy [i].position.x + 32, enemy [i].position.y 
              + 32);
              glTexCoord2f (0.0f, 1.0f); glVertex2f (enemy [i].position.x, enemy [i].position.y + 32);
            glEnd ();
            break;
          }
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