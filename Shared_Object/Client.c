#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>

int main(int argc,char **argv)
{
	void *handle;
	int (*fptr)(int,int);
	char *error;

	handle=dlopen("./Server.so",RTLD_LAZY);
	if(!handle)
	{
		fprintf(stderr,"%s\n",dlerror());
		exit(EXIT_FAILURE);
	}
	

	fptr=(int (*)(int,int))dlsym(handle,"Add");

	if((error=dlerror())!=NULL)
	{
		fprintf(stderr,"%s\n",error);
		exit(EXIT_FAILURE);
	}
	printf("Add is %d\n",(*fptr)(4,2));
	
	fptr=(int (*)(int,int))dlsym(handle,"Sub");
	if((error=dlerror())!=NULL)
	{
		fprintf(stderr,"%s\n",error);
		exit(EXIT_FAILURE);
	}

	printf("Sub is %d\n",(*fptr)(4,2));

	fptr=(int (*)(int,int))dlsym(handle,"Mul");
	if((error=dlerror())!=NULL)
	{
		fprintf(stderr,"%s\n",error);
		exit(EXIT_FAILURE);
	}

	printf("Mul is %d\n",(*fptr)(4,2));
	
	fptr=(int (*)(int,int))dlsym(handle,"Div");
	if((error=dlerror())!=NULL)
	{
		fprintf(stderr,"%s\n",error);
		exit(EXIT_FAILURE);
	}

	printf("Div is %d\n",(*fptr)(4,2));

	dlclose(handle);
	handle=NULL;
	exit(EXIT_SUCCESS);
}
