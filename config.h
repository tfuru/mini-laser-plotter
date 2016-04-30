/* config.h - compile time configuration
 *
 *
 */

#ifndef mlp_config_h
#define mlp_config_h

#include <Arduino.h>

// Serial 起動時に送信する文字列 (UniversalGcodeSender 等がバージョン認識に利用)
#define FIRMWARE_HEADER_LINE "Grbl 0.9j [MLP v0.1]"

namespace {

  // Serial baud rate
  #define BAUD_RATE 115200

  // G-code 1行分のバファー
  #define LINE_BUFFER_LENGTH 512

  // デバッグ プリントする
  boolean verbose = true;

  // Laser Pin
  const int laserPin = 10;
  
  // Initialize steppers for X- and Y-axis using this Arduino pins L9110S
  const int xPin1 = 4,xPin2 = 5,xPin3 = 2,xPin4 = 3;
  const int yPin1 = 6,yPin2 = 7,yPin3 = 8,yPin4 = 9;
  
  // Should be right for DVD steppers, but is not too important here
  const int stepsPerRevolution = 20;
  
  //  Drawing settings, should be OK
  float StepInc = 1;
  int StepDelay = 10;
  int LineDelay = 60;
  
  // Motor steps to go 1 millimeter.
  // Use test sketch to go 100 steps. Measure the length of line.
  // Calculate steps per mm. Enter here.
  float StepsPerMillimeterX = 42.0;
  float StepsPerMillimeterY = 42.0;
  
  // Drawing robot limits, in mm
  // OK to start with. Could go up to 50 mm if calibrated well.
  float Xmin = 0;
  float Xmax = 155;
  float Ymin = 0;
  float Ymax = 155;
  
  float Xpos = Xmin;
  float Ypos = Ymin;
  
  // Structures, global variables
  struct point {
    float x;
    float y;
    float z;
  };
  
  // Current position of plothead
  struct point actuatorPos;
  
  //原点からの絶対的な位置への移動モード
  boolean isAbsolute = true;
}

#endif
