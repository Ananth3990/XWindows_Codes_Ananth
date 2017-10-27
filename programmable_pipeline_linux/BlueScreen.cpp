#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <memory.h> 

#include <X11/Xlib.h> 
#include <X11/Xutil.h> 
#include <X11/XKBlib.h> 
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h> 

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

FILE *gpFile = NULL;

Display *gpDisplay=NULL;

XVisualInfo *gpXVisualInfo=NULL;

Colormap gColormap;

Window gWindow;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;

GLXFBConfig gGLXFBConfig;

GLXContext gGLXContext;

bool gbFullscreen = false;

void CreateWindow(void);
void ToggleFullscreen(void);
void initialize(void);
void resize(int,int);
void display(void);
void uninitialize(void);
	
int main(int argc, char *argv[])
{
	gpFile=fopen("Log.txt", "w");
	if (gpFile==NULL)
	{
		printf("Log File Can Not Be Created.\n");
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Is Successfully Opened.\n");
	}
	
	CreateWindow();
	
	initialize();

	XEvent event;
	KeySym keySym;
	int winWidth;
	int winHeight;
	bool bDone=false;
	
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
					keySym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
					switch(keySym)
					{

						case XK_Escape:
							bDone=true;
							break;

						case XK_F:
						case XK_f:
							if(gbFullscreen==false)
							{
								ToggleFullscreen();
								gbFullscreen=true;
							}
							else
							{
								ToggleFullscreen();
								gbFullscreen=false;
							}
							break;

						default:
							break;
					}
					break;

				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1: //Left MouseButton
							break;
						case 2: //Middle Mouse Button
							break;
						case 3: //Right Mouse Button
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
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	XSetWindowAttributes winAttribs;
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumFBConfigs=0;
	int styleMask;
	int i;

	int sampleBuffer,samples;
	
	static int frameBufferAttributes[]=
		{
		GLX_X_RENDERABLE,True,

		GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,

		GLX_RENDER_TYPE,GLX_RGBA_BIT,

		GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,

		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE,8,

		GLX_DOUBLEBUFFER,True,

		None};
	
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("XOpenDisplay() failed\n");
		uninitialize();
		exit(1);
	}
	
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
	if(pGLXFBConfigs==NULL)
	{
		printf( "Failed to get valid framebuffer config.\n");
		uninitialize();
		exit(1);
	}
	printf("%d Matching FB Configs Found.\n",iNumFBConfigs);
	
	int bestFramebufferConfig=-1;
	int worstFramebufferConfig=-1;
	int bestNumberOfSamples=-1;
	int worstNumberOfSamples=999;

	for(i=0;i<iNumFBConfigs;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
		if(pTempXVisualInfo)
		{
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&sampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);
printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
			
			if(bestFramebufferConfig < 0  ||  sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFramebufferConfig=i;
				bestNumberOfSamples=samples;
			}


			if( worstFramebufferConfig < 0  ||  !sampleBuffer ||  samples < worstNumberOfSamples)
			{
				worstFramebufferConfig=i;
			    	worstNumberOfSamples=samples;
			}
		}
		XFree(pTempXVisualInfo);
	}

	bestGLXFBConfig = pGLXFBConfigs[bestFramebufferConfig]; //Saving FBConfig here

	gGLXFBConfig=bestGLXFBConfig; //Saving it in global varaiable
	
	XFree(pGLXFBConfigs);
	
	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );
	
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(
					gpDisplay,
					RootWindow(gpDisplay,gpXVisualInfo->screen), 
					gpXVisualInfo->visual,
					AllocNone); 
										
	winAttribs.event_mask=StructureNotifyMask | KeyPressMask | ButtonPressMask |PointerMotionMask|ExposureMask | VisibilityChangeMask;
	
	styleMask=CWBorderPixel | CWEventMask | CWColormap;
	gColormap=winAttribs.colormap;										           
	
	gWindow=XCreateWindow(gpDisplay,
			      RootWindow(gpDisplay,gpXVisualInfo->screen),
			      0,
			      0,
			      WIN_WIDTH,
			      WIN_HEIGHT,
			      0,
			      gpXVisualInfo->depth,         
		              InputOutput, 
			      gpXVisualInfo->visual,
			      styleMask,
			      &winAttribs);

	if(!gWindow)
	{
		printf("XCreateWindow() failed\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay,gWindow,"First OpenGL BlueScreen Window");
	
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);

	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
	Atom wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	
	XEvent event;
	memset(&event,0,sizeof(XEvent));
	
	event.type=ClientMessage;

	event.xclient.window=gWindow;

	event.xclient.message_type=wm_state;

	event.xclient.format=32;

	event.xclient.data.l[0]=gbFullscreen ? 0 : 1;

	Atom fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);	

	event.xclient.data.l[1]=fullscreen;
	
	XSendEvent(gpDisplay,
		RootWindow(gpDisplay,gpXVisualInfo->screen),
		False, 
		StructureNotifyMask, 
		&event);	
}

void initialize(void)
{
	//Step 1 :  Use GetProcAddress to get the function address
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	
	//Step 2 : Define the version of opengl context you want (3.3 not working)
	GLint attribs[] = 
		{
		GLX_CONTEXT_MAJOR_VERSION_ARB,3,
		GLX_CONTEXT_MINOR_VERSION_ARB,1,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0 };
		
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);

	if(!gGLXContext) 
	{
		GLint attribs[] = 
			{
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0 };

		printf("Failed To Create GLX 3.1 context and hence using 1.0 Context\n");
		
		gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	}
	else 
	{
		printf("OpenGL Context 3.1 Is Created.\n");
	}
	
	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("Indirect GLX Rendering context obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering context obtained\n" );
	}
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f,0.0f,1.0f,0.0f);
	
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void resize(int width,int height)
{
	if(height==0)
	{
		height=1;
	}
		
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glXSwapBuffers(gpDisplay,gWindow);
}

void uninitialize(void)
{	
	GLXContext currContext=glXGetCurrentContext();


	if(currContext!=NULL && currContext==gGLXContext)
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
		gpXVisualInfo=NULL;
	}
	
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
