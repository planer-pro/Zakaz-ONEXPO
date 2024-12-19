#include <Arduino.h>

uint8_t last;
uint32_t tm;

void setup()
{
    Serial.begin(115200);

    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
}

void loop()
{
    if (digitalRead(5) && last != 1)
    {
        last = 1;
        Serial.println("CH_A");
        tm = millis();
    }
    else if (digitalRead(4) && last != 2)
    {
        last = 2;
        Serial.println("CH_B");
        tm = millis();
    }
    else if (digitalRead(3) && last != 3)
    {
        last = 3;
        Serial.println("CH_C");
        tm = millis();
    }
    else if (digitalRead(2) && last != 4)
    {
        last = 4;
        Serial.println("CH_D");
        tm = millis();
    }

    if (millis() - tm > 500)
        last = 0;
}