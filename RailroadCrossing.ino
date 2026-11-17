#include <Servo.h>

const int SENSOR_A_PIN = 2;
const int SENSOR_B_PIN = 3;

const int LEFT_LED_PIN  = 4;
const int RIGHT_LED_PIN = 5;

const int SERVO_PIN = 9;

const int GATE_DOWN = 25;
const int GATE_UP   = 100;

const unsigned long CLEAR_DELAY  = 300;
const unsigned long WARNING_TIME = 100;
const unsigned long REARM_DELAY  = 1000;

// Flash timing
const unsigned long FLASH_INTERVAL = 350;

// Servo timing
const unsigned long SERVO_INTERVAL = 20;

Servo gateServo;

enum State
{
  IDLE,
  WARNING,
  LOWERING,
  A_TO_B_WAIT_FOR_B,
  A_TO_B_EXITING,
  B_TO_A_WAIT_FOR_A,
  B_TO_A_EXITING,
  RAISING,
  REARM
};

State state = IDLE;

int gatePosition = GATE_UP;

unsigned long clearTimer = 0;
unsigned long flashTimer = 0;
unsigned long servoTimer = 0;
unsigned long warningStartTime = 0;
unsigned long rearmStartTime = 0;

bool flashSide = false;
bool directionAToB = true;

//--------------------------------------------------

void startFlashing()
{
  flashTimer = millis();
  flashSide = false;

  digitalWrite(LEFT_LED_PIN, HIGH);
  digitalWrite(RIGHT_LED_PIN, LOW);
}

void stopFlashing()
{
  digitalWrite(LEFT_LED_PIN, LOW);
  digitalWrite(RIGHT_LED_PIN, LOW);
}

void updateFlashers()
{
  if (state == IDLE || state == REARM)
  {
    digitalWrite(LEFT_LED_PIN, LOW);
    digitalWrite(RIGHT_LED_PIN, LOW);
    return;
  }

  if (millis() - flashTimer >= FLASH_INTERVAL)
  {
    flashTimer = millis();

    flashSide = !flashSide;

    digitalWrite(LEFT_LED_PIN, flashSide);
    digitalWrite(RIGHT_LED_PIN, !flashSide);
  }
}

//--------------------------------------------------

bool moveGateToward(int target)
{
  if (millis() - servoTimer < SERVO_INTERVAL)
    return false;

  servoTimer = millis();

  if (gatePosition < target)
  {
    gatePosition++;
    gateServo.write(gatePosition);
  }
  else if (gatePosition > target)
  {
    gatePosition--;
    gateServo.write(gatePosition);
  }

  return (gatePosition == target);
}

//--------------------------------------------------

void setup()
{
  Serial.begin(115200);

  pinMode(SENSOR_A_PIN, INPUT_PULLUP);
  pinMode(SENSOR_B_PIN, INPUT_PULLUP);

  pinMode(LEFT_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);

  gatePosition = GATE_UP;
  gateServo.write(gatePosition);

  Serial.println("Railroad Crossing Controller");
}

//--------------------------------------------------

void loop()
{
  bool sensorA = (digitalRead(SENSOR_A_PIN) == LOW);
  bool sensorB = (digitalRead(SENSOR_B_PIN) == LOW);

  updateFlashers();

  switch (state)
  {
    case IDLE:

      if (sensorA)
      {
        Serial.println("Train entered from A");

        directionAToB = true;

        startFlashing();

        warningStartTime = millis();

        state = WARNING;
      }
      else if (sensorB)
      {
        Serial.println("Train entered from B");

        directionAToB = false;

        startFlashing();

        warningStartTime = millis();

        state = WARNING;
      }

      break;

    case WARNING:

      if (millis() - warningStartTime >= WARNING_TIME)
      {
        Serial.println("Lowering Gate");

        state = LOWERING;
      }

      break;

    case LOWERING:

      if (moveGateToward(GATE_DOWN))
      {
        if (directionAToB)
        {
          Serial.println("Gate Down, waiting for B");
          state = A_TO_B_WAIT_FOR_B;
        }
        else
        {
          Serial.println("Gate Down, waiting for A");
          state = B_TO_A_WAIT_FOR_A;
        }
      }

      break;

    case A_TO_B_WAIT_FOR_B:

      if (sensorB)
      {
        Serial.println("Train reached B");

        clearTimer = 0;
        state = A_TO_B_EXITING;
      }

      break;

    case A_TO_B_EXITING:

      if (!sensorB)
      {
        if (clearTimer == 0)
          clearTimer = millis();

        if (millis() - clearTimer >= CLEAR_DELAY)
        {
          Serial.println("Train cleared B");
          state = RAISING;
        }
      }
      else
      {
        clearTimer = 0;
      }

      break;

    case B_TO_A_WAIT_FOR_A:

      if (sensorA)
      {
        Serial.println("Train reached A");

        clearTimer = 0;
        state = B_TO_A_EXITING;
      }

      break;

    case B_TO_A_EXITING:

      if (!sensorA)
      {
        if (clearTimer == 0)
          clearTimer = millis();

        if (millis() - clearTimer >= CLEAR_DELAY)
        {
          Serial.println("Train cleared A");
          state = RAISING;
        }
      }
      else
      {
        clearTimer = 0;
      }

      break;

    case RAISING:

      if (moveGateToward(GATE_UP))
      {
        stopFlashing();

        Serial.println("Gate Up");

        rearmStartTime = 0;

        state = REARM;
      }

      break;

    case REARM:

      digitalWrite(LEFT_LED_PIN, LOW);
      digitalWrite(RIGHT_LED_PIN, LOW);

      if (!sensorA && !sensorB)
      {
        if (rearmStartTime == 0)
          rearmStartTime = millis();

        if (millis() - rearmStartTime >= REARM_DELAY)
        {
          Serial.println("Crossing idle");
          state = IDLE;
        }
      }
      else
      {
        rearmStartTime = 0;
      }

      break;
  }
}