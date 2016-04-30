/** motion.cpp
*
*/
#include "motion.h"
#include "config.h"

//コンストラクタ
Motion::Motion(){
  pinMode(laserPin,OUTPUT);

  myStepperX = new Stepper(stepsPerRevolution, xPin1,xPin2,xPin3,xPin4);
  myStepperY = new Stepper(stepsPerRevolution, yPin1,yPin2,yPin3,yPin4);

  // Decrease if necessary
  myStepperX->setSpeed(250);
  myStepperY->setSpeed(250);
}

//デストラクタ
Motion::~Motion(){
  delete myStepperX;
  delete myStepperY;
}

// laser の On,Off
void Motion::laserPower(uint8_t value) {
  digitalWrite(laserPin,value);
  delay(LineDelay);
  if (verbose) {
    if(value == LOW){
      //LOW
      Serial.println("Laser off.");
    }
    else{
      //HIGH
      Serial.println("Laser on.");
    }
  }
}

//指定位置へ移動する
/*********************************
 * Draw a line from (x0;y0) to (x1;y1).
 * Bresenham algo from https://www.marginallyclever.com/blog/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
 * int (x1;y1) : Starting coordinates
 * int (x2;y2) : Ending coordinates
 **********************************/
void Motion::movePosition(float x1, float y1) {
  if (verbose)
  {
    Serial.print("fx1, fy1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }

  //  Bring instructions within limits
  /* 相対的な位置 への対応の為 コメントアウト
  if (x1 >= Xmax) {
    x1 = Xmax;
  }
  if (x1 <= Xmin) {
    x1 = Xmin;
  }
  if (y1 >= Ymax) {
    y1 = Ymax;
  }
  if (y1 <= Ymin) {
    y1 = Ymin;
  }
  */

  if (verbose)
  {
    Serial.print("Xpos, Ypos: ");
    Serial.print(Xpos);
    Serial.print(",");
    Serial.print(Ypos);
    Serial.println("");
  }

  if (verbose)
  {
    Serial.print("x1, y1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }

  //  Convert coordinates to steps
  x1 = (int)(x1*StepsPerMillimeterX);
  y1 = (int)(y1*StepsPerMillimeterY);
  float x0 = Xpos;
  float y0 = Ypos;

  //  Let's find out the change for the coordinates
  long dx = abs(x1-x0);
  long dy = abs(y1-y0);
  int sx = x0<x1 ? StepInc : -StepInc;
  int sy = y0<y1 ? StepInc : -StepInc;

  long i;
  long over = 0;

  if (dx > dy) {
    for (i=0; i<dx; ++i) {
      myStepperX->step(sx);
      over+=dy;
      if (over>=dx) {
        over-=dx;
        myStepperY->step(sy);
      }
      delay(StepDelay);
    }
  }
  else {
    for (i=0; i<dy; ++i) {
      myStepperY->step(sy);
      over+=dx;
      if (over>=dy) {
        over-=dy;
        myStepperX->step(sx);
      }
      delay(StepDelay);
    }
  }

  if (verbose)
  {
    Serial.print("dx, dy:");
    Serial.print(dx);
    Serial.print(",");
    Serial.print(dy);
    Serial.println("");
  }

  if (verbose)
  {
    Serial.print("Going to (");
    Serial.print(x0);
    Serial.print(",");
    Serial.print(y0);
    Serial.println(")");
  }

  //  Delay before any next lines are submitted
  delay(LineDelay);
  //  Update the positions
  Xpos = x1;
  Ypos = y1;
}
