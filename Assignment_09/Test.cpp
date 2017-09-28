#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/gl.h>
#include<GL/glu.h>
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

GLfloat angleTri = 0.0f;
GLfloat angleSquare = 360.0f;

FILE *gpFile=NULL;

//Solar System
static GLint shoulder = 0;
static GLint elbow = 0;
GLUquadric *quadric = NULL;

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
	void update(void);

	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;

	bool bDone=false;

	if(remove("./Log.txt"))
	{

	}
	else
	{

	}

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

							case XK_1:
							elbow = (elbow + 6) % 360;
							break;
							
							case XK_2:
							elbow = (elbow-6) % 360;
							break;

							case XK_3:
							shoulder = (shoulder+3) % 360;
							break;

							case XK_4:
							shoulder = (shoulder-3) % 360;
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
	doFileIO("Leaving main()\n");
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

	doFileIO("Leaving Createwindow()\n");
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

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}

	if(gpFile)
	{
		fclose(gpFile);
		gpFile=NULL;
	}	

	doFileIO("Leaving uninitialize()\n");
}


void initialize(void)
{
	void resize(int,int);
	
	doFileIO("Inside initialize()\n");

	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	//Added for Double buffer
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);

	
	resize(giWindowWidth,giWindowHeight);


	doFileIO("Leaving uninitialize()\n");
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
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	doFileIO("Leaving resize()\n");
}

void ToggleFullScreen(void)
{
	doFileIO("Inside ToggleFullScreen()\n");

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

	doFileIO("Outside ToggleFullScreen()\n");
	
}

/*
void display(void)
{
	//Changed for DoubleBuffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//Changed for Double Buffer	
	//glFlush();
	glXSwapBuffers(gpDisplay,gWindow);
}
*/

void display(void)
{	
	doFileIO("Inside display()\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glTranslatef(0.0f, 0.0f, -12.0f);

		glPushMatrix();

	glRotatef((GLfloat)shoulder,0.0f,0.0f,1.0f);
	glTranslatef(1.0f,0.0f,0.0f);
			glPushMatrix();
	glScalef(2.0f,0.5f,1.0f);
	quadric = gluNewQuadric();
	glColor3f(0.5f,0.35f,0.05f);
	gluSphere(quadric,0.5f,10,10);

			glPopMatrix();

	glTranslatef(1.0f,0.0f,0.0f);
	glRotatef((GLfloat)elbow,0.0f,0.0f,1.0f);
	glTranslatef(1.0f, 0.0f, 0.0f);
				glPushMatrix();
	glScalef(2.0f, 0.5f, 1.0f);
	quadric = gluNewQuadric();
	glColor3f(0.5f, 0.35f, 0.05f);
	gluSphere(quadric, 0.5f, 10, 10);
				glPopMatrix();
		glPopMatrix();

	glXSwapBuffers(gpDisplay,gWindow);

	doFileIO("Laeving Display()\n");
}
