/*
下記ページのコードを参考にしました。
 Mini CNC Plotter - Arduino Based
 [instructables] http://www.instructables.com/id/Mini-CNC-Plotter-Arduino-Based/

 Mini CNC Plotter firmware, based in TinyCNC https://github.com/MakerBlock/TinyCNC-Sketches
 Send GCODE to this Sketch using gctrl.pde https://github.com/damellis/gctrl
 Convert SVG to GCODE with MakerBot Unicorn plugin for Inkscape available here https://github.com/martymcguire/inkscape-unicorn

 More information about the Mini CNC Plotter here (german, sorry): http://www.makerblog.at/2015/02/projekt-mini-cnc-plotter-aus-alten-cddvd-laufwerken/
*/
#ifndef mlp_gcode_h
#define mlp_gcode_h

#include <Arduino.h>
#include "motion.h"

class Gcode {
public:
  //コンストラクタ
  Gcode();
  //デストラクタ
  ~Gcode();
  
  // 初期化
  void init();

  // Serialから受信した g-code 1行づつ解析
  uint8_t execute();  
private:
  // 各コマンドを実行する
  void processIncomingLine( char* line, int charNB );

  //各コマンドの番号 取得
  void getCmdNum(char *buffer,char *line,int currentIndex);

  //Mコマンドの解析&実行
  void cmdM(char *line,int currentIndex);

  //Gコマンド解析&実行
  void cmdG(char *line,int currentIndex);

  //指定位置へ移動する
  void movePosition(float x1, float y1);

  Motion motion;
};

#endif
