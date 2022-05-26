#ifndef VTF_SETUP_H
#define VTF_SETUP_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include "Adafruit_DRV2605.h"
#include "servo_setup.h"

// Multiplexer Address
#define TCAADDR0            0x70

// LRAs Setup
#define RATED_VOLTAGE       99
#define OVERDRIVE_VOLTAGE   161
#define COMPENSATION        12  
#define BACK_EMF            108  

// Init motors
#define DEFAULT             0
#define AUTOCALIBRATE       1

#define MOTOR0 0 // motor 6
#define MOTOR1 1 // motor 7
#define MOTOR2 2 // motor 5
#define MOTOR3 3 // motor 3
#define MOTOR4 4 // motor 4
#define MOTOR5 5 // motor 2
#define MOTOR6 6 // motor 0
#define MOTOR7 7 // motor 1 

// Define the type of message being sent over serial
#define START_BLOCK 		0xF3
#define END_BLOCK 			0xF4
#define START_CMD 		    0xF1
#define END_CMD 			0xF2
#define TYPE_PREDEFINED 	0xF5
#define TYPE_REAL_TIME 		0xF6
#define TYPE_PAUSE 			0xF8


// Vibration Types
#define PRE_DEFINED         0 
#define REAL_TIME           1 

class vtf_setup {

    public:
        
        vtf_setup(void);

        // Setup Driver 
        void change_motor(int motorNum);
        void tcas_set_multi(int* motors);
        void LRA_setup(int motorNum, Adafruit_DRV2605* drv_x, int set_up_type);
        void ERM_setup(int motorNum, Adafruit_DRV2605* drv_x);
        uint8_t get_tcas(int motorNum);
        uint8_t get_tcas_port(int motorNum);
        uint8_t is_LRA(int motorNum);

        // Control Motor
        void set_motor(int effect, int motor); 
        void switch_vibration_type(int vibration_type);

        // Process message block/s from serial
        void process_real_time (int* data);
        void process_pre_defined(int* data); 
        void process_pause(int* data); 
        void process_block(int* data, int dataSize); 

        void set_programmed_mode(int motorNum); 
        void set_individual_motor(int motorNum, int effect); 
        void set_real_time_mode(int motorNum); 
        void set_real_time_motor(int motorNum, int intensity); 
        
        // Init motor 
        void motor_setup(void);
        void init_vtf(void); 

    private:
};

#endif