#define THRESHOLD 900

const int Kp = 200;
const int Kd = 130;

int rawSensors[] = {0,0,0,0,0,0,0,0};
int ledOutput[] = {2,3,4,5,6,7,8,9};
bool done = 0;
int lastSeen = 0;

/*
Interface to L298N module

Arduino | L298N Module
-----------------------
  D2          IN1
  D3          IN2
  D4          IN3
  D5          IN4
  D9          ENA
  D10         ENB

*/
int motor1pin1 = 2;
int motor1pin2 = 3;

int motor2pin1 = 4;
int motor2pin2 = 5;

float errorWeights[] = {-3, -2, -1, -0.1, 0.1, 1, 2, 3};

const int baseSpeed = 150;
int previousError = 0;

void setup() {
  Serial.begin(9600);

  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1,  OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  // analog output for motor speed
  pinMode(9,  OUTPUT); 
  pinMode(10, OUTPUT);

  // configured to move forward - always
  digitalWrite(motor1pin1,  HIGH);
  digitalWrite(motor1pin2, LOW);

  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);

  // delay(3000);
}

void loop() {
  // sense environment
  sense();
  // move 
  move();
  delay(10);
}

// read raw photo transistor data 
void sense(){
  rawSensors[0] = analogRead(A7);
  rawSensors[1] = analogRead(A6);
  rawSensors[2] = analogRead(A5);
  rawSensors[3] = analogRead(A4);
  rawSensors[4] = analogRead(A3);
  rawSensors[5] = analogRead(A2);
  rawSensors[6] = analogRead(A1);
  rawSensors[7] = analogRead(A0);

  // output data for sensors
  for (int i = 0; i < 8; i++){
    Serial.print(rawSensors[i]);Serial.print(",");
  }
  Serial.println("");


}

void move(){

  if (!done){

    // analyze sensors
    int cnt = 0;
    float error = 0;
    for (int i = 0; i < 8; i++){
      // Dark line is detected! 
      if (rawSensors[i] < THRESHOLD){
        error += errorWeights[i];
        cnt += 1;
      }
    }

    if (cnt == 0) {
      Serial.print("CANT SEE! ");
    // Line lost — recover based on last seen direction
      if (lastSeen == 1) {
        // Spin left
        analogWrite(9, 0);
        analogWrite(10, baseSpeed);
        Serial.println("going left");
      } else {
        // Spin right
        analogWrite(9, baseSpeed);
        analogWrite(10, 0);
        Serial.println("going right");
      }
    } 
    // reached the end (all leds detect dark color)
    else if (cnt == 8){
      done = 1;
      return;

    }
    // perform PID controller
    else{
      int derivative = error - previousError;
      int correction = (Kp * error) + (Kd * derivative);
      previousError = error;

      if (error < 0) {
        lastSeen = -1; // more black on left
      } else if (error > 0) {
        lastSeen = 1;  // more black on right
      }

      int leftSpeed = constrain(baseSpeed - correction, 0, 255);
      int rightSpeed = constrain(baseSpeed + correction, 0, 255);

      Serial.print(correction);Serial.print(",");Serial.print(leftSpeed);Serial.print(",");Serial.println(rightSpeed);
      analogWrite(9, leftSpeed); //ENA  pin
      analogWrite(10, rightSpeed); //ENB pin
    }
  }
  // don't move
  else{
    analogWrite(9, 0); // ENA  pin
    analogWrite(10, 0); //ENB pin

  }

}
