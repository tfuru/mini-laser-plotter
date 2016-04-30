/** motion.h
*
*/
#ifndef mlp_motion_h
#define mlp_motion_h

#include <Arduino.h>
#include <Stepper.h>

class Motion {
public:
  //コンストラクタ
  Motion();
  //デストラクタ
  ~Motion();
  // laser の On,Off
  void laserPower(uint8_t value);  
  //指定位置へ移動する
  void movePosition(float x1, float y1);

private:
  Stepper *myStepperX;
  Stepper *myStepperY;
};

#endif
