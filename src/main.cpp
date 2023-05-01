#include <M5Core2.h>
#include <M5Touch.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#define NUM_EVENTS    8
#define TE_TOUCH    0x0001
#define TE_RELEASE    0x0002
#define TE_MOVE     0x0004
#define TE_GESTURE    0x0008
#define TE_TAP        0x0010
#define TE_DBLTAP    0x0020
#define TE_DRAGGED    0x0040
#define TE_PRESSED    0x0080
#define TE_ALL        0x0FFF
#define TE_BTNONLY    0x1000

//Method declarations
void swiped(Event& e);
void pinched(Event& e);
void drawSquares();
void rotateRow(uint32_t Side1[3][3], uint32_t Side2[3][3], uint32_t Side3[3][3], uint32_t Side4[3][3], int RowNumber);
void rotateColumn(uint32_t Side1[3][3], uint32_t Side2[3][3], uint32_t Side3[3][3], uint32_t Side4[3][3], int ColNumber);
void drawUnfolded();

//Variable declarations
enum Rows {Row_BOTTOM, Row_MID, Row_TOP};
Rows currentRow = Row_BOTTOM; //init

enum Column {Col_LEFT, Col_MID, Col_RIGHT};
Column currentColumn = Col_LEFT; //init

enum state {S_Splash, S_Cube, S_Scramble};
state screenState = S_Splash; //init

 uint32_t SideOne[3][3] = {
    {RED, RED, RED},
    {RED, RED, RED},
    {RED, RED, RED}
  };

 uint32_t SideTwo[3][3] = {
    {ORANGE, ORANGE, ORANGE},
    {ORANGE, ORANGE, ORANGE},
    {ORANGE, ORANGE, ORANGE}
  };

 uint32_t SideThree[3][3] = {
    {YELLOW, YELLOW, YELLOW},
    {YELLOW, YELLOW, YELLOW},
    {YELLOW, YELLOW, YELLOW}
  };
  
 uint32_t SideFour[3][3] = {
    {BLUE, BLUE, BLUE},
    {BLUE, BLUE, BLUE},
    {BLUE, BLUE, BLUE}
  };
  
 uint32_t SideFive[3][3] = {
    {GREEN, GREEN, GREEN},
    {GREEN, GREEN, GREEN},
    {GREEN, GREEN, GREEN}
  };

 uint32_t SideFRONT[3][3] = {
    {WHITE, WHITE, WHITE},
    {WHITE, WHITE, WHITE},
    {WHITE, WHITE, WHITE}
  };

//Zones
Zone topZone(0,0,320,80);
//Zone midZone(0, 80, 320, 160);  //idk why but when this is active, it overrules bottomZone...instead, just refer to this area as !top && !bottom
Zone bottomZone(0, 165, 320, 240);
Zone leftZone(0, 0, 106, 240);
Zone centerZone(107, 0, 106, 240);
Zone rightZone(214, 0, 106, 240);
Zone topLeftCornerZone(0,0,160,120);
Zone bottomRightCornerZone(160,120,160,120);

//Gestures
//_ = "", uint16_t minDistance_ = (uint16_t)75U, int16_t direction_ = (int16_t)(-32768), uint8_t plusminus_ = (uint8_t)45U, bool rot1_ = false, uint16_t maxTime_ = (uint16_t)500U)
Gesture swipeRight("Swipe Right", 75, 90, 45, true, 500);
Gesture swipeUp("Swipe Up", 75, 0, 45, true, 500);
Gesture swipeDown("Swipe Down", 75, 180, 45, true, 500);
Gesture swipeLeft("Swipe Left", 75, 270, 45, true, 500);
Gesture pinch(topLeftCornerZone, bottomRightCornerZone, "pinch");

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

#define OUTPUT_GAIN 100

void setup() {
  M5.begin();
  M5.Axp.SetSpkEnable(true);

  WiFi.mode(WIFI_OFF); 
  delay(500);
  
  M5.Lcd.setTextFont(2);
  Serial.printf("Peaches by Jack Black is playing!\n");

  // pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html
  file = new AudioFileSourceSD("/Peaches.mp3");
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(1, 0); // Output to builtInDAC
  out->SetPinout(12, 0, 2);
  out->SetOutputModeMono(true);
  out->SetGain((float)OUTPUT_GAIN/100.0);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);

  swipeRight.addHandler(swiped);
  swipeUp.addHandler(swiped);
  swipeDown.addHandler(swiped);
  swipeLeft.addHandler(swiped);
  pinch.addHandler(pinched);
  drawSquares();
}

