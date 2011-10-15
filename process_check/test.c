#include<stdio.h>
#include<stdlib.h>
void main()
{
	char buf[100];

	sprintf(buf,"CAO NI MA\n");
	printf("sizeof buf = %d ,strlen buf = %d\n",sizeof(buf),strlen(buf));
}
