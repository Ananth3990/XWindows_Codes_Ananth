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

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

FILE *gpFile=NULL;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
typedef GLXContext (*glXCreateContextAttribsARBProc) (Display*,GLXFBConfig,GLXContext,Bool,const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;
GLXFBConfig  gGLXFBConfig;
GLXContext  gGLXContext;
char ascii[32];

bool gbFullscreen = false;

int main(int argc,char *argv[])
{
	void CreateWindow(void);
	void initialize(void);
	void display(void);
	void uninitialize();
	void ToggleFullScreen(void);
	void resize(int,int);

	XEvent event;
	KeySym keySym;
	int winWidth;
	int winHeight;
	bool bDone=false;

	gpFile=fopen("Log.txt","w");
	if(gpFile==NULL)
	{
		printf("Log file can now be created.Exiting now");

	}
	else
	{
		fprintf(gpFile,"Log file is sucessfully opened");
	}

	CreateWindow();

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

							case XK_f:
							case XK_F:
								if(gbFullscreen==true)
								{
										ToggleFullScreen();
										gbFullscreen=false;
								}
								else
								{
										ToggleFullScreen();
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
									case 1:

										break;

									case 2:

										break;

									case 3:

										break;

									case 4:

										break;

									case 5:

										break;

									case 6:

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
} 

void CreateWindow(void)
{
	void initialize(void);
	void uninitialize(void);

	XSetWindowAttributes  winAttribs;
	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumFBConfigs=0;
	int StyleMask;
	int i;

	static int frameBufferAttributes[]=
	{
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 8,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
		None
	};

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("Unable to open XDsiplay\n");
		uninitialize();
		exit(1);

	}

	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
	if(pGLXFBConfigs==NULL)
	{
		printf("Failed to get valid framebuffer Config\n");
		uninitialize();
		exit(1);
	}
	printf("%d matching FBConfigs found\n",iNumFBConfigs);

	int bestFramebufferConfig=-1;
	int worstFramebufferConfig=-1;
	int bestNumberOfSamples=-1;
	int worstNumberOfSamples=999;

	for(int i=0;i<iNumFBConfigs;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
		if(pTempXVisualInfo)
		{
			int SampleBuffer,samples;

			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&SampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);

			printf("Matching framebuffer config=%d : Visual Id =0X%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,SampleBuffer,samples);

			if(bestFramebufferConfig <0 || SampleBuffer && samples > bestNumberOfSamples )
			{
					bestFramebufferConfig=i;
					bestNumberOfSamples=samples;
			}

			if(worstFramebufferConfig <0 || !SampleBuffer ||  samples <  worstNumberOfSamples)
			{
				worstFramebufferConfig=1;
				worstNumberOfSamples=samples;
			}

			bestGLXFBConfig=pGLXFBConfigs[bestFramebufferConfig];

			gGLXFBConfig = bestGLXFBConfig;

			XFree(pGLXFBConfigs);

			gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);

			printf("Chosen Visual Id =0X%lu\n",gpXVisualInfo->visualid);

			winAttribs.border_pixel=0;
			winAttribs.background_pixmap=0;
			winAttribs.colormap=XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),gpXVisualInfo->visual,AllocNone);
			winAttribs.event_mask=StructureNotifyMask|KeyPressMask|ButtonPressMask|ExposureMask|VisibilityChangeMask|PointerMotionMask;
			StyleMask=CWBorderPixel|CWEventMask|CWColormap;
			gColormap=winAttribs.colormap;

			gWindow=XCreateWindow(
				gpDisplay,
				RootWindow(gpDisplay,gpXVisualInfo->screen),
				0,
				0,
				WIN_WIDTH,
				WIN_HEIGHT,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				StyleMask,
				&winAttribs
				);

			if(!gWindow)
			{
				printf("Problem in Window Creation\n");
				uninitialize();
				exit(1);
			}

			XStoreName(gpDisplay,gWindow,"OpenGL Window");

			Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);

			XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
			XMapWindow(gpDisplay,gWindow);
		}
	}
}

void initialize(void)
{
	void initialize();
	void resize(int,int);
	void uninitialize(void);

	glXCreateContextAttribsARB=(glXCreateContextAttribsARBProc)glXGetProcAddress((GLubyte*) "glXCreateContextAttribsARB");


	GLint attribs[]={
		GLX_CONTEXT_MAJOR_VERSION_ARB,3,
		GLX_CONTEXT_MINOR_VERSION_ARB,3,
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0
	};

	gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);

	if(!gGLXContext)
	{
		GLint attribs[]={
			GLX_CONTEXT_MAJOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			0		
		};

		printf("Failed to create 3.3 context. Hence using old style GLX Context\n");

		gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
	}
	else
	{
		printf("OpenGL Context 3.3 created\n");

	}

	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("Indirect GLX Rendering Context Obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering Context Obtained\n");	
	}

	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f,0.0f,1.0f,0.0f);

	resize(WIN_WIDTH,WIN_HEIGHT);
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
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glXSwapBuffers(gpDisplay,gWindow);	
}

void uninitialize(void)
{
	GLXContext currentContext=glXGetCurrentContext();
	if(currentContext!=NULL && currentContext==gGLXContext)
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

	if(gpFile)
	{
		fprintf(gpFile,"Log File is about to be closed\n");
		fclose(gpFile);
		gpFile=NULL;
	}
}

void ToggleFullScreen(void)
{

}
