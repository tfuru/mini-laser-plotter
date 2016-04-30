/*
下記ページのコードを参考にしました。
 Mini CNC Plotter - Arduino Based
 [instructables] http://www.instructables.com/id/Mini-CNC-Plotter-Arduino-Based/
 [G-code/ja] http://reprap.org/wiki/G-code/ja#G92:_.E4.BD.8D.E7.BD.AE.E3.82.92.E6.8C.87.E5.AE.9A.E3.81.99.E3.82.8B.28Set_Position.29

 Mini CNC Plotter firmware, based in TinyCNC https://github.com/MakerBlock/TinyCNC-Sketches
 Send GCODE to this Sketch using gctrl.pde https://github.com/damellis/gctrl
 Convert SVG to GCODE with MakerBot Unicorn plugin for Inkscape available here https://github.com/martymcguire/inkscape-unicorn

 More information about the Mini CNC Plotter here (german, sorry): http://www.makerblog.at/2015/02/projekt-mini-cnc-plotter-aus-alten-cddvd-laufwerken/
*/
#include "gcode.h"
#include "config.h"

//コンストラクタ
Gcode::Gcode(){

}

//デストラクタ
Gcode::~Gcode(){

}

// 初期化
void Gcode::init(){
  // Serial 初期化
  Serial.begin( BAUD_RATE );

  Serial.println( FIRMWARE_HEADER_LINE );

  Serial.println("Mini Laser Plotter (Benbox 300mW Mini Laser)");

  Serial.print("X range is from ");
  Serial.print(Xmin);
  Serial.print(" to ");
  Serial.print(Xmax);
  Serial.println(" mm.");
  Serial.print("Y range is from ");
  Serial.print(Ymin);
  Serial.print(" to ");
  Serial.print(Ymax);
  Serial.println(" mm.");
  Serial.println();

  Serial.println("Needs to interpret");
  Serial.println("G00,G01 for moving");
  Serial.println("G04 P150 (wait 150ms)");
  Serial.println("M02 (End of program)");
  Serial.println("M03 S30 (laser off)");
  Serial.println("M03 S50 (laser on)");
  Serial.println();

  //レーザー off
  motion.laserPower(LOW);
}

// Serialから受信した g-code 1行づつ解析
uint8_t Gcode::execute(){
  delay(200);

  char line[ LINE_BUFFER_LENGTH ];
  char c;
  int lineIndex;
  bool lineIsComment, lineSemiColon;

  lineIndex = 0;
  lineSemiColon = false;
  lineIsComment = false;

  while ( Serial.available()>0 ) {
    c = Serial.read();
    if (( c == '\n') || (c == '\r') ) {             // End of line reached
      if ( lineIndex > 0 ) {                        // Line is complete. Then execute!
        line[ lineIndex ] = '\0';                   // Terminate string
        processIncomingLine( line, lineIndex );
        lineIndex = 0;
      }
      else {
        // Empty or comment line. Skip block.
      }
      lineIsComment = false;
      lineSemiColon = false;
      //Serial.println("ok");
    }
    else {
      if ( (lineIsComment) || (lineSemiColon) ) {   // Throw away all comment characters
        if ( c == ')' )  lineIsComment = false;     // End of comment. Resume line.
      }
      else {
        if ( c <= ' ' ) { // Throw away whitepace and control characters
        }
        else if ( c == '/' ) {                    // Block delete not supported. Ignore character.
        }
        else if ( c == '(' ) {                    // Enable comments flag and ignore all characters until ')' or EOL.
          lineIsComment = true;
        }
        else if ( c == ';' ) {
          lineSemiColon = true;
        }
        else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
          Serial.println( "ERROR - lineBuffer overflow" );
          lineIsComment = false;
          lineSemiColon = false;
        }
        else if ( c >= 'a' && c <= 'z' ) {        // Upcase lowercase
          line[ lineIndex++ ] = c-'a'+'A';
        }
        else {
          line[ lineIndex++ ] = c;
        }
      }
    }
  }
}

void Gcode::processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  // Hope that 64 is enough for 1 parameter

  while( currentIndex < charNB ) {
     // Select command, if any
    switch ( line[ currentIndex++ ] ) {
    case 'U':
      motion.laserPower(LOW);
      break;
    case 'D':
      motion.laserPower(HIGH);
      break;
    case 'G':{
        cmdG(line,currentIndex);
        Serial.println("ok");
        break;
      }
    case 'M':{
        cmdM(line,currentIndex);
        Serial.println("ok");
      }
    }
  }
}

//各コマンドの番号 取得
void Gcode::getCmdNum(char *buffer,char *line,int currentIndex){
  int i = 0;
  for(i=0;i<64;i++){
    char c = line[ currentIndex++ ];
    if(isdigit(c) == 0){
      currentIndex-=1;
      break;
    }
    buffer[i] = c;
  }
  buffer[i] = '\0';
}

