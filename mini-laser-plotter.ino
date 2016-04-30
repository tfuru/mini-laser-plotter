#include "gcode.h"
Gcode gcode;

void setup() {
  //g-code 解析の為の初期化
  gcode.init();
}

void loop() {
  //g-code 解析
  gcode.execute();
}

