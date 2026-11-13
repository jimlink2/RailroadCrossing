#include <Servo.h>

Servo gate;

const int GATE_DOWN = 20;
const int GATE_UP   = 90;

void setup()
{
  gate.attach(9);
  gate.write(GATE_UP);
}

void loop()
{
  // Lower gate slowly
  for (int pos = GATE_UP; pos >= GATE_DOWN; pos--)
  {
    gate.write(pos);
    delay(20);
  }

  delay(3000);

  // Raise gate slowly
  for (int pos = GATE_DOWN; pos <= GATE_UP; pos++)
  {
    gate.write(pos);
    delay(20);
  }

  delay(3000);
}