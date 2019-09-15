//speedControl.h

//Some encoder/callback code is taken directly from the pigpio example code.
//Specifically Rotary Encoder Examples in C++ 2013-12-30 http://abyz.me.uk/rpi/pigpio/examples.html

#ifndef speedCONTROL_H_
#define speedCONTROL_H_

#include <stdint.h>

//PULSE_CONSTANT is assuming tick is going to me in microseconds,
//Also, (360 pulses/rot)/(60 second/min) gives the factor of 60... May use milliseconds later
//To speed up the float math.
#define PULSE_CONSTANT 60.0/1000000.0

//defined the time a signal needs to be set to be reported
#define STEADY 100

typedef void (*re_decoderCB_t)(int);

//Entire class may become a sub-class of Motor later
class SpeedControl{
	
	public:
		SpeedControl(int PI_VALUE, unsigned int encA, unsigned int encB, int motor, re_decoderCB_t callback);
		~SpeedControl();
		
		int encoderSetup(); //return for errors

		int PI_VALUE;
				
	private:
		uint32_t cur_tick;
		uint32_t prev_tick;
		
		unsigned int encA;
		unsigned int encB;
		int motor;
		int callback_A;
		int callback_B;
		
		float RPM;
		
		re_decoderCB_t mycallback;
		
		//single callback function to trigger on each level change.
		void _pulse(int pin, int level, uint32_t tick);

		/* Need a static callback to link with C. */
		static void _pulseEx(int pin, int level, uint32_t tick, void *user);
		
		
};
	
#endif