void loop() {
  M5.update();

  //check if the mp3 file is running or if it finished
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("Peaches by Jack Black has finished playing!\n");
    delay(1000);
  }

  //check for buttonpress
  if (M5.BtnA.wasPressed()) {
    drawUnfolded();
    delay(5000);
    drawSquares();
  }
}

void pinched(Event& e) {
  if (e.type == TE_GESTURE && e.gesture == &pinch) {
    drawUnfolded();
  }
}

void swiped(Event& e) {
  if (e.type == TE_GESTURE) {
    //direction check, then zone check
    //RIGHT
    if (e.gesture == &swipeRight) {
      Serial.println("\n--- SWIPE RIGHT DETECTED ---");
      //so right, but which row is rotating?
      if(e.from.in(topZone)) {
        rotateRow(SideFive, SideFRONT, SideThree, SideOne, 0);//top row, so 0th Row Number
        drawSquares();
      } 
      else if(e.from.in(bottomZone)) {
        rotateRow(SideFive, SideFRONT, SideThree, SideOne,2);//bottom row, so 2nd Row Number
        drawSquares();
      }
      else {
        rotateRow(SideFive, SideFRONT, SideThree, SideOne,1);//mid row, so 1st Row Number
        drawSquares();
      }
    }
    //LEFT
    if (e.gesture == &swipeLeft) {
      Serial.println("\n--- SWIPE LEFT DETECTED ---");
      //so left, but which row is rotating?
      if(e.from.in(topZone)) {
        rotateRow(SideThree, SideFRONT, SideFive, SideOne,0);//top row, so 0th Row Number
        drawSquares();
      } 
      else if(e.from.in(bottomZone)) {
        rotateRow(SideThree, SideFRONT, SideFive, SideOne,2);//bottom row, so 2nd Row Number
        drawSquares();
      }
      else {
        rotateRow(SideThree, SideFRONT, SideFive, SideOne,1);//middle row, so 1st Row Number
        drawSquares();
      }
    }
    //UP
    if (e.gesture == &swipeUp) {
      Serial.println("\n--- SWIPE UP DETECTED ---");
      //so up, but which column is rotating?
      if(e.from.in(leftZone)) {
        rotateColumn(SideOne, SideTwo, SideFRONT, SideFour, 0);//left col, so 0th Col Number
        drawSquares();
      }
      else if(e.from.in(centerZone)) {
        rotateColumn(SideOne, SideTwo, SideFRONT, SideFour, 1);//mid col, so 1st Col Number
        drawSquares();
      }
      else if(e.from.in(rightZone)) {
        rotateColumn(SideOne, SideTwo, SideFRONT, SideFour, 2);//right col, so 2nd Col Number
        drawSquares();
      }
    }
    //DOWN
    if (e.gesture == &swipeDown) {
      Serial.println("\n--- SWIPE DOWN DETECTED ---");
      //so down, but which column is rotating?
      if(e.from.in(leftZone)) {
        rotateColumn(SideFour, SideFRONT, SideTwo, SideOne, 0);//left col, so 0th Col Number
        drawSquares();
      }
      else if(e.from.in(centerZone)) {
        rotateColumn(SideFour, SideFRONT, SideTwo, SideOne, 1);//mid col, so 1st Col Number
        drawSquares();
      }
      else if(e.from.in(rightZone)) {
        rotateColumn(SideFour, SideFRONT, SideTwo, SideOne, 2);//right col, so 2nd Col Number
        drawSquares();
      }
    }    
  }
}


void rotateRow(uint32_t Side1[3][3], uint32_t Side2[3][3], uint32_t Side3[3][3], uint32_t Side4[3][3], int RowNumber) {
    // Store the value of the first element of the row to be rotated
  uint32_t temp[3] = {Side1[RowNumber][0], Side1[RowNumber][1], Side1[RowNumber][2]};

  // Side 1[RN] = Side 2 [RN]
  Side1[RowNumber][0] = Side2[RowNumber][0];
  Side1[RowNumber][1] = Side2[RowNumber][1];
  Side1[RowNumber][2] = Side2[RowNumber][2];

  // Side 2[RN] = Side 3 [RN]
  Side2[RowNumber][0] = Side3[RowNumber][0];
  Side2[RowNumber][1] = Side3[RowNumber][1];
  Side2[RowNumber][2] = Side3[RowNumber][2];

  // Side 3[RN] = Side 4 [RN]
  Side3[RowNumber][0] = Side4[RowNumber][0];
  Side3[RowNumber][1] = Side4[RowNumber][1];
  Side3[RowNumber][2] = Side4[RowNumber][2];

  // Side 4[RN] = temp [RN]
  Side4[RowNumber][0] = temp[0];
  Side4[RowNumber][1] = temp[1];
  Side4[RowNumber][2] = temp[2];
}

