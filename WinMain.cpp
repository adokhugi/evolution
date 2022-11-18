#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "game.h"

Game intro;

bool g_isProgramLooping;
bool g_createFullScreen;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD		tickCount;

	GL_Window* window = (GL_Window*)(GetWindowLong (hWnd, GWL_USERDATA));

	switch (uMsg)
	{
		case WM_ERASEBKGND:
			return 0;

		case WM_PAINT:
		{
			tickCount = GetTickCount ();
			intro.Update (tickCount, window->lastTickCount);
			window->lastTickCount = tickCount;
			intro.Draw (tickCount);
			SwapBuffers (window->hDC);
		}
		return 0;

		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				return 0;
			}
			break;
		}
		return 0;

		case WM_CREATE:
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, (LONG)(window));
		}
		return 0;

		case WM_CLOSE:
			TerminateApplication(window);
		return 0;

		case WM_SIZE:
			switch (wParam)
			{
				case SIZE_MINIMIZED:
					window->isVisible = FALSE;
				return 0;

				case SIZE_MAXIMIZED:
					window->isVisible = TRUE;
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));
				return 0;

				case SIZE_RESTORED:
					window->isVisible = TRUE;
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));
				return 0;
			}
		break;

		case WM_KEYDOWN:
			if ((wParam >= 0) && (wParam <= 255))
			{
				window->keys->keyDown [wParam] = TRUE;
				return 0;
			}
		break;

		case WM_KEYUP:
			if ((wParam >= 0) && (wParam <= 255))
			{
				window->keys->keyDown [wParam] = FALSE;
				return 0;
			}
		break;

		case WM_TOGGLEFULLSCREEN:
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;
	GL_Window			window;
	Keys				keys;
	BOOL				isMessagePumpActive;
	MSG				msg;

	application.className = "OpenGL";
	application.hInstance = hInstance;

	ZeroMemory (&window, sizeof (GL_Window));
	window.keys			= &keys;
	window.init.application		= &application;

	window.init.title		= "Evolution - A Game by Adok (Claus Volko)";

	window.init.width		= 640;
	window.init.height		= 480;
	window.init.bitsPerPixel	= 32;
	window.init.isFullScreen	= TRUE;

	ZeroMemory (&keys, sizeof (Keys));

	if (RegisterWindowClass (&application, WindowProc) == FALSE)
	{
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", 
		MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}

	g_isProgramLooping = TRUE;
	g_createFullScreen = (bool) window.init.isFullScreen;
	while (g_isProgramLooping)
	{
		window.init.isFullScreen = (BOOL) g_createFullScreen;
		if (CreateWindowGL (&window) == TRUE)
		{
			if (intro.Initialize (&window, &keys, GetTickCount ()) == false)
				TerminateApplication (&window);
			else
			{
				isMessagePumpActive = TRUE;
				while (isMessagePumpActive == TRUE)
				{
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						if (msg.message != WM_QUIT)
						{
							DispatchMessage (&msg);
						}
						else
						{
							isMessagePumpActive = FALSE;
						}
					}
					else
					{
						if (window.isVisible == FALSE)
						{
							WaitMessage ();
						}
					}
				}
			}

			intro.Deinitialize ();

			DestroyWindowGL (&window);
		}
		else
		{
			MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", 
			MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;
		}
	}

	UnregisterClass (application.className, application.hInstance);
	return 0;
}