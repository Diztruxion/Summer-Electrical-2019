//piCleanup.c
 #include <stdio.h>
 #include <pigpiod_if2.h>
 
 #define LOW 0

//This will set all pins on the pi as input pins regardless of usage
//as per suggested in pi documentation to prevent grounding outputs
//Cleanup GPIO 2-27 Pins 0/1 are reserved for EEPROM access and are not used.

void piCleanup(int PI_VALUE){
	int temp = 0;
	
	for (int i = 2; i <= 27; i++){
		
		temp = gpio_write(PI_VALUE, i, LOW);
		temp = set_mode(PI_VALUE, i, PI_OUTPUT);
		if (temp != 0) printf("Pin %d Failed Cleanup\n", i);
	}
	printf("\nCleanup Successful!");
		
}
