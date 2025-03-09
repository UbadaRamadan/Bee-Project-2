#define directionPin_x 2
#define stepPin_x 3
#define directionPin_y 22
#define stepPin_y 23

const unsigned long SPEEDS[] = {200, 100, 50}; // Slow, Medium, Fast (µs)
enum SpeedState { SLOW, MEDIUM, FAST };
SpeedState currentSpeed = MEDIUM;
unsigned long stepInterval = SPEEDS[currentSpeed];

unsigned long lastStepTimeX = 0;
unsigned long lastStepTimeY = 0;
bool xMoving = false, yMoving = false;
bool xDir = HIGH, yDir = HIGH;

void setup() {
  pinMode(directionPin_x, OUTPUT);
  pinMode(stepPin_x, OUTPUT);
  pinMode(directionPin_y, OUTPUT);
  pinMode(stepPin_y, OUTPUT);
  Serial.begin(1000000);
}

void loop() {
  unsigned long currentMicros = micros();

  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'U': 
        xDir = HIGH; 
        xMoving = true; 
        lastStepTimeX = currentMicros; // Reset X timer on movement start
        break;
      case 'D': 
        xDir = LOW;  
        xMoving = true; 
        lastStepTimeX = currentMicros; 
        break;
      case 'R': 
        yDir = HIGH; 
        yMoving = true; 
        lastStepTimeY = currentMicros; // Reset Y timer on movement start
        break;
      case 'L': 
        yDir = LOW;  
        yMoving = true; 
        lastStepTimeY = currentMicros; 
        break;
      case 'S': 
        xMoving = false; 
        yMoving = false; 
        break;
      case 'F': 
        if (currentSpeed < FAST) currentSpeed = (SpeedState)(currentSpeed + 1);
        stepInterval = SPEEDS[currentSpeed];
        break;
      case 'W': 
        if (currentSpeed > SLOW) currentSpeed = (SpeedState)(currentSpeed - 1);
        stepInterval = SPEEDS[currentSpeed];
        break;
    }
    digitalWrite(directionPin_x, xDir);
    digitalWrite(directionPin_y, yDir);
  }

  // X-axis stepping
  if (xMoving && (currentMicros - lastStepTimeX >= stepInterval)) {
    digitalWrite(stepPin_x, !digitalRead(stepPin_x));
    lastStepTimeX = currentMicros;
  }

  // Y-axis stepping
  if (yMoving && (currentMicros - lastStepTimeY >= stepInterval)) {
    digitalWrite(stepPin_y, !digitalRead(stepPin_y));
    lastStepTimeY = currentMicros;
  }
}
