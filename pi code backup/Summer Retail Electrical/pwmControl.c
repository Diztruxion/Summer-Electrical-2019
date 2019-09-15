//pwmControl.c
/*
 * This will make PWM control easier later on in the project by
 * simpifying our code slightly
*/
 #include <stdio.h>
 #include <pigpiod_if2.h>
 #include <math.h>
 #include "pwmControl.h"

 /* 
 * Configure PWM based on a given freq, a percent/100 and the IO pin.
 * Used during initial setup of PWM only
 * 
 */
 
 int initPWM(int PI_VALUE, int freq, float percent, int pin){
	 
	int temp = 0; //Used for catching values from the GPIO daemon
	int dutyCycle = (percent/100.0)*PWM_RANGE; //Used for Duty Cycle
	 
	//First check to see if the GPIO pin is an output, if not set it
	temp = get_mode(PI_VALUE, pin); 
	
	if (temp == PI_BAD_GPIO) return temp; //unable to retrieve status

	if (temp == PI_INPUT) printf("Switch Modes\n");
	temp = set_mode(PI_VALUE, pin, PI_OUTPUT);
	if (temp != OK) return temp;
		
	//Pin is set as output, set frequency, and write if it doesn't match
	temp = set_PWM_frequency(PI_VALUE, pin, freq);
	if((temp == PI_BAD_USER_GPIO) || (temp == PI_NOT_PERMITTED)) return temp;
	if(temp != freq) printf("Real freq set to %d", temp);
	
	temp = set_PWM_range(PI_VALUE, pin, PWM_RANGE);
	if(temp != OK) return temp;
	
	temp = set_PWM_dutycycle(PI_VALUE, pin, dutyCycle);
	if(temp != OK) return temp;
		
	return 0;
 }
 
 
 //Changes the PWM percent on a pin. Assumes pin is setup for PWM
 int changePWM(int PI_VALUE, float percent, int pin){
	int temp = 0; //Used to catch errors
	
	//Hard shuts down the motors if we send HARD_STOP command (-1)
	if (percent == HARD_STOP){
		set_PWM_dutycycle(PI_VALUE, pin, 0);
		return 0;
	}
	//if we exceed the Max set speed, revert to maximum speed
	if (percent > MAX_SPEED) percent = MAX_SPEED;
	
	/*
	USED ONLY TO FIX DEMO, REMOVE AND FIX BETTER AFTER ADJUSTS LEFT MOTOR TO COMPENSATE FOR STARTING RESISTANCE
	*/
	
	
	int dutyCycle = (percent/100.0)*PWM_RANGE; //Used for Duty Cycle -Changed June 14th, not tested
	
	temp = set_PWM_dutycycle(PI_VALUE, pin, dutyCycle);
	if(temp != OK) return temp;
	 
	return 0;
}
