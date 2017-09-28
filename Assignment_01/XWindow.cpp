#include<iostream>
#include<stdio.h> //for printf
#include<stdlib.h> //for exit
#include<memory.h> //for memset

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

using namespace std;

bool bFullScreen = false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo =NULL;
Colormap gColorMap;
Window gWindow;
int igWindowWidth=800;
int igWindowHeight=600;

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
	void CreateWindow();
        void ToggleFullScreen();
	void uninitialize(void);

	int winWidth=800;
	int winHeight=600;

	CreateWindow();

        XEvent event;
        KeySym keysym;

	doFileIO("Inside main()\n");

        while(1)
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
                                                                uninitialize();
                                                                exit(0);
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
                                                case 4:
                                                        break;
                                                case 5:
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
                        break;

                        case Expose:
                                        break;

                        case DestroyNotify:
                                        break;

                        case 33:
                                uninitialize();
                                exit(0);

                        default:
                                break;
                }
        }

        uninitialize();

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

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("Unable to open XDisplay\n");
		printf("Exiting Now\n");
		uninitialize();
		exit(1);
	}

	defaultScreen=XDefaultScreen(gpDisplay);
	
	defaultDepth=XDefaultDepth(gpDisplay,defaultScreen);
;
	gpXVisualInfo=(XVisualInfo*)malloc(sizeof(XVisualInfo));

	if(XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo)==0)
	{
		printf("Unable to get a visual\n");
		printf("Exiting Now\n");
		uninitialize();
		exit(1);
	}

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;

	winAttribs.colormap=XCreateColormap(
	gpDisplay,
	RootWindow(gpDisplay,gpXVisualInfo->screen),
	gpXVisualInfo->visual,
	AllocNone
	);

	gColorMap=winAttribs.colormap;	

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

        gWindow=XCreateWindow(gpDisplay,
                        RootWindow(gpDisplay,gpXVisualInfo->screen),
                        0,
                        0,
                         igWindowWidth,
                         igWindowHeight,
                         0,
                         gpXVisualInfo->depth,
                         InputOutput,
                         gpXVisualInfo->visual,
                         styleMask,
                         &winAttribs
                         );

        if(!gWindow)
        {
            printf("Failed to Create Window\n");
            printf("Exiting now\n");
            uninitialize();
            exit(1);   
        }       	
 
        XStoreName(gpDisplay,gWindow,"Shree Ganesha");

        Atom ACDestroyWindow=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
        XSetWMProtocols(gpDisplay,gWindow,&ACDestroyWindow,1);

        XMapWindow(gpDisplay,gWindow);

	doFileIO("Outside CreateWiindow()\n");
}

void uninitialize(void)
{
	doFileIO("Inside uninitialize()\n");

	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}

	if(gColorMap)
	{
		XFreeColormap(gpDisplay,gColorMap);
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
		fclose(gpFile);
		gpFile=NULL;
	}

	doFileIO("Outside uninitialize()\n");
}

void ToggleFullScreen()
{
	Atom wm_state;
	Atom fullscreen;

	doFileIO("Inside ToggleFullscreen()\n");

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

	XSendEvent(gpDisplay,
			RootWindow(gpDisplay,gpXVisualInfo->screen),
			False,
			StructureNotifyMask,
			&xev);


	doFileIO("Outside ToggleFullScreen()\n");	
}
