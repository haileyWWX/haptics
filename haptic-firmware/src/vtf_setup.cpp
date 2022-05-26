#include "vtf_setup.h"
 
// VTF DRIVER INSTANCE
Adafruit_DRV2605 drv;

// MOTOR DRIVER INSTANCES
Adafruit_DRV2605 drv0;
Adafruit_DRV2605 drv1;
Adafruit_DRV2605 drv2;
Adafruit_DRV2605 drv3;
Adafruit_DRV2605 drv4;
Adafruit_DRV2605 drv5;
Adafruit_DRV2605 drv6;
Adafruit_DRV2605 drv7;
Adafruit_DRV2605* driver_list[8] = {&drv0, &drv1, &drv2, &drv3, &drv4, &drv5, &drv6, &drv7};

// GLOBAL MOTOR VALUES
int globalMotorsSet[9] = {40};
int globalMotorPos = 0;
int motorPos0 = 0;
int motorPos1 = 0;
int motorPos2 = 0;
int motorPos3 = 0;
int motorPos4 = 0;
int motorPos5 = 0;
int motorPos6 = 0;
int motorPos7 = 0;

// Global Vibration Modes 
int currentMode = PRE_DEFINED; 
int currentRealTimeMotor = 40;

vtf_setup::vtf_setup(){}

/**************************************************************************/
/*!
  @brief Change to a single motor
  @param motorNum Motor to be selected
*/
/**************************************************************************/
void vtf_setup::change_motor(int motorNum) {
    
    byte error;
    byte error1;

    int tcas = get_tcas(motorNum);
    int port = get_tcas_port(motorNum);

    Wire.beginTransmission(tcas);
    Wire.write(1 << port);
    error = Wire.endTransmission(tcas);

	//Serial.println("here?");
    if (error) {
		Serial.print("Error occured when writing to ");

        if (tcas == TCAADDR0) {
            Serial.println("TCAADDR0");
        } 

		if (error == 5) {
		    Serial.println("It was a timeout");
		    Serial.println();
        }
	}

    if (tcas != TCAADDR0) {
        Wire.beginTransmission(TCAADDR0);
        Wire.write(0);
        error1 = Wire.endTransmission(TCAADDR0);

        if (error1) {
		    Serial.println("Error occured when writing to TCAADDR0");
        }

		if (error == 5) {
            Serial.println("It was a timeout");
            Serial.println();
        }
    }
}


/**************************************************************************/
/*!
  @brief Select multiple motor drivers at the same time for easy programming
  @param motors Motors to be set, Terminated with a -1
*/
/**************************************************************************/
void vtf_setup::tcas_set_multi(int *motors) {
    uint8_t TCAADDR0_Addr = 0;

    for(int i = 0; i < 9; i++) { 
        if(motors[i] == -1) {

            break;
        } else {
         
            if(get_tcas(motors[i])==TCAADDR0_Addr) {
                TCAADDR0_Addr = TCAADDR0_Addr + (1<<get_tcas_port(motors[i]));
            }
            
        }
    }
    
    Wire.beginTransmission(TCAADDR0);
    Wire.write(TCAADDR0_Addr);
    Wire.endTransmission(TCAADDR0);
}


/**************************************************************************/
/*!
  @brief Return the TCAS Address for a given Motor
  @param motorNum Motor number to return TCAs adress for

*/
/**************************************************************************/
uint8_t vtf_setup::get_tcas(int motorNum) {
   
    if (motorNum < 8) {
        return TCAADDR0;
    }
    return -1;
}


/**************************************************************************/
/*!
  @brief Return the TCAS port for a given motor
  @param motorNum Motor number to return TCAs port for

*/
/**************************************************************************/
uint8_t vtf_setup::get_tcas_port(int motorNum) {
    
    //Serial.println("get tcas port");

    if (motorNum < 8) {
       
        if (motorNum == 0) {
            return 6;
        } else if (motorNum == 1) {
            return 7;
        } else if (motorNum == 2) {
            return 5;
        } else if (motorNum == 3) {
            return 3;
        } else if (motorNum == 4) {
            return 4;
        } else if (motorNum == 5) {
            return 2;
        } else if (motorNum == 6) {
            return 0;
        } else if (motorNum == 7) {
            return 1;
        }
    }
    return -1;
}


