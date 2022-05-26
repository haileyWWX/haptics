#ifndef SERVO_SETUP_H
#define SERVO_SETUP_H

#include <Servo.h>
#include <Arduino.h> 

// Define the type of message being sent over serial
// DIRECTION TYPES 
#define CLOCKWISE 0x31 // 0x31 -  '1' 
#define COUNTERCLOCKWISE 0x32 // 0x32 - '2'

// TURN TYPES
#define QUARTER 0xC8 // 0x33 - '3' 
#define SET 0xC9 // 0x36 - '6'
#define RESET 0xCA // 0x37 - '7'
#define SQUEEZE 0xCB // 0x38 - '8' 
#define LOOSEN 0xCC // 0x39 - '9' 
#define RELEASE 0xCD // 0x40 - '10' 
#define END 0xCE 

// Write servo direction values (servo.writeMicroseconds)
#define CW 1160 // clockwise
#define CCW 1850 // counter clockwise
#define STP 1550 // stop 

// Turn magnitude for x time (delay(x))
#define QUARTER_TURN 600 // 225 
#define SQUEEZE_TURN 100  // for squeezing 

class servo_setup {
    
    public:
        servo_setup(void);
        void init_servo(void); 

        // for base line 
        void ccw_quarter(void); 
        void cw_quarter(void); 

        // for squeezing 
        void ccw_squeeze(void); 
        void cw_squeeze(void); 

    private:
};

#endif