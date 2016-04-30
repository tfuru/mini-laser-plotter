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

// Initialize steppers for X- and Y-axis using this Arduino pins L9110S
Stepper myStepperX(stepsPerRevolution, xPin1,xPin2,xPin3,xPin4);
Stepper myStepperY(stepsPerRevolution, yPin1,yPin2,yPin3,yPin4);

// 初期化
void gcode_init(){
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

  //レーザー
  pinMode(penLaserPin,OUTPUT);
  laserPower(LOW);
  
  // Decrease if necessary
  myStepperX.setSpeed(250);
  myStepperY.setSpeed(250);
}

// Serialから受信した g-code 1行づつ解析
uint8_t gcode_execute(){
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

void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  // Hope that 64 is enough for 1 parameter
  
  while( currentIndex < charNB ) {
     // Select command, if any
    switch ( line[ currentIndex++ ] ) {             
    case 'U':
      laserPower(LOW);
      break;
    case 'D':
      laserPower(HIGH);
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
void getCmdNum(char *buffer,char *line,int currentIndex){
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
void cmdG(char *line,int currentIndex){
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

        movePosition(newPos.x, newPos.y);      
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
    case 4:{
      if (verbose){
        Serial.println("G4 Dwell");
      }
      char* p = strchr( line+currentIndex, 'P' );
      Serial.print("p:");
      Serial.println(p);
      delay( atoi( p + 1) );
      break;
    }
    case 28:{
      //原点に移動する
      if (verbose){
        Serial.println("G28 Move to Origin");
      }
      movePosition(0,0);
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
    case 92:{
      //Set Position
      if (verbose){
        Serial.println("G92 Set Position");
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
        movePosition(newPos.x, newPos.y);
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
void cmdM(char *line,int currentIndex){
  char buffer[ 64 ];
  
  //コマンド番号を取得
  getCmdNum(buffer,line,currentIndex);

  struct point newPos;
  newPos.x = 0.0;
  newPos.y = 0.0;

  switch ( atoi( buffer ) ){
    case 2:{
      laserPower(LOW);
      newPos.x = 0;
      newPos.y = 0;
      movePosition(newPos.x, newPos.y);
      break;
    }
    case 3:
    case 300:{
      char* indexS = strchr( line+currentIndex, 'S' );
      float Spos = atof( indexS + 1);
      if (Spos == 30) {
        laserPower(HIGH);
      }
      if (Spos == 50) {
        laserPower(LOW);
      }
      break;
    }
  default:
    Serial.print( "Command not recognized : M");
    Serial.println( buffer );
  }
}


//指定位置へ移動する
/*********************************
 * Draw a line from (x0;y0) to (x1;y1).
 * Bresenham algo from https://www.marginallyclever.com/blog/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
 * int (x1;y1) : Starting coordinates
 * int (x2;y2) : Ending coordinates
 **********************************/
void movePosition(float x1, float y1) {          
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
      myStepperX.step(sx);
      over+=dy;
      if (over>=dx) {
        over-=dx;
        myStepperY.step(sy);
      }
      delay(StepDelay);
    }
  }
  else {
    for (i=0; i<dy; ++i) {
      myStepperY.step(sy);
      over+=dx;
      if (over>=dy) {
        over-=dy;
        myStepperX.step(sx);
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

// laser の On,Off
void laserPower(uint8_t value) {
  digitalWrite(penLaserPin,value);
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
