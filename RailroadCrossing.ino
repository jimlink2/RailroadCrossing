#include <Servo.h>

Servo gate;

const int GATE_DOWN = 25;
const int GATE_UP   = 105;

const int LEFT_LED  = 4;
const int RIGHT_LED = 5;

// Adjust this to taste:
// 20 = fast
// 50 = moderate
// 75 = slow
const unsigned long SERVO_MOVE_INTERVAL = 50;

enum State
{
  IDLE,
  LOWERING,
  OCCUPIED,
  RAISING
};

State state = IDLE;

unsigned long stateStartTime = 0;
unsigned long lastFlashTime = 0;
unsigned long lastServoTime = 0;

bool leftLampOn = true;
int gatePos = GATE_UP;

void setup()
{
  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);

  gate.attach(9);
  gate.write(GATE_UP);

  stateStartTime = millis();
}

void updateFlashers()
{
  if (state == IDLE)
  {
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    return;
  }

  if (millis() - lastFlashTime >= 250)
  {
    lastFlashTime = millis();

    leftLampOn = !leftLampOn;

    digitalWrite(LEFT_LED, leftLampOn);
    digitalWrite(RIGHT_LED, !leftLampOn);
  }
}

void loop()
{
  updateFlashers();

  switch (state)
  {
    case IDLE:

      // Simulate train approaching every 5 seconds
      if (millis() - stateStartTime > 5000)
      {
        state = LOWERING;
        stateStartTime = millis();
      }

      break;

    case LOWERING:

      if (millis() - lastServoTime >= SERVO_MOVE_INTERVAL)
      {
        lastServoTime = millis();

        if (gatePos > GATE_DOWN)
        {
          gatePos--;
          gate.write(gatePos);
        }
        else
        {
          state = OCCUPIED;
          stateStartTime = millis();
        }
      }

      break;

    case OCCUPIED:

      // Fake train occupies crossing for 10 seconds
      if (millis() - stateStartTime > 10000)
      {
        state = RAISING;
      }

      break;

    case RAISING:

      if (millis() - lastServoTime >= SERVO_MOVE_INTERVAL)
      {
        lastServoTime = millis();

        if (gatePos < GATE_UP)
        {
          gatePos++;
          gate.write(gatePos);
        }
        else
        {
          state = IDLE;
          stateStartTime = millis();
        }
      }

      break;
  }
}