/**************************************************************************/
/*!
  @brief Setup an LRA using the DRV2605 Auto Calibrate. Will only work
         with the LV101040A LRA motor. Params must be changed for other motors
  @param motorNum    Motor to be setup
  @param drv_x       DRV2605 instance for given motorNum initiated in main
  @param set_up_type Determains if Auto Calibration is attempted 
                     or Default values are used

*/
/**************************************************************************/
void vtf_setup::LRA_setup(int motorNum, Adafruit_DRV2605* drv_x, int set_up_type) {
   
    // Serial.println("Starting AUTO Calibration");
    change_motor(motorNum);
    drv_x->begin();
    drv_x->writeRegister8(0x01, 0x00);                                              // out of standby

    drv_x->writeRegister8(DRV2605_REG_RATEDV, RATED_VOLTAGE);                        //Set RATED_VOLTAGE ->106
    drv_x->writeRegister8(DRV2605_REG_CLAMPV, OVERDRIVE_VOLTAGE);                    //Set OD_CLAMP (Max Allowed Voltage) ->126

    uint8_t feedback_val = 0;
    feedback_val |= (1 << 7);                                                       // Set LRA mode
    feedback_val |= (3 << 4);                                                       // Set Brake Factor to 4x
    feedback_val |= (2 << 2);                                                       // Set Loop Gain to high
    drv_x->writeRegister8(DRV2605_REG_FEEDBACK, feedback_val);

    //Control 1
    uint8_t control_1_val = 0;
    control_1_val |= (1<<7);                                                        //Set Startup Boost
    control_1_val |= (24<<0);                                                       //Set DRIVE_TIME 24    
    drv_x->writeRegister8(DRV2605_REG_CONTROL1, control_1_val);

    //Control 2
    uint8_t control_2_val = 0;
    control_2_val |= (1<<7);                                                        //Set Bidirectional input mode
    control_2_val |= (1<<6);                                                        //Set Brake Stabaliser
    control_2_val |= (2<<4);                                                        //Set SAMPLE_TIME -> 250us
    control_2_val |= (1<<2);                                                        //Set BLANKING_TIME
    control_2_val |= (1<<0);                                                        //Set IDISS_TIME (Current Dissapate Time)
    drv_x->writeRegister8(DRV2605_REG_CONTROL2, control_2_val);

    //Control 3
    uint8_t control_3_val = 0;
    control_3_val |= (2<<6);                                                        //Set NG_THRESH 4%
    control_3_val |= (1<<5);                                                        //For ERMs (Not Important)
    control_3_val |= (1<<0);                                                        //Disables Auto-resonance mode as this is a wideband LRA
    drv_x->writeRegister8(DRV2605_REG_CONTROL3, control_3_val);
    

    //Control 4
    uint8_t control_4_val = 0;
    control_4_val |= (3 << 4);                                                        //Set Auto Cal Time to 1000 ms (minimum), 1200 ms (maximum)
    drv_x->writeRegister8(DRV2605_REG_CONTROL4, control_4_val);

    if (set_up_type == AUTOCALIBRATE) {
        //Auto Calibraiton
        drv_x->writeRegister8(DRV2605_REG_MODE, 0x07);                               //Auto Calibration Mode

        drv_x->go();                                                                 //Begin Auto Calibration
        while((drv_x->readRegister8(DRV2605_REG_GO)  & 0x01) != 0) {                    //Wait For Calibration to finish
            delay(2);
        }                        

        //Check if Calibration was successful
        if ((drv_x->readRegister8(DRV2605_REG_STATUS) & (0x08))) {                                       
            Serial.println("AUTO Calibration Failed");
        }

    } else if (set_up_type == DEFAULT) {
       
        drv_x->writeRegister8(DRV2605_REG_AUTOCALCOMP, COMPENSATION);
        drv_x->writeRegister8(DRV2605_REG_AUTOCALEMP, BACK_EMF);
    }
    
    //Setup as LRA in Predef Mode
    drv_x->selectLibrary(6);
    drv_x->setMode(DRV2605_MODE_INTTRIG); 
    drv_x->setWaveform(0, 0);
	drv_x->go();
    
    /*Use this to see print out of Calibration Values */
    Serial.print("Motor ");
    Serial.print(motorNum);
    Serial.print(" compensation:");
    Serial.print(drv_x->readRegister8(DRV2605_REG_AUTOCALCOMP));
    Serial.print(" back-EMF:");
    Serial.println(drv_x->readRegister8(DRV2605_REG_AUTOCALEMP));
    return;
}


