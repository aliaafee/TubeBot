#include <Servo.h>

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <StateMachine.h>

#define TFT_CS A5
#define TFT_DC A3
#define TFT_RESET A4
// MISO->12, LED->3v3, SCK->13, MOSI->11, GND->GND, VCC->5V
// To use with 5v logic short J1

#define TS_CS A2
#define TS_IRQ 2
// T_DO->12, T_DIN->11, T_CLK->13

//TS Calibration
#define TS_CAL_X1 177
#define TS_CAL_X2 3860
#define TS_CAL_Y1 93
#define TS_CAL_Y2 3737


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

XPT2046_Touchscreen ts(TS_CS, TS_IRQ);

Servo rightArm;
Servo leftArm;

#define RIGHT_ARM_SERVO 3
#define LEFT_ARM_SERVO 5

#define POKE_BUTTON 7

String currentCommand = "";

StateMachine tubeBot = StateMachine();

State* stateSleep = tubeBot.addState(&funcSleep);
State* stateAwake = tubeBot.addState(&funcAwake);
State* stateGreet = tubeBot.addState(&funcGreet);
State* stateDance = tubeBot.addState(&funcDance);

String readSerialIfAvailable() {
  if (Serial.available() > 0) {
    String it = Serial.readString();
    it.trim();
    Serial.print("." + it + ".");
    return it;
  }
  return "";
}

void funcSleep() {
  if (tubeBot.executeOnce) {
    clearEyes();
    drawEyesClosed();
    clearMouth();
    drawMouthNeutral();
    setStateText("Zzz...");
  }

  if (millis() % 6000 == 0) {
    setStateText("Zzz...ZZz");
  }

  if (millis() % 8000 == 0) {
    setStateText("zzZ...zZz");
  }

  if (readSerialIfAvailable() == "poke") {
    tubeBot.transitionTo(stateAwake);
  }
}

unsigned long awakeTime = 0;
unsigned long nextGreetIn = 2000;
String userName = "Ali";
void funcAwake() {
  if (tubeBot.executeOnce) {
    clearEyeBrows();
    drawEyeBrowsNeutral();
    drawEyesOpen();
    clearMouth();
    drawMouthSmile();
    if (userName != "") {
      //setStateText("Hello " + userName);
      setStateText("");
    } else {
      setStateText("");
    }
    awakeTime = millis();
  }

  if (userName == "") {
    if (millis() - awakeTime > nextGreetIn) {
      tubeBot.transitionTo(stateGreet);
    }
  }

  if (millis() % 5000 == 0) {
    clearEyes();
    drawEyesClosed();
    delay(100);
    drawEyesOpen(random(-1,2));
  }

  if (millis() % 8000 == 0) {
    clearEyeBrows();
    drawEyeBrowsWorried();
    clearMouth();
    drawMouthNeutral();
    delay(100);
    clearEyeBrows();
    drawEyeBrowsNeutral();
    clearMouth();
    drawMouthSmile();
  }

  String command = readSerialIfAvailable();

  if (command == "bye") {
    setStateText("Bye Bye "+ userName);
    delay(1000);
    tubeBot.transitionTo(stateSleep);
  }

  if (command == "dance") {
    tubeBot.transitionTo(stateDance);
  }
}

unsigned long greetTime = 0;
void funcGreet() {
  if (tubeBot.executeOnce) {
    clearMouth();
    drawMouthO();
    setStateText("Hello");
    delay(2000);
    setStateText("What's your name?");
    greetTime = millis();
  }
  userName = readSerialIfAvailable();
  if (userName != "") {
    raiseRightArm();
    delay(300);
    setStateText("Hello " + userName);
    delay(2000);
    lowerRightArm();
    tubeBot.transitionTo(stateAwake);
  } else {
    if (millis() - greetTime > 5000) {
      setStateText("You must be busy");
      delay(1000);
      setStateText("Talk later...");
      nextGreetIn = 20000;
      delay(1000);
      tubeBot.transitionTo(stateAwake);
    }
  }
}

unsigned long danceTime = 0;
void funcDance() {
  if (tubeBot.executeOnce) {
    danceTime = millis();
  }

  clearMouth();
  drawMouthSmile();
  rightArm.write(120);
  leftArm.write(120);
  delay(500);
  rightArm.write(60);
  leftArm.write(60);
  clearMouth();
  drawMouthO();
  delay(500);

  if (millis() - danceTime > 10000) {
    lowerRightArm();
    lowerLeftArm();
    tubeBot.transitionTo(stateAwake);
  }
  
  if (readSerialIfAvailable() == "stop") {
    lowerRightArm();
    lowerLeftArm();
    tubeBot.transitionTo(stateAwake);
  }
}

bool sleepToAwake() {
  int sw = HIGH;
  sw = digitalRead(POKE_BUTTON);
  if (sw == LOW) {
    return true;
  }
  if (currentCommand=="poke") {
    return true;
  }
  return false;
}

bool awakeToSleep() {

  if (currentCommand=="bye") {
    return true;
  }
  return false;
}


void drawFace() {
  drawEyesClosed();

  //Eye brows
  //drawEyeBrowsNeutral()
  drawEyeBrowsWorried();

  drawMouthNeutral();
}

void clearEyeBrows() {
  tft.fillRect(59, 25, 41, 11, ILI9341_BLACK);
  tft.fillRect(159, 25, 41, 11, ILI9341_BLACK);
}