void rotateColumn(uint32_t Side1[3][3], uint32_t Side2[3][3], uint32_t Side3[3][3], uint32_t Side4[3][3], int ColNumber) {
    // Store the value of the first element of the row to be rotated
  uint32_t temp[3] = {Side1[0][ColNumber], Side1[1][ColNumber], Side1[2][ColNumber]};

  // Side 1[RN] = Side 2 [RN]
  Side1[0][ColNumber] = Side2[0][ColNumber];
  Side1[1][ColNumber] = Side2[1][ColNumber];
  Side1[2][ColNumber] = Side2[2][ColNumber];

  // Side 2[RN] = Side 3 [RN]
  Side2[0][ColNumber] = Side3[0][ColNumber];
  Side2[1][ColNumber] = Side3[1][ColNumber];
  Side2[2][ColNumber] = Side3[2][ColNumber];
  
  // Side 2[RN] = Side 3 [RN]
  Side3[0][ColNumber] = Side4[0][ColNumber];
  Side3[1][ColNumber] = Side4[1][ColNumber];
  Side3[2][ColNumber] = Side4[2][ColNumber];

  // Side 3[RN] = temp [RN]
  Side4[0][ColNumber] = temp[0];
  Side4[1][ColNumber] = temp[1];
  Side4[2][ColNumber] = temp[2];
}