/**************************************************************************/
/*!
  @brief Setup an ERM motor
  @param motorNum Motor to be setup
  @param drv_x DRV2605 instance for given motorNum initiated in main

*/
/**************************************************************************/
void vtf_setup::ERM_setup(int motorNum, Adafruit_DRV2605* drv_x) {
   
    change_motor(motorNum);
    drv_x->begin();
    drv_x->selectLibrary(1);
	drv_x->setMode(DRV2605_MODE_INTTRIG); 
	drv_x->setWaveform(0, 0);
    drv_x->go();
}


/**************************************************************************/
/*!
  @brief Check if Motor is type LRA
  @param motorNum Motor to be selected
  @return 1 if LRA else ERM 0  
*/
/**************************************************************************/
uint8_t vtf_setup::is_LRA(int motorNum) {
    
    if(motorNum == 2 || motorNum == 5) {
        
        // ERM motor 
        return 0;
    } else {
        
        return 1;
    }
}


/**************************************************************************/
/*!
  @brief Setup motors
  @param effect type of vibration (pre-programmed mode)
  @param motor Motor's number to be setup
*/
/**************************************************************************/
void vtf_setup::set_motor(int effect, int motor) {
	
	Serial.print("\nEffect:");
	Serial.print(effect);
	Serial.print(" Motor:");
	Serial.println(motor);

	if (motor == 0) {
		
		drv0.setWaveform(motorPos0, (effect));
		motorPos0++;
	} else if (motor == 1) {

		drv1.setWaveform(motorPos1, (effect));
		motorPos1++;	
	} else if (motor == 2) {
		
		drv2.setWaveform(motorPos2, (effect));
		motorPos2++;
	} else if (motor == 3) {
		
		drv3.setWaveform(motorPos3, (effect));
		motorPos3++;
	} else if (motor == 4) {
		
		drv4.setWaveform(motorPos4, (effect));
		motorPos4++;
	} else if (motor == 5) {
		
		drv5.setWaveform(motorPos5, (effect));
		motorPos5++;
	} else if (motor == 6) {
		
		drv6.setWaveform(motorPos6, (effect));
		motorPos6++;
	} else if (motor == 7) {
		
		drv7.setWaveform(motorPos7, (effect));
		motorPos7++;
	} 
}


/**************************************************************************/
/*!
  @brief Switch between real time & pre-defined modes 
  @param vibrationType type of mode 
  					   pre-programmed mode - 0
					   real time mode - 1
*/
/**************************************************************************/
void vtf_setup::switch_vibration_type(int modeType) {

	if (modeType == REAL_TIME) {
		Serial.println("CHANGING TO REALTIME");
		for (int i = 0; i < 8; i++) {
			change_motor(i);
			driver_list[i]->setMode(DRV2605_MODE_REALTIME); 
			Wire.endTransmission();
		}		
		Serial.println("Done");
	} else {
		Serial.println("CHANGING TO PRE");
		for (int i = 0; i < 8; i++) {
			change_motor(i);
			driver_list[i]->setMode(DRV2605_MODE_INTTRIG); 
			Wire.endTransmission();
		}
		Serial.println("Done");
	}
}


/**************************************************************************/
/*!
  @brief Process pause 
  @param data pause duration 
*/
/**************************************************************************/
void vtf_setup::process_pause(int* data) {
	int pauseTime = 0; 

	for (int i = 0; i < 4; i++) {
		pauseTime += data[i + 1] << (i * 8); 
	}

	delay(pauseTime);
} 


