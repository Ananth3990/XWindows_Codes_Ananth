#include<stdio.h>
#include<stdlib.h>

typedef struct TestStruct
{
	union Member
	{
		int i;
	}m;		
}ts;

int main()
{
	ts *temp=NULL;
	temp=(ts*)malloc(sizeof(ts));

	while(1)
	{
		printf("Enter a number\n");
		scanf("%d",&temp->m.i);

		if(temp->m.i==1)
		{
			printf("Shree Ganesha\n");
		}
		else if(temp->m.i==2)
		{
			printf("Jai Shree Ram\n");
		}
		else if(temp->m.i==0)
		{
			break;
		}
		else
		{
			printf("Unknown functionality\n");
		}
	}
}
