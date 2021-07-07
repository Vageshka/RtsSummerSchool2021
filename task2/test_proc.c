#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>


int main(){
	int f;
	f=open("/dev/mydevice",O_RDONLY);
	for(int i=0; i<5; i++){
		unsigned int k;
		read(f,&k,sizeof(k));
		printf("num: %d\n",k);
	}
	return 0;
}	