void drawEyeBrowsNeutral() {
  tft.drawLine(60, 30, 80, 30, ILI9341_WHITE);
  tft.drawLine(160, 30, 180, 30, ILI9341_WHITE);
}

void drawEyeBrowsWorried() {
  tft.drawLine(60, 35, 80, 25, ILI9341_WHITE);
  tft.drawLine(160, 25, 180, 35, ILI9341_WHITE);
}

void clearEyes() {
  tft.fillCircle(70, 80, 30, ILI9341_BLACK);
  tft.fillCircle(170, 80, 30, ILI9341_BLACK);
}

void drawEyesOpen() {
  tft.fillCircle(70, 80, 30, ILI9341_WHITE);
  tft.fillCircle(170, 80, 30, ILI9341_WHITE);

  //Pupil
  tft.fillCircle(70, 90 , 10, ILI9341_BLUE);
  tft.fillCircle(170, 90 , 10, ILI9341_BLUE);
}

void drawEyesOpen(int eyesDirection) {
  tft.fillCircle(70, 80, 30, ILI9341_WHITE);
  tft.fillCircle(170, 80, 30, ILI9341_WHITE);

  //Pupil
  tft.fillCircle(70 + (eyesDirection * 10), 90 , 10, ILI9341_BLUE);
  tft.fillCircle(170 + (eyesDirection * 10), 90 , 10, ILI9341_BLUE);
}

void drawEyesClosed() {
  tft.drawLine(40, 80, 100, 80, ILI9341_WHITE);
  tft.drawLine(140, 80, 200, 80, ILI9341_WHITE);
}

void clearMouth() {
  tft.fillRect(59, 159, 122, 42, ILI9341_BLACK);
}

void drawMouthSmile() {
  tft.drawLine(60, 160,  120, 180, ILI9341_WHITE);
  tft.drawLine(180, 160, 120, 180, ILI9341_WHITE);
}

void drawMouthNeutral() {
  tft.drawLine(60, 180,  120, 180, ILI9341_WHITE);
  tft.drawLine(180, 180, 120, 180, ILI9341_WHITE);
}

void drawMouthO() {
  tft.drawCircle(120, 180, 20, ILI9341_WHITE);
}

void raiseRightArm() {
  rightArm.write(180);
}

void raiseLeftArm() {
  leftArm.write(0);
}

void lowerRightArm() {
  rightArm.write(0);
}

void lowerLeftArm() {
  leftArm.write(180);
}

unsigned long tftClear() {
  tft.fillScreen(ILI9341_BLACK);
}

int p_x = 0;
int p_y = 0;
String p_text = "";
void tftSetText(String text, int x, int y) {
  tft.setTextSize(2);

  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(p_x, p_y);
  tft.println(p_text);

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(x, y);
  tft.println(text);

  p_x = x;
  p_y = y;
  p_text = text;
}

void setStateText(String text) {
  tftSetText(text, 10, 230);
}


void setup() {
  //Initialize TFT pins 
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RESET, OUTPUT);
  pinMode(TFT_CS, OUTPUT);

  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_RESET, HIGH);
  digitalWrite(TFT_CS, HIGH);

  pinMode(TS_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  
  //Initialize Arm Servos
  rightArm.attach(RIGHT_ARM_SERVO);
  leftArm.attach(LEFT_ARM_SERVO);
  //Move to Down Position
  lowerRightArm();
  lowerLeftArm();

  //Start Serial Connection
  Serial.begin(115200);

  //Initialize state machine
  stateSleep->addTransition(&sleepToAwake, stateAwake);
  stateAwake->addTransition(&awakeToSleep, stateSleep);
  tubeBot.transitionTo(stateSleep);

  //Begin TFT
  tft.begin();
  tftClear();
  tft.setRotation(2);

  //Begin TS
  ts.begin();
  ts.setRotation(2);

  //draw Initial Face
  drawFace();

  //Setup Button
  pinMode(POKE_BUTTON, INPUT_PULLUP);
}

long minX = 100000 ;
long maxX = 0;
long minY = 100000;
long maxY = 0;

void loop() {
  tubeBot.run();
  delay(1);

  boolean istouched = ts.tirqTouched();
  if (istouched) {
    TS_Point p = ts.getPoint();
    //Serial.print("X=");
    //Serial.println(p.x);
    //Serial.print("Y=");
    //Serial.println(p.y);

    if (p.x < minX) { minX = p.x; }
    if (p.x > maxX) { maxX = p.x; }
    if (p.y < minY) { minY = p.y; }
    if (p.y > maxY) { maxY = p.y; }

    Serial.print("Min=");
    Serial.print(minX);
    Serial.print(",");
    Serial.print(minY);
    Serial.print("   ");
    Serial.print("Max=");
    Serial.print(maxX);
    Serial.print(",");
    Serial.print(maxY);
    Serial.println("");

    int subY = 100;
    int screenX = map(p.x, TS_CAL_X1, TS_CAL_X2, 0, 240);
    int screenY = map(p.y, TS_CAL_Y1, TS_CAL_Y2, 320, 0);
    
    tft.fillCircle(screenX, screenY, 3, ILI9341_WHITE);

    Serial.print(p.x);
    Serial.print(",");
    Serial.print(p.y);
    Serial.println("");

    Serial.print(screenX);
    Serial.print(",");
    Serial.print(screenY);
    Serial.println("");
    
    
  }
}
