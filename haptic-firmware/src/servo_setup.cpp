#include <servo_setup.h>

Servo servo;  // create servo object to control a servo

servo_setup::servo_setup() {}


/**************************************************************************/
/*!
  @brief Set counter clockwise quarter turn
*/
/**************************************************************************/
void servo_setup::ccw_quarter(void) {

	  servo.writeMicroseconds(CCW);
	  delay(QUARTER_TURN);
    servo.writeMicroseconds(STP);
    Serial.print(" quarter ccw turn\n");
}


/**************************************************************************/
/*!
  @brief Set clockwise quarter turn
*/
/**************************************************************************/
void servo_setup::cw_quarter(void) {

	  servo.writeMicroseconds(CW);
	  delay(QUARTER_TURN);
    servo.writeMicroseconds(STP);
    Serial.print(" quarter cw turn\n");
}


/**************************************************************************/
/*!
  @brief Set counter clockwise squeeze turn
*/
/**************************************************************************/
void servo_setup::ccw_squeeze(void) {

	  servo.writeMicroseconds(CCW);
   	delay(SQUEEZE_TURN);
    servo.writeMicroseconds(STP);
    Serial.print(" squezze ccw turn\n");
}


/**************************************************************************/
/*!
  @brief Set clockwise squeeze turn
*/
/**************************************************************************/
void servo_setup::cw_squeeze(void) {

	  servo.writeMicroseconds(CW); 
	  delay(SQUEEZE_TURN);
    servo.writeMicroseconds(STP);
    Serial.print(" squeeze cw turn\n");
}


/**************************************************************************/
/*!
  @brief Initialise servo
*/
/**************************************************************************/
void servo_setup::init_servo(void) {
    
    servo.attach(9);  // attaches the servo on pin 9 to the servo object
}
