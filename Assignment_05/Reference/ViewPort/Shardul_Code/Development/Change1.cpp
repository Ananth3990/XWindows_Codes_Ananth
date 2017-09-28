#include<windows.h> //For Win32 APIs
#include<stdio.h>   //For File IO
#include<gl/GL.h> 	//For OpenGL APIs
#include<gl/GLU.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

//For using the current window's handle across functions
HWND ghwnd;
RECT rt;

//C++ Boolean variables for respective functionality
bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;

//Global handles for Device context and Rendering context
HDC ghdc;
HGLRC ghrc;

//For fullscreen implementation:
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

//For Logfile
FILE *gpFile = NULL;

//For Viewport switching
int iKeyPressed;
GLfloat viewportHeight;
GLfloat viewportWidth;
GLfloat viewport_x = 0.0f;
GLfloat viewport_y = 0.0f;

//For delay additon before ViewPort switching
GLint delay_counter_for_viewport_switching = 0;

//CALLBACK function for your window!
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Entry point function:
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	//Function Prototypes
	void initialize(void);
	void display(void);
	void animate(void);

	//Variables
	WNDCLASSEX wndClass;
	TCHAR szAppName[] = TEXT("PIXEL FFP Demo");
	HWND hwnd;
	bool bDone = false;
	MSG msg;

	//Code-Start logging
	if (fopen_s(&gpFile, "FixedFunctionPipeline_Viewport.txt", "w") != 0) //fopen_s() will return 0 on success and non-zero on failure!
	{
		MessageBox(NULL, TEXT("Log file could not be created..exiting!!!"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file created successfully!!\n");
	}

	//code--Creation of Window Class with member initialization!
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Own-DC is non-purgeable DC

	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;

	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hInstance = hInstance;

	wndClass.lpszClassName = szAppName;
	wndClass.lpszMenuName = NULL;
	wndClass.lpfnWndProc = WndProc;

	//Code-Register your window class.
	RegisterClassEx(&wndClass);

	//code-Create the window in memory.
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,  // Window will be exclusive-fullscreen
		szAppName,     //Class name(Whose window is this?)
		TEXT("PIXEL: FFP Demo!"), //Title bar text.
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, //x
		0, //y
		WIN_WIDTH,  //width
		WIN_HEIGHT, //height
		NULL,		//Parent window handle
		NULL,  		// Window menu handle
		hInstance,  //
		NULL		// Any Creation parameters
	);

	ghwnd = hwnd;

	initialize(); //Initialize the pixel element

				  //Show the created window where second parameter will specify any parameters set through command line
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd); //Bring the window to the foreground
	SetFocus(hwnd);			   // Set focus on your window	

							   //Game loop				 
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
				//If-Window is active and escape key is not pressed then here we need to give a call to the render function
				animate();
				display();
			}
		}
	}//gameloop			

	if (gpFile) //gpFile is non-NULL
	{
		fprintf(gpFile, "Log file closed successfully!!");
		fclose(gpFile);
		gpFile = NULL;
	}

	return((int)msg.wParam);
}//WinMain()


 //CALLBACK Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Function declarations
	void ToggleFullscreen(void);
	void resize(int, int);

	//variable declaration
	int xWidth, yHeight;

	xWidth = GetSystemMetrics(SM_CXSCREEN);

	yHeight = GetSystemMetrics(SM_CYSCREEN);


	//code
	fprintf(gpFile, "=====In WndProc()========\n");

	switch (message)
	{
	case WM_CREATE:
		break;

	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;

		break;

	case WM_KEYDOWN:
		fprintf(gpFile, "Keydown:%x\n", wParam);
		switch (wParam)
		{
		case VK_ESCAPE: //Escape key pressed: VK_ESCAPE is also applicable
			fprintf(gpFile, "Under Case 'Escape'!\n");
			if (gbEscapeKeyIsPressed == false)
				gbEscapeKeyIsPressed = true;
			break;

		case 0x46: //'f' or 'F' for fullscreen
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else //window mode
			{
				ToggleFullscreen();
				gbFullscreen = false;

			}
			break;

		case 0x61: //For Numpad key '1'
		case 0x31: //For dual key '1'(Ex.Single key having '!' and '1')	 
			iKeyPressed = 1;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x62://
		case 0x32://2

			iKeyPressed = 2;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;
		case 0x63:
		case 0x33://3

			iKeyPressed = 3;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;
		case 0x64:
		case 0x34://4

			iKeyPressed = 4;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x65:
		case 0x35://5

			iKeyPressed = 5;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;
		case 0x66:
		case 0x36://6

			iKeyPressed = 6;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x67:
		case 0x37://7

			iKeyPressed = 7;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x68:
		case 0x38://8

			iKeyPressed = 8;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x69:
		case 0x39://9

			iKeyPressed = 9;
			if (gbFullscreen == true)
				resize(xWidth, yHeight);
			else
				resize(WIN_WIDTH, WIN_HEIGHT);
			break;

		case 0x51: //Q-Quit 
			PostQuitMessage(0);
			break;
		}
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	}//switch
	return(DefWindowProc(hwnd, message, wParam, lParam));
}//WndProc()

 /*
 Initialize(): Will prepare the pixel for OpenGL.
 1) Fill in the PIXELFORMATDESCRIPTOR Structure.
 2) Get the device context from Windows OS.
 3) From the "Device context" obtained, use it further till WGL comes into picture!
 4) Set the chosen pixel format
 5)
 */