/**************************************************************************/
/*!
  @brief Process Real Time
  @param data Info from serial to execute real time mode
*/
/**************************************************************************/
void vtf_setup::process_real_time (int* data) {
	Serial.print("\n1Current Real Time Motor:"); 
	Serial.print( currentRealTimeMotor); 

	Serial.print("\n Motor Num: "); 
	Serial.println(data[1]); 
	if (data[1] != currentRealTimeMotor) {
		change_motor(data[1]); 
		currentRealTimeMotor = data[1]; 
		delay(250); 
	}
	Serial.print("\n Intensity:"); 
	Serial.println(data[2]); 
	driver_list[currentRealTimeMotor]->setRealtimeValue(data[2]); 

	Wire.endTransmission();
}


/**************************************************************************/
/*!
  @brief Process Pre-Defined
  @param data Info from serial to execute pre-defined mode
*/
/**************************************************************************/
void vtf_setup::process_pre_defined(int* data) {

	byte test; 
	int motorPos = 0; 
	int motors[9], first[8] = {0}; 
	int firstInt = data[2]; 

	// place 1 into the array to indicate which motor picked
	for (int i = 0; firstInt > 0; i++) {
		first[i] = firstInt%2; 
		firstInt = firstInt/2; 
	}

	// if 1 present in the array, that motor is pick, swap with motor num
	for (int i = 0; i < 8; i++) {
		if(first[i]) {
			motors[motorPos] = i; 
			motorPos++;
		}
	}

	motors[motorPos] = -1; 

	int alreadySet = 0; 
	for (int i = 0; i <= motorPos; i++) {
		for (int j = 0; j < 9; j++) {
			if (globalMotorsSet[j] == motors[i]) {
				alreadySet = 1; 
			}
		}

		if ((!alreadySet) && (motors[i] != -1)) {
			globalMotorsSet[globalMotorPos] = motors[i]; 
			globalMotorPos++; 
		}

		alreadySet = 0; 
	}

	Serial.print("Global motor pos: ");
	Serial.print(globalMotorPos);

	Serial.print("Data Block Info: Data[1]: "); 
	Serial.print(data[1]); 

	Serial.print(" Data[2]: "); 
	Serial.print(data[2]);  

	// set motor 
	for (int i = 0; i < motorPos; i++) {
		change_motor(motors[i]); 
		set_motor(data[1] + 1, motors[i]);
		test = Wire.endTransmission();
		Serial.print("Wire End: "); 
		Serial.println(test);
	}

	globalMotorsSet[globalMotorPos] = -1;


	for (int i = 0; i < 8; i++) {
		Serial.print(globalMotorsSet[i]);
		Serial.print("\n");
	}
	
	Serial.print("global motor pos: ");
	Serial.print(globalMotorPos);

	// fire motor 
	for(int i = 0; i < globalMotorPos; i++ ) {
		Serial.println("\n Firing Motors");
		change_motor(globalMotorsSet[i]);
		set_motor(0, globalMotorsSet[i]); 
		driver_list[globalMotorsSet[i]]->go();
		Wire.endTransmission();
	}

	Serial.println("\nMotors Fired");

	// clear motor 
	for(int i = 0; i < 9; i++) {
		globalMotorsSet[i] = 40;
	}
		
	globalMotorPos = 0;
	motorPos0 = 0;
	motorPos1 = 0;
	motorPos2 = 0;
	motorPos3 = 0;
	motorPos4 = 0;
	motorPos5 = 0;
	motorPos6 = 0;
	motorPos7 = 0;
	
	delay(250);

	Serial.println("Cleared effect\n");
}


