#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/gl.h>
#include<GL/glx.h>

using namespace std;

bool bFullScreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

GLXContext gGLXContext;

int main(void)
{
	void CreateWindow(void);
	void initialize(void);
	void ToggleFullScreen(void);
	void resize(int width,int height);
	void display(void);

	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;

	bool bDone=false;

	CreateWindow();

	initialize();

	XEvent event;
	KeySym keysym;

	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
						break;
				case KeyPress:

						keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);

						switch(keysym)
						{

							case XK_Escape:
									bDone=true;
									break;
		
							case XK_F:
							case XK_f:
								if(bFullScreen==false)
								{
									ToggleFullScreen();
									bFullScreen=true;
								}
								else
								{
									ToggleFullScreen();
									bFullScreen=false;
								}
							break;

							default:
								break;
						}						
						break;

				case ButtonPress:
						switch(event.xbutton.button)
						{
							case 1:
								break;

							case 2:
								break;

							case 3:
								break;

							default:
								break;
						}
						break;

				case MotionNotify:
						break;

				case ConfigureNotify:
						winWidth=event.xconfigure.width;
						winHeight=event.xconfigure.height;
						resize(winWidth,winHeight);
						break;
				case Expose:
						break;
				case DestroyNotify:
						break;
				case 33:
					bDone=true;
					break;

				default:
					break;
			}
		}
		display();
	}
	return 0;
}

void CreateWindow(void)
{
	void uninitialize(void);

	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	/*
	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE,1,
		GLX_GREEN_SIZE,1,
		GLX_BLUE_SIZE,1,
		GLX_ALPHA_SIZE,1,
		None
	};
	*/
	
	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DOUBLEBUFFER,True,
		GLX_DEPTH_SIZE,24,
		None
	};

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("Unable to open X Display\n");
		exit(1);
	}	

	defaultScreen=XDefaultScreen(gpDisplay);

	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),gpXVisualInfo->visual,AllocNone);
	gColormap=winAttribs.colormap;
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

	gWindow=XCreateWindow(
	gpDisplay,
	RootWindow(gpDisplay,gpXVisualInfo->screen),
	0,
	0,
	giWindowWidth,
	giWindowHeight,
	0,
	gpXVisualInfo->depth,
	InputOutput,
	gpXVisualInfo->visual,
	styleMask,
	&winAttribs
	);

	if(!gWindow)
	{
		printf("Failed to create Main Window\n");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay,gWindow,"First OpenGL Window");

	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);	
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	}

	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}

	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}

	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo==NULL;
	}

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}


void initialize(void)
{
	void resize(int,int);

	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glClearColor(0.0f,0.0f,1.0f,0.0f);

	//Added for Double buffer
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);

	
	resize(giWindowWidth,giWindowHeight);
}

void resize(int width,int height)
{
	if(height==0)
	{
		height=1;
	}
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
}

void ToggleFullScreen(void)
{
	Atom wm_state;
	Atom fullscreen;

	XEvent xev={0};

	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullScreen?0:1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(
	gpDisplay,
	RootWindow(gpDisplay,gpXVisualInfo->screen),
	False,
	StructureNotifyMask,
	&xev
	);
	
}

void display(void)
{
	//Changed for DoubleBuffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//Changed for Double Buffer	
	//glFlush();
	glXSwapBuffers(gpDisplay,gWindow);
}
