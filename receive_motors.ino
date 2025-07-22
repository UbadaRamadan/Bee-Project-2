#define directionPin_y 24
#define stepPin_y 25
#define directionPin_x 22
#define stepPin_x 23

const unsigned long SPEEDS[] = {200, 100, 50}; // Slow, Medium, Fast (µs)
enum SpeedState { SLOW, MEDIUM, FAST };
SpeedState currentSpeed = MEDIUM;
unsigned long stepInterval = SPEEDS[currentSpeed];

// Position tracking
int xStepCount = 0, yStepCount = 0;
const unsigned long xStepLimit = 10000, yStepLimit = 2000;
int startX = 0, startY = 0; // Will store the actual starting position

// Movement control
unsigned long lastStepTimeX = 0;
unsigned long lastStepTimeY = 0;
bool xMoving = false, yMoving = false;
bool xDir = HIGH, yDir = HIGH;
bool xContinuous = false, yContinuous = false;
bool homingInProgress = false;
bool isFirstRun = true; // To capture initial position

void stepMotorX(int Steps) {
  if (Steps > 0) {
    xDir = HIGH;
    xMoving = true;
    xStepCount += 1;
  }
  else if (Steps < 0) {
    xDir = LOW;
    xMoving = true;
    xStepCount -= 1;
  }
}

void stepMotorY(int Steps) {
  if (Steps > 0) {
    yDir = HIGH;
    yMoving = true;
    yStepCount += 1;
  }
  else if (Steps < 0) {
    yDir = LOW;
    yMoving = true;
    yStepCount -= 1;
  }
}

void setStartPosition() {
  if (isFirstRun) {
    startX = xStepCount;
    startY = yStepCount;
    isFirstRun = false;
    Serial.print("Start position set to: (");
    Serial.print(startX);
    Serial.print(", ");
    Serial.print(startY);
    Serial.println(")");
  }
}

void home() {
  setStartPosition(); // Ensure start position is set
  
  if (xStepCount == startX && yStepCount == startY) {
    Serial.println("Already at start position");
    homingInProgress = false;
    return;
  }
  
  homingInProgress = true;
  
  // Set X direction
  if (xStepCount > startX) {
    xDir = LOW;  // Move towards lower values
    xMoving = true;
    xContinuous = true;
  } 
  else if (xStepCount < startX) {
    xDir = HIGH; // Move towards higher values
    xMoving = true;
    xContinuous = true;
  }

  // Set Y direction
  if (yStepCount > startY) {
    yDir = LOW;  // Move towards lower values
    yMoving = true;
    yContinuous = true;
  }
  else if (yStepCount < startY) {
    yDir = HIGH; // Move towards higher values
    yMoving = true;
    yContinuous = true;
  }
}

void stopAll() {
  xMoving = false;
  yMoving = false;
  xContinuous = false;
  yContinuous = false;
  homingInProgress = false;
}

void setup() {
  pinMode(directionPin_x, OUTPUT);
  pinMode(stepPin_x, OUTPUT);
  pinMode(directionPin_y, OUTPUT);
  pinMode(stepPin_y, OUTPUT);
  Serial.begin(1000000);
  digitalWrite(directionPin_x, xDir);
  digitalWrite(directionPin_y, yDir);
  
  // Initialize start position to wherever we are when powered on
  startX = xStepCount;
  startY = yStepCount;
  Serial.print("Initial start position: (");
  Serial.print(startX);
  Serial.print(", ");
  Serial.print(startY);
  Serial.println(")");
}

void loop() {
  unsigned long currentMicros = micros();

  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case 'U': // Toggle X positive
        xContinuous = !xContinuous;
        if (xContinuous) {
          stepMotorX(1);
        } else {
          xMoving = false;
        }
        lastStepTimeX = currentMicros;
        break;
        
      case 'D': // Toggle X negative
        xContinuous = !xContinuous;
        if (xContinuous) {
          stepMotorX(-1);
        } else {
          xMoving = false;
        }
        lastStepTimeX = currentMicros;
        break;
        
      case 'R': // Toggle Y positive
        yContinuous = !yContinuous;
        if (yContinuous) {
          stepMotorY(1);
        } else {
          yMoving = false;
        }
        lastStepTimeY = currentMicros;
        break;
        
      case 'L': // Toggle Y negative
        yContinuous = !yContinuous;
        if (yContinuous) {
          stepMotorY(-1);
        } else {
          yMoving = false;
        }
        lastStepTimeY = currentMicros;
        break;
        
      case 'S': // Stop all movement
        stopAll();
        break;
        
      case 'H': // Home command
        home();
        break;
        
      case 'F': // Increase speed
        if (currentSpeed < FAST) {
          currentSpeed = (SpeedState)(currentSpeed + 1);
          stepInterval = SPEEDS[currentSpeed];
        }
        break;
        
      case 'W': // Decrease speed
        if (currentSpeed > SLOW) {
          currentSpeed = (SpeedState)(currentSpeed - 1);
          stepInterval = SPEEDS[currentSpeed];
        }
        break;
        
      case 'X': // Report X position
        Serial.println(xStepCount);
        break;
        
      case 'Y': // Report Y position
        Serial.println(yStepCount);
        break;
        
      case 'P': // Report start position
        Serial.print("Start: (");
        Serial.print(startX);
        Serial.print(", ");
        Serial.print(startY);
        Serial.println(")");
        break;
        
      case 'T': // Test command
        Serial.println("OK");
        break;
    }
    
    digitalWrite(directionPin_x, xDir);
    digitalWrite(directionPin_y, yDir);
  }

  // X-axis stepping
  if (xContinuous && (currentMicros - lastStepTimeX >= stepInterval)) {
    digitalWrite(stepPin_x, !digitalRead(stepPin_x));
    lastStepTimeX = currentMicros;
    
    // Update position and check if reached start position
    if (xDir == HIGH) {
      xStepCount++;
      if (homingInProgress && xStepCount >= startX) {
        xContinuous = false;
        xMoving = false;
      }
    } else {
      xStepCount--;
      if (homingInProgress && xStepCount <= startX) {
        xContinuous = false;
        xMoving = false;
      }
    }
  }

  // Y-axis stepping
  if (yContinuous && (currentMicros - lastStepTimeY >= stepInterval)) {
    digitalWrite(stepPin_y, !digitalRead(stepPin_y));
    lastStepTimeY = currentMicros;
    
    // Update position and check if reached start position
    if (yDir == HIGH) {
      yStepCount++;
      if (homingInProgress && yStepCount >= startY) {
        yContinuous = false;
        yMoving = false;
      }
    } else {
      yStepCount--;
      if (homingInProgress && yStepCount <= startY) {
        yContinuous = false;
        yMoving = false;
      }
    }
  }

  // Check if homing is complete
  if (homingInProgress && !xMoving && !yMoving) {
    homingInProgress = false;
    Serial.println("Homing complete - returned to start position");
    Serial.print("Current position: (");
    Serial.print(xStepCount);
    Serial.print(", ");
    Serial.print(yStepCount);
    Serial.println(")");
  }
}