void initialize(void)
{
	//Function declarations
	void resize(int, int);

	//Variables
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	//Code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;  //MS defined value
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	//Get the device context from current/underlying windows OS-->Device context.
	ghdc = GetDC(ghwnd);

	//From the device context obtained, "Choose the pixel format which is closest match to the values through which pfd has been filled in"
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormatIndex == 0) //ChoosePixelFormat() failed to match and choose the pixel format!
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == false) //SetPixelFormat() failed!
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;

	}
	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc, ghrc) == NULL)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	GetClientRect(ghwnd, &rt);
	resize(rt.right, rt.bottom);

}//initialize()



 /*
 "Windowed-->Fullscreen toggle"
 ------------------------------
 ## Get the current window style
 ## Remove WS_OVERLAPPED Window flag from the current window style
 ## Save the current window position and all its parameters
 ## Set the window style to "Fullscreen" and do not show the cursor in fullscreen mode

 Fullscreen-->Windowed toggle
 ------------------------------
 ## Add the removed "WS_OVERLAPPEDWINDOW" style to the current window style
 ## Set the window placement to the previous one where the window was before going into fullscreen
 ## Set the window position
 ## Show the Cursor

 */
void ToggleFullscreen(void)
{
	//variable

	MONITORINFO mi = { sizeof(MONITORINFO) };
	bool IsWindowPlacementReceived;
	bool IsMonitorInfo;
	HMONITOR hMonitor;

	//code
	if (gbFullscreen == false) //Window-->Fullscreen
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE); //Get the current window style into dwStyle which contains entire inforamtion about the current window

		if (dwStyle & WS_OVERLAPPEDWINDOW) //Current window style contains this style, remove it for switching to fullscreen
		{
			//a) Get the "Current Window Placement" 
			IsWindowPlacementReceived = GetWindowPlacement(ghwnd, &wpPrev);

			//b) Now get the monitor 
			hMonitor = MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY);

			//c) From the monitor information from thw handle and store it in the monitor info variable
			IsMonitorInfo = GetMonitorInfo(hMonitor, &mi);

			//Monitor information retrieved and previous placement saved, switch to fullscreen now
			if (IsWindowPlacementReceived && IsMonitorInfo)
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				//Set the window position

				SetWindowPos(
					ghwnd,
					HWND_TOP, //Top of all the windows
					mi.rcMonitor.left, //x-coord
					mi.rcMonitor.top,  //y-coord
					mi.rcMonitor.right - mi.rcMonitor.left, //Width
					mi.rcMonitor.bottom - mi.rcMonitor.top,  //height
					SWP_NOZORDER | SWP_FRAMECHANGED
				);
			}
			ShowCursor(FALSE);
		}

	}
	else //Fullscreen-->Windowed
	{
		//Add WS_OVERLAPPEDWIDNOW
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		//Set the window placement to the previous state
		SetWindowPlacement(ghwnd, &wpPrev);

		//Set the window Position
		SetWindowPos(
			ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOMOVE |
			SWP_NOSIZE |
			SWP_NOOWNERZORDER |
			SWP_NOZORDER |
			SWP_FRAMECHANGED


		);
		//Show thw cursor in windowed mode	
		ShowCursor(TRUE);

	}


}//ToggleFullscreen()

 /*
 Resize():
 glViewport(x,y,width,height);

 where x,y-->Starting point coordinates
 By default (x,y) pair for shopping is normal.
 */

