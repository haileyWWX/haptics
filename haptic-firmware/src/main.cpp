#include <Arduino.h>
#include "servo_setup.h"
#include "vtf_setup.h"

servo_setup myServo;
vtf_setup vtf; 

// Flags/Counters for setting up servo base line 
int numBaseTurnCount = 0; // keep track of how many turns till baseline
int baseQuarTurnCount = 0; // keep track of how many quarter turns for baseline 

int baseSet = 0; // check if 'Set' is pressed, true when baseline set else do baseline init 
int ccw = 0; // check if counterclockwise
int cw = 0; // check if clockwise

// Squeeze Counters/flags
int numSqueezeCount = 0; // keep track of how many times squeeze is pressed
int release = 0; // check if release 


/**************************************************************************/
/*!
  @brief Debug print function to keep track parameters
  @param baseCount base counter
  @param cw Clockwise flag
  @param ccw Counterclockwise flag
  @param quar Turn counter
  @param set Set flag 
*/
/**************************************************************************/
void print_servo_debug(int baseCount, int cw, int ccw, int quar, int set) {
	
	Serial.print("Base:");
	Serial.println(baseCount);
	Serial.print("CW:");
	Serial.println(cw); 
	Serial.print("CCW:");
	Serial.println(ccw); 
	Serial.print("Quarter:");
	Serial.println(quar); 
	Serial.print("Set:");
	Serial.println(set);
	Serial.print("\n");
} 

/**************************************************************************/
/*!
  @brief Handle armband setup
  @param incomingByte Command from serial to execute 
  					  clockwise/counterclockwise/turn/set
*/
/**************************************************************************/
void setup_baseline(int incomingByte) {
	
	switch (incomingByte) {
		case CLOCKWISE: 
			cw = 1; 
			break; 

		case COUNTERCLOCKWISE: 
			ccw = 1; 
			break;

		case QUARTER: 
			if (cw == 1) {
				myServo.cw_quarter();
				numBaseTurnCount++; 
				baseQuarTurnCount++;
				print_servo_debug(numBaseTurnCount, cw, ccw, baseQuarTurnCount, baseSet); 
				cw = 0; 
			} else if (ccw == 1) {
				myServo.ccw_quarter();							
				numBaseTurnCount--; 
				baseQuarTurnCount--; 
				print_servo_debug(numBaseTurnCount, cw, ccw, baseQuarTurnCount, baseSet); 
				ccw = 0; 
			}
			break;

		case SET: 
			baseSet = 1;
			cw = 0; 
			ccw = 0; 
			Serial.print("base line set\n"); 
			break; 
	}
	
}

/**************************************************************************/
/*!
  @brief Debug print function to keep track parameters
  @param squeezeCount number squeeze button pressed
  @param release number release button pressed
*/
/**************************************************************************/
void print_sq_servo_debug(int squeezeCount, int release) {
	
	Serial.print("SqueezeCount:");
	Serial.println(squeezeCount);
	Serial.print("Release:");
	Serial.println(release); 
	Serial.print("\n");
} 


/**************************************************************************/
/*!
  @brief Handle squeezing after baseline is set
  @param incomingByte Command from serial to execute 
  					  squeeze/loosen/release/end
*/
/**************************************************************************/
void do_squeeze(int incomingByte) {

	// start squeezing in cw or ccw 
	switch (incomingByte) {

		case SQUEEZE: 
			myServo.cw_squeeze();
			numSqueezeCount++; 
			print_sq_servo_debug(numSqueezeCount, release); 
			break;
		
		case LOOSEN: 
			myServo.ccw_squeeze();							
			numSqueezeCount--;  				
			print_sq_servo_debug(numSqueezeCount, release); 
			break;
			
		case RELEASE: 
			release = 1; 
			while (release == 1) {
				if (baseQuarTurnCount != 0) {
					if (baseQuarTurnCount > 0) {
						// if squeeze count is positive 
						for (int i = baseQuarTurnCount; i > 0 ; --i) {
							myServo.ccw_quarter();
							baseQuarTurnCount--; 
						}
						for (int i = numSqueezeCount; i > 1 ; --i) {
							myServo.ccw_squeeze();
							numSqueezeCount--; 
						}
					} else if (baseQuarTurnCount < 0) {
						// if squeeze count is negative
						for (int i = baseQuarTurnCount; i < 0 ; ++i) {
							myServo.ccw_quarter();
							baseQuarTurnCount++; 
						}	
						for (int i = numSqueezeCount; i < 1 ; ++i) {
							myServo.ccw_squeeze();
							numSqueezeCount++; 
						}
					}
				} else {
					release = 0; 
				}
			} 
			print_sq_servo_debug(baseQuarTurnCount, release); 			
			break; 

		default: 
			break; 
	}
}


/**************************************************************************/
/*!
  @brief Initialise all hardware 
*/
/**************************************************************************/
void setup() {
    Serial.begin(9600);
	
    myServo.init_servo(); 
	vtf.init_vtf(); 
}

// Main loop 
void loop() {  
  	
	int incomingByte = 0; 
	// servo data process
	int set = 0; // check if 'Set' is pressed, true when baseline set else do baseline init 

	int buffer = 20; 
	int startCnt = 0;
	int endCnt = 0; 
	int gettingData = 0; 
	int dataPos = 0; 
	int* dataProcess; 
	dataProcess = (int*)malloc(sizeof(int) * buffer); 
	
	while(1) {
	
		// Serial Processing 
		if (Serial.available() > 0) {
			Serial.print("received: ");
			incomingByte = Serial.read(); 
			Serial.println(incomingByte);
		}

		// process vibrotactiles 
		if (gettingData) {
			if (incomingByte == END_BLOCK) {
				endCnt++; 
				if (endCnt == 2) {
					vtf.process_block(dataProcess, dataPos); 
					gettingData = 0; 
					dataPos = 0; 
					endCnt = 0;
				}
			} else {
				if (endCnt == 1) {
					dataProcess[dataPos] = END_BLOCK; 
					dataPos++; 
					endCnt = 0; 
				} 
				dataProcess[dataPos] = incomingByte; 
				dataPos++; 
				if (dataPos == buffer) {
				buffer = buffer * 2;
				dataProcess = (int*)realloc(dataProcess, buffer); 
				}
			}
		}

		if (set == 0) {
			setup_baseline(incomingByte); // set up armband's base line
			if (incomingByte == SET) {
				set = 1; 
			}
		} else {
			// process squeeze 
			do_squeeze(incomingByte);
			
			if (incomingByte == END) {
				set = 0; 
			} 

			if (incomingByte == START_BLOCK) {
				startCnt++; 
				if (startCnt == 2) {
					gettingData = 1; 
				} 
			} else {
				startCnt = 0;
			}
			incomingByte = 0;
		}
		 
	}
	Serial.println("Done");
}