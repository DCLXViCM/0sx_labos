const int LEDS_PIN[] = {7, 8, 9, 10};
const int BUTTON_PIN = 2;
const int POTENTIOMETER_PIN = A1;

const long STUDENT_ID = 6268332;
const int LAST_ID_NUMBER = STUDENT_ID % 10;

static int lastButtonState = HIGH;
static int buttonState = HIGH;
const int BOUNCE_DELAY = 50;
static unsigned long lastChange = 0;

void setup(){
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  for(int i = 0; i < 4; i++){
    pinMode(LEDS_PIN[i], OUTPUT);
  }
}

void loop(){
  int potentiometerValue = analogRead(POTENTIOMETER_PIN);
  int readingScale = map(potentiometerValue, 0, 1023, 0, 20);

  int state = scaleToState(readingScale);

  ledManagement(state);
  buttonManagement(state);
}

int scaleToState(int readingScale){
  int state;

  if(readingScale >= 0 && readingScale < 5) state = 1;
  else if(readingScale < 10) state = 2;
  else if(readingScale < 15) state = 3;
  else state = 4;

  return state;
}

void ledManagement(int state){
  int ledOutput;

  if(LAST_ID_NUMBER % 2 == 0){ // SI le dernier chiffre du numéros étudiant est pair
    for(int i = 0; i < 4; i++){
      if(i == state - 1) ledOutput = HIGH;
      else ledOutput = LOW;
      digitalWrite(LEDS_PIN[i], ledOutput);
    }
  }
  else{ // SINON
    for(int i = 0; i < 4; i++){
      if(i <= state - 1) ledOutput = HIGH;
      else ledOutput = LOW;
      digitalWrite(LEDS_PIN[i], ledOutput);
    }
  }
}

void buttonManagement(int state){
 int currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState != lastButtonState) lastChange = millis();

  if ((millis() - lastChange) > BOUNCE_DELAY && currentButtonState != buttonState) { 
    buttonState = currentButtonState;
    printManagement(state);
  }

  lastButtonState = currentButtonState;
}

void printManagement(int state){
  int percentage = state * 25;
  int length = state * 5;

  if(LAST_ID_NUMBER % 2 == 0){ // SI le dernier chiffre du numéros étudiant est pair
    Serial.print("[");
    
    for(int i = 0; i < 20; i++){
      if(i < length) Serial.print("!");
      else Serial.print(".");
    }

    if(state == 4) Serial.print("] ");
    else Serial.print("]  ");
    

    Serial.print(percentage);
    Serial.println("%");
  }
  else{ // SINON
    Serial.print(percentage);
    Serial.print("%");

    if(state == 4) Serial.print(" ");
    else Serial.print("  ");

    Serial.print("[");

    for(int i = 0; i < 20; i++){
      if(i < length) Serial.print("-");
      else Serial.print(".");
    }

    Serial.println("]");
  }
}