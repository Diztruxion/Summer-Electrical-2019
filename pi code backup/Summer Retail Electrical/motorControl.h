//motorControl.h
//also contains defined encoder class to deal with C static function linking

#ifndef motorCONTROL_H_
#define motorCONTROL_H_

#define X_MODE 1
//used to find RPM, set X_MODE on encoder configuration
//                      us->s   s->min  edge->RPM
#define PULSE_CONSTANT 1000000.0*60.0/(90.0*X_MODE)
//For reliable input
#define STEADY 10

// costants for PID controller once determined
// #define Kp
// #define Kd
// #define Ki

//used for encoder function throw
typedef void (*re_decoderCB_t)(int, int);

/*
 * MAP of Motor Location and PWM/Control with GPIO pins:
 * 
 *         			     FRONT_MOTOR[0] (DIR=5, PWM=6)
 * 
 * LEFT_MOTOR[3] (DIR=3, PWM=4)            RIGHT_MOTOR[1] (DIR=23, PWM=24)
 * 
 *         			     REAR_MOTOR[2] (DIR=25, PWM=26)
 */
#define FRONT_MOTOR 0
//GPIO pins for Front Motor
#define FRONT_MOTOR_DIR 16
#define FRONT_MOTOR_PWM 26 //Was 6
#define FRONT_MOTOR_ENC_A 9
#define FRONT_MOTOR_ENC_B 7

#define RIGHT_MOTOR 1
//GPIO pins for Right Motor
#define RIGHT_MOTOR_DIR 20
#define RIGHT_MOTOR_PWM 21 //Was 24
#define RIGHT_MOTOR_ENC_A 25
#define RIGHT_MOTOR_ENC_B 7

#define REAR_MOTOR 2
//GPIO pins for Rear Motor
#define REAR_MOTOR_DIR 6
#define REAR_MOTOR_PWM 12 //Was 26
#define REAR_MOTOR_ENC_A 11
#define REAR_MOTOR_ENC_B 7

#define LEFT_MOTOR 3
//GPIO pins for Left Motor
#define LEFT_MOTOR_DIR 13
#define LEFT_MOTOR_PWM 19 //Was 4
#define LEFT_MOTOR_ENC_A 8
#define LEFT_MOTOR_ENC_B 7

#define OFF 0
#define ON 1

#define PWM_FREQ 1000 //was 400/800/1000

#define OK 0

class Motor{

	//For linking to C for Encoder
	static void _pulseExMotor(int PI_VALUE, unsigned gpio, unsigned level, uint32_t tick, void *user);
	
	public:
		
		//For Encoder Only for Motor
		int callback_A, callback_B, lastGpio;
		unsigned mygpioA, mygpioB, levA, levB;
		
		re_decoderCB_t mycallback;
		
		void _pulseMotor(int PI_VALUE, unsigned gpio, unsigned level, uint32_t tick);
		
		void EncoderSetup(int PI_VALUE, unsigned gpioA, unsigned gpioB, re_decoderCB_t callback);
		
		//Speed tracking and for PID loop
		float RPM, setSpeed, Kp,Ki,Kd;
		int count;//Tracks the number of rising edges on the particular motor.
		
		uint32_t prev_tick;

		/*
		This function releases the resources used by the decoder.
		*/
		void re_cancel(void);
	
	
		//Rest of Motor Class
		Motor(int, int, int, int, unsigned, unsigned);

		int getdirPIN();
		int getpwmPIN();
		float getsetSpeed();
		int getmotor();
		int getspdPIN();
		
		bool rpmControl;
		
		int pinSetup();
				
		void changeRun(int);
		int getRun();

		void changeSpeed(float);
		bool changeState(bool);
		
		void display();
		
		int PI_VALUE;
		
		unsigned encA;
		unsigned encB;
				
//		void getDistance(); //returns the distance the motor has moved since initialization
		int motor;

		float percent; //tracks percent output to PWM control
		
	private:
		int running; //If it should, what direction 0 OFF; 1 Running
		int distance; //distance will track number of encoder ticks
		int dirPIN;
		int pwmPIN;
		int spdPIN;
		float error, prev_error, total_error;


};

#endif
