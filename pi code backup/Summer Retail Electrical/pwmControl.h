//pwmControl.h
#ifndef pwmCONTROL_H_
#define pwmCONTROL_H_
 
#define OK 0
#define PWM_RANGE 1000 //Used to try variable PWM range steps, still based on percent.. broken
//#define RANGE_RESOLUTION 100 // will determine how many steps we can use for variable speed

#define MAX_SPEED 50 //called when changing speed so we never exceed the safe limit of motors

#define HARD_STOP -1 //Used when we want... a hard stop on a PWM pin instead of ramp down

int initPWM(int PI_VALUE, int freq, float percent, int pin);
int changePWM(int PI_VALUE, float percent, int pin);


#endif



