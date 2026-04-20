//Labo 3 : Michaël Allard : 6268332
#include <LCD_I2C.h>

const unsigned long STUDENT_ID = 6268332;
const String LAST_NAME = "Allard";

const int THERMO_PIN = A0;
const int JOY_X_PIN = A1;
const int JOY_Y_PIN = A2;
const int JOY_BTN_PIN = 2;
const int LED_PIN = 8;

const unsigned long BOUNCE_DELAY = 200;
const unsigned long SERIAL_INTERVAL = 100;
const unsigned long LCD_INTERVAL = 250;
const unsigned long ALT_INTERVAL = 1000;
const unsigned long STARTUP_DELAY = 3000;

const int JOY_DEADZONE = 100;
const int JOY_CENTER = 512;

const float THERMO_RE = 10000.0;
const float CO_1 = 1.129148e-03;
const float CO_2 = 2.34125e-04;
const float CO_3 = 8.76741e-08;

const float MAX_ALT = 200.0;
const float TEMP_COOL_ON = 20.0;
const float TEMP_COOL_OFF = 18.0;

enum LcdScreen {
  PAGE_JOYSTICK,
  PAGE_COOL_SYS
};

byte customChar[8] = {
  0b11000,
  0b00100,
  0b11100,
  0b00111,
  0b11001,
  0b00111,
  0b00100,
  0b00111
};

LCD_I2C lcd(0x27, 16, 2);

static LcdScreen currentPage = PAGE_JOYSTICK;
static bool startupDone = false;
static bool systemOn = false;

static float altitude = 0.0;
static int angleDir = 0;
static int rawX = JOY_CENTER;
static int rawY = JOY_BTN_PIN;
static float temperature = 0.0;

static int lastBtnState = HIGH;
static int btnState = HIGH;

static unsigned long lastBtnChange = 0;
static unsigned long lastSerial = 0;
static unsigned long lastLdc = 0;
static unsigned long lastAlt = 0;
static unsigned long startupTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);

  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, customChar);

  displayStartup();
  startupTime = millis();
}

void loop() {
  if(!startupDone) {
    if(millis() - startupTime >= STARTUP_DELAY) {
      startupDone = true;
      lcd.clear();
    }
    return;
  }
  
  joystickManagement();
  altitudeManagement();
  thermoManagement();
  buttonManagement();
  lcdManagement();
  serialManagement();
}

void displayStartup() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(LAST_NAME);

  String id = String(STUDENT_ID);
  String masked = "";

  for(int i = 0; i < id.length() - 2; i++) masked += "*";
  masked += id.substring(id.length() - 2);

  lcd.setCursor(0, 1);
  lcd.write(byte(0));

  int col = 16 - masked.length();
  lcd.setCursor(col, 1);
  lcd.print(masked);
}

void joystickManagement() {
  rawX = analogRead(JOY_X_PIN);
  rawY = analogRead(JOY_Y_PIN);
  angleDir = map(rawX, 0, 1023, -90, 90);
}

void altitudeManagement() {
  if(millis() - lastAlt < ALT_INTERVAL) return;

  float deltaTime = (millis() - lastAlt) / 1000.0;
  lastAlt = millis();

  if(rawY < JOY_CENTER - JOY_DEADZONE) {
    altitude = min(altitude + 1.0 * deltaTime, MAX_ALT);
  }
  else if(rawY > JOY_CENTER + JOY_DEADZONE) {
    altitude = max(altitude - 1.0 * deltaTime, 0.0);
  }
}

void thermoManagement() {
  int raw = analogRead(THERMO_PIN);

  float resistance = (THERMO_RE * raw) / (1023.0 - raw + 0.001);
  float logRe = log(resistance);
  float tempRaw = 1.0 / (CO_1 + CO_2 * logRe + CO_3 * pow(logRe, 3));
  
  temperature = tempRaw - 273.15;

  if(temperature > TEMP_COOL_ON) systemOn = true;
  if(temperature < TEMP_COOL_OFF) systemOn = false;

  digitalWrite(LED_PIN, systemOn ? HIGH : LOW);
}

void buttonManagement() {
  int currentBtnState = digitalRead(JOY_BTN_PIN);

  if(currentBtnState != lastBtnState) lastBtnChange = millis();

  if((millis() - lastBtnChange) > BOUNCE_DELAY && currentBtnState != btnState) {
    btnState = currentBtnState;
    if(currentBtnState == LOW) {
      currentPage = (currentPage == PAGE_JOYSTICK) ? PAGE_COOL_SYS : PAGE_JOYSTICK;
      lcd.clear();
    }
  }

  lastBtnState = currentBtnState;
}

void lcdManagement() {
  if(millis() - lastLdc < LCD_INTERVAL) return;
  lastLdc = millis();

  switch (currentPage) {
    case PAGE_JOYSTICK: lcdPageJoystick(); break;
    case PAGE_COOL_SYS: lcdPageCoolingSystem(); break; 
  }
}

void lcdPageJoystick() {
  String direction;

  if(rawY < JOY_CENTER - JOY_DEADZONE) direction = "UP";
  else if(rawY > JOY_CENTER + JOY_DEADZONE) direction = "DOWN";
  else direction = "";

  lcdPrint("ALT:" + String(altitude) + "m " + direction, 0);
  lcdPrint("DIR:" + String(angleDir) + "(" + String(angleDir < 0 ? "G" : "D") + ")", 1);
}

void lcdPageCoolingSystem() {
  lcdPrint(String(temperature) + " C", 0);
  lcdPrint("COOL: " + String(systemOn ? "ON" : "OFF"), 1);
}

void lcdPrint(String text, int row) {
  while(text.length() < 16) text += " ";
  lcd.setCursor(0, row);
  lcd.print(text);
}

void serialManagement() {
  if(millis() - lastSerial < SERIAL_INTERVAL) return;
  lastSerial = millis();

  Serial.print("etd:" + String(STUDENT_ID) + ",x:" + String(rawX) + ",y:" + String(rawY) + ",sys:");
  Serial.println(systemOn ? 1 : 0);
}