#include <Servo.h>
#include <LCD_I2C.h>
#include <HCSR04.h>

const int PIN_SERVO = 8;
const int PIN_TRIG = 9;
const int PIN_ECHO = 10;
const int PIN_BTN_OPEN = 2;
const int PIN_BTN_EMERGENCY = 4;

const int DISTANCE_THRESHOLD = 20;
const int POS_CLOSE = 0;
const int POS_START = 10;
const int POS_OPEN = 170;
const unsigned long WAITING_TIME = 10000;
const unsigned long STEP_DELAY = 15;
const unsigned long BOUNCE_DELAY = 200;
const unsigned long LCD_INTERVAL = 250;
const unsigned long SENSOR_INTERVAL = 100;

enum State {
  CLOSE,
  OPEN,
  OPENING,
  CLOSING,
  EMERGENCY
};

Servo servo;
LCD_I2C lcd(0x27, 16, 2);
HCSR04 distanceSensor(PIN_TRIG, PIN_ECHO);

static State currentState = CLOSE;
static int currentPosition = POS_CLOSE;
static int emergencyPosition = POS_CLOSE;

static long distance = 999;

static unsigned long lastWaitingTime = 0;
static unsigned long lastServoStep = 0;
static unsigned long lastLcd = 0;
static unsigned long lastSensor = 0;

static int lastBtnOpenState = HIGH;
static int btnOpenState = HIGH;
static unsigned long btnOpenLastBounce = 0;
static bool openPressed = false;

static int lastBtnEmergencyState = HIGH;
static int btnEmergencyState = HIGH;
static unsigned long btnEmergencyLastBounce = 0;
static bool emergencyPressed = false;

static bool lcdRefreshNeeded = false;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_BTN_OPEN, INPUT_PULLUP);
  pinMode(PIN_BTN_EMERGENCY, INPUT_PULLUP);

  servo.attach(PIN_SERVO);
  servo.write(POS_CLOSE);

  lcd.begin();
  lcd.backlight();

  lcdRefreshNeeded = true;
}

void loop() {
  sensorManagement();
  buttonsManagement();
  doorManagement();
  servoManagement();
  lcdManagement();
}

void sensorManagement() {
  if(millis() - lastSensor < SENSOR_INTERVAL) return;
  lastSensor = millis();

  float read = distanceSensor.dist();
  distance = (read == 0) ? 999 : read;
}

void buttonsManagement() {
  int currentOpen = digitalRead(PIN_BTN_OPEN);

  if(currentOpen != lastBtnOpenState) btnOpenLastBounce = millis();
  if((millis() - btnOpenLastBounce) > BOUNCE_DELAY && currentOpen != btnOpenState) {
    btnOpenState = currentOpen;
    if(currentOpen == LOW) openPressed = true;
  }
  lastBtnOpenState = currentOpen;

  int currentEmergency = digitalRead(PIN_BTN_EMERGENCY);

  if(currentEmergency != lastBtnEmergencyState) btnEmergencyLastBounce = millis();
  if((millis() - btnEmergencyLastBounce) > BOUNCE_DELAY && currentEmergency != btnEmergencyState) {
    btnEmergencyState = currentEmergency;
    if(currentEmergency == LOW) emergencyPressed = true;
  }
  lastBtnEmergencyState = currentEmergency;
}

void doorManagement() {
  bool isDetect = (distance > 0 && distance <= DISTANCE_THRESHOLD);

  if(emergencyPressed && currentState != EMERGENCY) {
    emergencyPressed  = false;
    openPressed       = false;
    emergencyPosition = currentPosition;
    switchState(EMERGENCY);
    return;
  }

  switch(currentState) {
    case CLOSE:
      if(openPressed || isDetect) {
        openPressed     = false;
        currentPosition = POS_START;
        switchState(OPENING);
      }
      break;

    case OPENING:
      openPressed = false;
      break;

    case OPEN:
      openPressed = false;
      if(millis() - lastWaitingTime >= WAITING_TIME) {
        switchState(CLOSING);
      }
      break;

    case CLOSING:
      if(openPressed) {
        openPressed = false;
        switchState(OPENING);
      }
      break;

    case EMERGENCY:
      if(emergencyPressed) {
        emergencyPressed = false;
        currentPosition  = emergencyPosition;
        lastServoStep    = millis();
        switchState(CLOSING);
      }
      break;
  }

  openPressed = false;
}

void servoManagement() {
  if(currentState == EMERGENCY || currentState == CLOSE || currentState == OPEN) return;
  if(millis() - lastServoStep < STEP_DELAY) return;

  lastServoStep = millis();

  if(currentState == OPENING) {
    if(currentPosition < POS_OPEN) {
      currentPosition++;
      servo.write(currentPosition);
    }
    else {
      lastWaitingTime = millis();
      switchState(OPEN);
    }
  }

  if(currentState == CLOSING) {
    if(currentPosition > POS_CLOSE) {
      currentPosition--;
      servo.write(currentPosition);
    }
    else {
      switchState(CLOSE);
    }
  }
}

void lcdManagement() {
  if(!lcdRefreshNeeded && millis() - lastLcd < LCD_INTERVAL) return;

  lastLcd = millis();
  lcdRefreshNeeded = false;

  switch(currentState) {
    case CLOSE:
      lcdPrint("FERME", 0);
      lcdPrint("", 1);
      break;

    case OPENING:
      lcdPrint("OUVERTURE...", 0);
      lcdPrint("", 1);
      break;

    case OPEN:
      lcdPrint("OUVERT", 0);
      lcdPrint("", 1);
      break;

    case CLOSING:
      lcdPrint("FERMETURE...", 0);
      lcdPrint("", 1);
      break;

    case EMERGENCY:
      lcdPrint("!! URGENCE !!", 0);
      lcdPrint("En attente...", 1);
      break;
  }
}

void switchState(State newState) {
  currentState = newState;
  lcdRefreshNeeded = true;

  Serial.print("Etat : ");
  Serial.println(currentState);
}

void lcdPrint(String text, int row) {
  while ((int)text.length() < 16) text += " ";
  lcd.setCursor(0, row);
  lcd.print(text.substring(0, 16));
}