//Gコマンド解析&実行
void Gcode::cmdG(char *line,int currentIndex){
  char buffer[ 64 ];
  //コマンド番号を取得
  getCmdNum(buffer,line,currentIndex);

  struct point newPos;
  newPos.x = 0.0;
  newPos.y = 0.0;

  // Select G command
  switch ( atoi( buffer ) ){
    case 0:
    case 1:{
        if (verbose){
          Serial.println("G0,G1 move");
        }
        char* indexX = strchr( line+currentIndex, 'X' );
        char* indexY = strchr( line+currentIndex, 'Y' );

        if(isAbsolute == true){
          //原点からの絶対的な位置への移動
          if (verbose){
            Serial.println("absolute");
          }
          if ( indexY <= 0 ) {
            newPos.x = atof( indexX + 1);
            newPos.y = actuatorPos.y;
          }
          else if ( indexX <= 0 ) {
            newPos.x = actuatorPos.x;
            newPos.y = atof( indexY + 1);
          }
          else {
            newPos.x = atof( indexX + 1);
            newPos.y = atof( indexY + 1);
          }
          actuatorPos.x = newPos.x;
          actuatorPos.y = newPos.y;
        }
        else{
          //相対的な位置を設定する
          if (verbose){
            Serial.println("relative");
          }
          actuatorPos.x += atof( indexX + 1);
          actuatorPos.y += atof( indexY + 1);
          newPos.x = actuatorPos.x;
          newPos.y = actuatorPos.y;
        }

        motion.movePosition(newPos.x, newPos.y);
        break;
    }
    case 2:{
      //G2 Controlled Move Arc Clockwise
      //時計回りの制御移動
      if (verbose){
        Serial.println("G2 Controlled Move Arc Clockwise");
      }
      break;
    }
    case 3:{
      //G3 Controlled Move Arc Clockwise-Clockwise
      //反時計回りの制御移動
      if (verbose){
        Serial.println("G3 Controlled Move Arc Clockwise-Clockwise");
      }
      break;
    }
    case 4: case 53:{
      if (verbose){
        Serial.println("G4,G53 Dwell");
      }
      char* p = strchr( line+currentIndex, 'P' );
      Serial.print("p:");
      Serial.println(p);
      delay( atoi( p + 1) );
      break;
    }
    case 90:{
      if (verbose){
        Serial.println("G90 Set to Absolute Positioning");
      }
      isAbsolute = true;
      break;
    }
    case 91:{
      if (verbose){
        Serial.println("G91 Set to Relative Positioning");
      }
      isAbsolute = false;
      break;
    }
    case 28:{
      //原点に移動する
      if (verbose){
        Serial.println("G28 Move to Origin");
      }
      motion.movePosition(0,0);
      break;
    }
    case 10: case 30: case 92:{
      //Set Position
      if (verbose){
        Serial.println("G10,G28,G30,G92 Set Position");
      }
      char* indexX = strchr( line+currentIndex, 'X' );
      char* indexY = strchr( line+currentIndex, 'Y' );

      if((indexX == '\0') && (indexY == '\0')){
        Xpos = actuatorPos.x;
        Ypos = actuatorPos.y;
        newPos.x = actuatorPos.x;
        newPos.y = actuatorPos.y;
      }
      else {
        newPos.x = atof( indexX + 1);
        newPos.y = atof( indexY + 1);
        motion.movePosition(newPos.x, newPos.y);
      }

      actuatorPos.x = Xmin;
      actuatorPos.y = Ymin;
      Xpos = Xmin;
      Ypos = Ymin;

      break;
   }
   default:{
    Serial.print( "Command not recognized : G");
    Serial.println( buffer );
   }
  }
}

//Mコマンドの解析&実行
void Gcode::cmdM(char *line,int currentIndex){
  char buffer[ 64 ];

  //コマンド番号を取得
  getCmdNum(buffer,line,currentIndex);

  struct point newPos;
  newPos.x = 0.0;
  newPos.y = 0.0;

  switch ( atoi( buffer ) ){
    case 2:{
      motion.laserPower(LOW);
      newPos.x = 0;
      newPos.y = 0;
      motion.movePosition(newPos.x, newPos.y);
      break;
    }
    case 3:
    case 300:{
      char* indexS = strchr( line+currentIndex, 'S' );
      float Spos = atof( indexS + 1);
      if (Spos == 30) {
        motion.laserPower(HIGH);
      }
      if (Spos == 50) {
        motion.laserPower(LOW);
      }
      break;
    }
  default:
    Serial.print( "Command not recognized : M");
    Serial.println( buffer );
  }
}



