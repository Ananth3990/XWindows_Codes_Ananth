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

FILE *gpFile=NULL;

void doFileIO(const char *msg)
{
	char *temp=NULL;
	temp=(char*)malloc(1000*sizeof(char));

	strcpy(temp,"AC :");
	strcat(temp," ");
	strcat(temp,msg);

	gpFile=fopen("./Log.txt","a");
	if(gpFile==NULL)
	{
		printf("Could not open File\n");
		exit(1);
	}

	fprintf(gpFile,"%s",temp);

	fclose(gpFile);
	gpFile=NULL;

	free(temp);
	temp=NULL;
}

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

	doFileIO("Inside main()\n");

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

	doFileIO("Outside main()\n");
	return 0;
}

void CreateWindow(void)
{
	void uninitialize(void);

	doFileIO("Inside CreateWindow()\n");

	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE,1,
		GLX_GREEN_SIZE,1,
		GLX_BLUE_SIZE,1,
		GLX_ALPHA_SIZE,1,
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

	doFileIO("Outside CreateWindow()\n");
}

void uninitialize(void)
{
	GLXContext currentGLXContext;

	doFileIO("Inside uninitialize()\n");

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

	if(gpFile)
	{
		fclose(gpFile);
		gpFile=NULL;
	}

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}

	doFileIO("Outside uninitialize()\n");
}


void initialize(void)
{
	void resize(int,int);

	doFileIO("Inside initialize()\n");

	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glClearColor(0.5f,0.5f,0.5f,0.0f);
	resize(giWindowWidth,giWindowHeight);

	doFileIO("Outside initialize()\n");
}

void resize(int width,int height)
{
	doFileIO("Inside resize()\n");

	if(height==0)
	{
		height=1;
	}
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-50.0f,50.0f,-50.0f,50.0f,-50.0f,50.0f);

	doFileIO("Outside resize()\n");
}

void ToggleFullScreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};

	doFileIO("Inside ToggleFullScreen()\n");

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

	doFileIO("Outside ToggleFullScreen()\n");
}

void RenderTriangleMultiColoredVertex(GLfloat vert[][3], GLfloat color[][3])
{
	doFileIO("Rendering triangle\n");	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBegin(GL_TRIANGLES);
	glColor3f(color[0][0], color[0][1], color[0][2]);
	glVertex3f(vert[0][0], vert[0][1], vert[0][2]);
	glColor3f(color[1][0], color[1][1], color[1][2]);
	glVertex3f(vert[1][0], vert[1][1], vert[1][2]);
	glColor3f(color[2][0], color[2][1], color[2][2]);
	glVertex3f(vert[2][0], vert[2][1], vert[2][2]);
	glEnd();

	doFileIO("Rendered triangle\n");
}

void display(void)
{
	void RenderTriangleMultiColoredVertex(GLfloat vert[][3], GLfloat color[][3]);

	doFileIO("Inside display()\n");

	typedef struct
	{
		GLfloat triangle_vert[3][3];
	}TRIANGLES;

	typedef struct
	{
		GLfloat color[3][3];
	}COLOR;

	TRIANGLES triangles;
	COLOR color;

	glClear(GL_COLOR_BUFFER_BIT);

	triangles = { { { -50.0f, -50.0f, 0.0f } ,{ 50.0f, -50.0f, 0.0f } ,{ 0.0f, 50.0f, 0.0f } } };
	color = { { { 1.0f, 0.0f, 0.0f } ,{ 0.0f, 1.0f, 0.0f } ,{ 0.0f, 0.0f, 1.0f } } };
	RenderTriangleMultiColoredVertex(triangles.triangle_vert, color.color);

	glFlush();

	doFileIO("Outside display()\n");
}

