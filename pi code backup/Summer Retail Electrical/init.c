//init.c
#include <pigpiod_if2.h>
#include <stdio.h>
//#include "speedControl.h"
#include "motorControl.h"

int init(void){
	
	int PI_VALUE = 0;
	//int temp =0;
	PI_VALUE = pigpio_start(NULL,NULL);
	
	return PI_VALUE;
}

