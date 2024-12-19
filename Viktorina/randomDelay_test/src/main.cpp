#include <Arduino.h>

#define RD_DIF 10
#define RD_MIN 15
#define RD_MAX 36

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(A0));

    pinMode(7, INPUT_PULLUP);
}

int rnd, rndOld;
uint32_t tm1; //, tm2;

void loop()
{
    if (millis() - tm1 > 1000)
    {
        rnd = random(RD_MIN, RD_MAX);

        while (abs(rnd - rndOld) < RD_DIF)
            rnd = random(RD_MIN, RD_MAX);

        rndOld = rnd;

        Serial.println(rnd);

        tm1 = millis();
    }

    // if (millis() - tm2 > 250)
    // {
    //     Serial.println(digitalRead(7));
    //     tm2 = millis();
    // }
}