/**************************************************************************/
/*!
  @brief Process Block
  @param data Process block data from serial
*/
/**************************************************************************/
void vtf_setup::process_block(int* data, int dataSize) {
	
	// 2D block to pack data
	int dataBlock[dataSize][10]; 

	// Counters
	int comStartCnt = 0; 
	int comEndCnt = 0; 
	int comPos = 0; 
	int comNum = 0; 
	int inCom = 0; 

	// Pack data blocks
	for (int i = 0; i < dataSize; i++) {
		
		if (inCom) {
			
			if (data[i] == END_CMD) {
				comEndCnt++; 
				if (comEndCnt == 2) {
					comStartCnt = comEndCnt = 0; 
					comPos = 0; 
					comNum++; 
					inCom = 0;
				}
			} else {
				dataBlock[comNum][comPos] = data[i];
				comPos++;
			}
		}
		if (data[i] == START_CMD) {
			comStartCnt++; 
			if (comStartCnt == 2) {
				inCom = 1; 
			}
		} else {
			comStartCnt = 0;
		}
	}
	
	// Execute data blocks 
	for (int i = 0; i < comNum; i++) {

		// Process Predefined
		if (dataBlock[i][0] == TYPE_PREDEFINED) {
			Serial.println("Pre-Def Type"); 
			if (currentMode == PRE_DEFINED) {
				process_pre_defined(dataBlock[i]); 
			} else {
				switch_vibration_type(PRE_DEFINED); 
				currentMode = PRE_DEFINED;
				process_pre_defined(dataBlock[i]); 
			}

		} else if (dataBlock[i][0] == TYPE_REAL_TIME) {
			// Process Real Time Data Block 
			Serial.println("Real Time Type"); 
			if (currentMode == REAL_TIME) { 
				process_real_time(dataBlock[i]); 
			} else {
				switch_vibration_type(REAL_TIME); 
				currentMode = REAL_TIME;
				process_real_time(dataBlock[i]); 
			}
		} else if (dataBlock[i][0] == TYPE_PAUSE) {
			process_pause(dataBlock[i]);
		} 
	}
}


/**************************************************************************/
/*!
  @brief Set Programmed Mode
  @param motorNum Motor number set to pre-programmed mode
*/
/**************************************************************************/
void vtf_setup::set_programmed_mode(int motorNum) {
		
	change_motor(motorNum); 
	driver_list[motorNum]->setMode(DRV2605_MODE_INTTRIG);
	Wire.endTransmission();
}


/**************************************************************************/
/*!
  @brief Set Real Time Mode
  @param motorNum Motor number set to real time mode
*/
/**************************************************************************/
void vtf_setup::set_real_time_mode(int motorNum) {
	
	change_motor(motorNum); 
	driver_list[motorNum]->setMode(DRV2605_MODE_REALTIME);
	Wire.endTransmission();
}


// testing real time motor individually 
/**************************************************************************/
/*!
  @brief Test function for testing real time mode on individual motors
  @param motorNum Motor selected 
  @param intensity Intensity value
*/
/**************************************************************************/
void vtf_setup::set_real_time_motor(int motorNum, int intensity) {
	
	set_real_time_mode(motorNum);
	change_motor(motorNum);	
	driver_list[motorNum]->setRealtimeValue(intensity); 
	Wire.endTransmission();
}


/**************************************************************************/
/*!
  @brief Setup all motors
*/
/**************************************************************************/
void vtf_setup::motor_setup(void) {

    // ERM
	ERM_setup(MOTOR2, &drv2);
	ERM_setup(MOTOR5, &drv5);

    // LRA
	LRA_setup(MOTOR0, &drv0, AUTOCALIBRATE);
	LRA_setup(MOTOR1, &drv1, AUTOCALIBRATE);
	LRA_setup(MOTOR3, &drv3, AUTOCALIBRATE);
	LRA_setup(MOTOR4, &drv4, AUTOCALIBRATE);
	LRA_setup(MOTOR6, &drv6, AUTOCALIBRATE);
	LRA_setup(MOTOR7, &drv7, AUTOCALIBRATE);
}


/**************************************************************************/
/*!
  @brief Initialise vtf module
*/
/**************************************************************************/
void vtf_setup::init_vtf(void) {
	
	Serial.println("VTF Driver Running");
	delay(1000);

	Wire.begin();
	Wire.setClock(100000);
	Wire.setTimeout(10000);

	motor_setup(); 
	Serial.println("Setup Done");
}