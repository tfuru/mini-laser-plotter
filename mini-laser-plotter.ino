#include "gcode.h"

void setup() {
  //g-code 解析の為の初期化
  gcode_init();
}

void loop() {
  //g-code 解析
  gcode_execute();
}

