/*******************************************/
/* Comment/Uncomment depending your device */
// #include <M5Stack.h>
#include <M5Atom.h>
/*******************************************/
#include "Grove_Motor_Driver_TB6612FNG.h"
#include <Wire.h>
#include <Bluepad32.h>
#include <Adafruit_NeoPixel.h>

#define PIN_RGB              27  
#define NUMPIXELS             1   
#define GAMEPAD_DEAD_ZONE_X  50
#define GAMEPAD_DEAD_ZONE_Y  50
#define GAMEPAD_MAX_X       510
#define GAMEPAD_MAX_Y       510
#define MOTOR_MIN_SPEED      50
#define MOTOR_MAX_SPEED     255

Adafruit_NeoPixel pixels = Adafruit_NeoPixel( NUMPIXELS, PIN_RGB,NEO_GRB + NEO_KHZ800);

ControllerPtr myControllers[BP32_MAX_CONTROLLERS];
MotorDriver motor;

void setup() {

  M5.begin(true,false);
  pixels.begin();
  pixels.setPixelColor(0,0,0,200);
  pixels.show();
  Wire.begin(26, 32);
  Wire.begin(26, 32);
  Serial.begin(9600);
  while (!Serial) {
    ;
  }  

  Serial.println();
  Serial.println("M5ZR");

  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  // To get the BD Address (MAC address) call:
  const uint8_t* addr = BP32.localBdAddress();
  Serial.print("BD Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(addr[i], HEX);
    if (i < 5)
      Serial.print(":");
    else
      Serial.println();
  }

  motor.init();  

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();  
}

void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;

  pixels.setPixelColor(0,0,50,0);
  pixels.show();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;

      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;
  
  pixels.setPixelColor(0,100,0,0);
  pixels.show();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr gamepad) {
  int16_t speed_x;
  int16_t speed_y;
  // There are different ways to query whether a button is pressed.
  // By query each button individually:
  //  a(), b(), x(), y(), l1(), etc...

  if (gamepad->a()) {    
    // A pressed
  }

  if (gamepad->b()) {
    //B pressed
  }

  if (gamepad->x())  
  {
    // X pressed
  }

  if (gamepad->y())  
  {
    // Y pressed
  }
  speed_x=MOTOR_MIN_SPEED+(abs(gamepad->axisX())-GAMEPAD_DEAD_ZONE_X)*MOTOR_MAX_SPEED/GAMEPAD_MAX_X;
  speed_y=MOTOR_MIN_SPEED+(abs(gamepad->axisY())-GAMEPAD_DEAD_ZONE_Y)*MOTOR_MAX_SPEED/GAMEPAD_MAX_Y;

  if (gamepad->axisY()<-GAMEPAD_DEAD_ZONE_Y) {          // FORWARD
    if (gamepad->axisX()<-200) {                        //  + LEFT
      motor.dcMotorRun(MOTOR_CHA, -speed_y);
      motor.dcMotorRun(MOTOR_CHB, -(speed_y-speed_x+MOTOR_MIN_SPEED));
    }
    else if (gamepad->axisX()>200) {                    //  + RIGHT
      motor.dcMotorRun(MOTOR_CHA, -(speed_y-speed_x+MOTOR_MIN_SPEED));
      motor.dcMotorRun(MOTOR_CHB, -speed_y);
    }
    else
    {
      motor.dcMotorRun(MOTOR_CHA, -speed_y);
      motor.dcMotorRun(MOTOR_CHB, -speed_y);    
    }
  }
  else if (gamepad->axisY()>GAMEPAD_DEAD_ZONE_Y) {      // BACKWARD    
    motor.dcMotorRun(MOTOR_CHA,  speed_y);
    motor.dcMotorRun(MOTOR_CHB,  speed_y);
  }
  else  if (gamepad->axisX()<-GAMEPAD_DEAD_ZONE_X) {    // LEFT
    motor.dcMotorRun(MOTOR_CHA, -150);
    motor.dcMotorRun(MOTOR_CHB,  150);
  }
  else   if (gamepad->axisX()>GAMEPAD_DEAD_ZONE_Y) {    // RIGHT
    motor.dcMotorRun(MOTOR_CHA,  150);
    motor.dcMotorRun(MOTOR_CHB, -150);
  } 
  else {                                // STOP
    motor.dcMotorStop(MOTOR_CHA);
    motor.dcMotorStop(MOTOR_CHB);
  }
}



void loop()
{
  BP32.update();

  // It is safe to always do this before using the controller API.
  // This guarantees that the controller is valid and connected.
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      if (myController->isGamepad())
        processGamepad(myController);
    }
  }
  delay(50);
}