void resize(int width, int height)
{
	if (height == 0)
		height = 1;

	switch (iKeyPressed)
	{

	case 1://Left Top Corner
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport(0, (GLfloat)height / 2, (GLsizei)width / 2, (GLsizei)height / 2);

		break;

	case 2://Right Top Corner
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);
		fprintf(gpFile, "viewportwidth=%f\nviewportheight=%f\n", viewportWidth, viewportHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//	 glViewport((GLfloat)0,(GLfloat)0,(GLfloat)width/2,(GLfloat)height/2);
		//	 gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		break;

	case 3://Left Bottom Corner
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport(0, 0, (GLsizei)width / 2, (GLsizei)height / 2);

		break;

	case 4://Right bottom Corner
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport((GLfloat)width / 2, 0, (GLsizei)width / 2, (GLsizei)height / 2);

		break;
	case 5://Left half of screen
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport(0, 0, (GLsizei)width / 2, height);

		break;
	case 6://Right half of the screen
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport((GLfloat)width / 2, 0, (GLsizei)width / 2, height);

		break;
	case 7://Top half of the screen
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport(0, (GLfloat)height / 2, width, (GLsizei)height / 2);

		break;
	case 8://Bottom half of the screen
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);

		glViewport(0, 0, width, (GLsizei)height / 2);

		break;
	case 9://Center
		fprintf(gpFile, "iKeyPressed=%d\n", iKeyPressed);
		glViewport((GLfloat)width / 4, 0, (GLsizei)width / 2, (GLsizei)height);
		break;

	default:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport((GLfloat)0, (GLfloat)0, (GLfloat)width, (GLfloat)height);
		gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	}


}//resize()

 /*
 Display():
 1) Actual Render function.

 */

 /*
 For Fragment Shader
 */
void ForFragmentShader(void)
{
	//Fragment Shader Code
	glTranslatef(0.0f, 0.0f, -5.0f);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Frgment shader
	ForFragmentShader();

	SwapBuffers(ghdc);
}//display()

void animate(void)
{
	void animateViewPort(void);
	delay_counter_for_viewport_switching++;
	if (delay_counter_for_viewport_switching >= 1000)
	{
		animateViewPort();
	}
}

void animateViewPort(void)
{
	GetClientRect(ghwnd, &rt);
	viewportHeight = rt.bottom;
	viewportWidth = rt.right;

	if (viewportHeight >= rt.bottom / 2)
	{
		viewportHeight -= 0.001f;
	}
	if (viewportWidth >= rt.right / 2)
	{
		viewportWidth -= 0.001f;
	}
	if (viewport_x <= rt.right / 2)
	{
		viewport_x += 0.001f;
	}
	if (viewport_y <= rt.bottom / 2)
	{
		viewport_y += 0.001f;
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, (GLfloat)viewportHeight / 2, (GLsizei)viewportWidth / 2, (GLsizei)viewportHeight / 2);
	gluPerspective(45.0f, (GLfloat)rt.right / (GLfloat)rt.bottom, 0.1f, 100.0f);
	fprintf(gpFile, "viewportwidth=%f\nviewportheight=%f\n", viewportWidth, viewportHeight);
}