void drawSquares() {
  int32_t padding = 5;
  int32_t tileHeight = 73.3;
  int32_t tileWidth = 100;

  M5.Lcd.fillScreen(BLACK);

  //top row
  M5.Lcd.fillRect(padding, padding, tileWidth, tileHeight, SideFRONT[0][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth, padding, tileWidth, tileHeight, SideFRONT[0][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2), padding, tileWidth, tileHeight, SideFRONT[0][2]);
  
  //middle row
  M5.Lcd.fillRect(padding, (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth,  (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2),  (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][2]);
  
  //bottom row
  M5.Lcd.fillRect(padding, ((padding * 3) + (tileWidth * 1.5)), tileWidth, tileHeight, SideFRONT[2][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth, ((padding * 3) + (tileWidth * 1.5)), tileWidth, tileHeight, SideFRONT[2][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2), ((padding * 3) + (tileWidth * 1.5)), tileWidth, tileHeight, SideFRONT[2][2]);
  
}

void drawUnfolded() {
  int32_t padding = 5;
  int32_t tileHeight = 20;
  int32_t tileWidth = 20;

  M5.Lcd.fillScreen(BLACK);

  //FAR LEFT - Side1
  //top row
  M5.Lcd.fillRect(padding, 80 + padding, tileWidth, tileHeight, SideOne[0][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth, 80 + padding, tileWidth, tileHeight, SideOne[0][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2), 80 + padding, tileWidth, tileHeight, SideOne[0][2]);
  //middle row
  M5.Lcd.fillRect(padding, 80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideOne[1][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth,  80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideOne[1][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2), 80 +  (padding * 2) + tileHeight, tileWidth, tileHeight, SideOne[1][2]);
  //bottom row
  M5.Lcd.fillRect(padding, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideOne[2][0]);
  M5.Lcd.fillRect((padding * 2) + tileWidth, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideOne[2][1]);
  M5.Lcd.fillRect((padding * 3) + (tileWidth * 2), 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideOne[2][2]);

  //LEFT - Side3
  //top row
  M5.Lcd.fillRect(75 + padding, 80 + padding, tileWidth, tileHeight, SideThree[0][0]);
  M5.Lcd.fillRect(75 + (padding * 2) + tileWidth, 80 + padding, tileWidth, tileHeight, SideThree[0][1]);
  M5.Lcd.fillRect(75 + (padding * 3) + (tileWidth * 2), 80 + padding, tileWidth, tileHeight, SideThree[0][2]);
  //middle row
  M5.Lcd.fillRect(75 + padding, 80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideThree[1][0]);
  M5.Lcd.fillRect(75 + (padding * 2) + tileWidth,  80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideThree[1][1]);
  M5.Lcd.fillRect(75 + (padding * 3) + (tileWidth * 2), 80 +  (padding * 2) + tileHeight, tileWidth, tileHeight, SideThree[1][2]);
  //bottom row
  M5.Lcd.fillRect(75 + padding, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideThree[2][0]);
  M5.Lcd.fillRect(75 + (padding * 2) + tileWidth, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideThree[2][1]);
  M5.Lcd.fillRect(75 + (padding * 3) + (tileWidth * 2), 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideThree[2][2]);


  //FRONT
  //top row
  M5.Lcd.fillRect(150 + padding, 80 + padding, tileWidth, tileHeight, SideFRONT[0][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, 80 + padding, tileWidth, tileHeight, SideFRONT[0][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 80 + padding, tileWidth, tileHeight, SideFRONT[0][2]);
  //middle row
  M5.Lcd.fillRect(150 + padding, 80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth,  80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 80 +  (padding * 2) + tileHeight, tileWidth, tileHeight, SideFRONT[1][2]);
  //bottom row
  M5.Lcd.fillRect(150 + padding, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFRONT[2][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFRONT[2][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFRONT[2][2]);

  //RIGHT - Side5
  //top row
  M5.Lcd.fillRect(225 + padding, 80 + padding, tileWidth, tileHeight, SideFive[0][0]);
  M5.Lcd.fillRect(225 + (padding * 2) + tileWidth, 80 + padding, tileWidth, tileHeight, SideFive[0][1]);
  M5.Lcd.fillRect(225 + (padding * 3) + (tileWidth * 2), 80 + padding, tileWidth, tileHeight, SideFive[0][2]);
  //middle row
  M5.Lcd.fillRect(225 + padding, 80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFive[1][0]);
  M5.Lcd.fillRect(225 + (padding * 2) + tileWidth,  80 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFive[1][1]);
  M5.Lcd.fillRect(225 + (padding * 3) + (tileWidth * 2), 80 +  (padding * 2) + tileHeight, tileWidth, tileHeight, SideFive[1][2]);
  //bottom row
  M5.Lcd.fillRect(225 + padding, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFive[2][0]);
  M5.Lcd.fillRect(225 + (padding * 2) + tileWidth, 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFive[2][1]);
  M5.Lcd.fillRect(225 + (padding * 3) + (tileWidth * 2), 80 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFive[2][2]);

  //TOP - Side2
  //top row
  M5.Lcd.fillRect(150 + padding, padding, tileWidth, tileHeight, SideTwo[0][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, padding, tileWidth, tileHeight, SideTwo[0][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), padding, tileWidth, tileHeight, SideTwo[0][2]);
  //middle row
  M5.Lcd.fillRect(150 + padding, (padding * 2) + tileHeight, tileWidth, tileHeight, SideTwo[1][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, (padding * 2) + tileHeight, tileWidth, tileHeight, SideTwo[1][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), (padding * 2) + tileHeight, tileWidth, tileHeight, SideTwo[1][2]);
  //bottom row
  M5.Lcd.fillRect(150 + padding,  ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideTwo[2][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth,  ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideTwo[2][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2),  ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideTwo[2][2]);

  //BOTTOM - Side4
  //top row
  M5.Lcd.fillRect(150 + padding, 160 + padding, tileWidth, tileHeight, SideFour[0][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, 160 + padding, tileWidth, tileHeight, SideFour[0][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 160 + padding, tileWidth, tileHeight, SideFour[0][2]);
  //middle row
  M5.Lcd.fillRect(150 + padding, 160 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFour[1][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth,  160 + (padding * 2) + tileHeight, tileWidth, tileHeight, SideFour[1][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 160 +  (padding * 2) + tileHeight, tileWidth, tileHeight, SideFour[1][2]);
  //bottom row
  M5.Lcd.fillRect(150 + padding, 160 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFour[2][0]);
  M5.Lcd.fillRect(150 + (padding * 2) + tileWidth, 160 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFour[2][1]);
  M5.Lcd.fillRect(150 + (padding * 3) + (tileWidth * 2), 160 + ((padding * 3) + (tileWidth * 2)), tileWidth, tileHeight, SideFour[2][2]);
}
