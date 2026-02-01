const int LED_PIN = 13;
const long STUDENT_ID = 6228332;

enum AppState{
  ON_OFF_STATE,
  FADE_STATE,
  BLINK_STATE
};

AppState appCurrentState = ON_OFF_STATE;

void setup(){
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop(){

  switch(appCurrentState){
    case ON_OFF_STATE:
      onoffState();
      break;
    case FADE_STATE:
      fadeState();
      break;
    case BLINK_STATE:
      blinkState();
      break;
  }
}

void onoffState(){
  Serial.print("Etat : Allume – ");
  Serial.println(STUDENT_ID);

  digitalWrite(LED_PIN, LOW);
  delay(300);

  digitalWrite(LED_PIN, HIGH);
  delay(2000);

  digitalWrite(LED_PIN, LOW);
  delay(1000);

  appCurrentState = FADE_STATE;
}

void fadeState(){
  Serial.print("Etat : Varie – ");
  Serial.println(STUDENT_ID);

  int forloopDelay = 2048 / 256;

  for(int i = 0;  i <= 255;  i++){
    analogWrite(LED_PIN, i);
    delay(forloopDelay);
  }
  
  appCurrentState = BLINK_STATE;
}

void blinkState(){
  Serial.print("Etat : Clignotement – ");
  Serial.println(STUDENT_ID);

  int penultimate = (STUDENT_ID / 10) % 10;

  if(penultimate == 0){
    penultimate = 10;
  }

  int numberOfBlinks = (penultimate + 1) / 2;

  for(int i = 0; i <= numberOfBlinks; i++){
    digitalWrite(LED_PIN, HIGH);
    delay(350);

    digitalWrite(LED_PIN, LOW);
    delay(350);
  }

  appCurrentState = ON_OFF_STATE;
}