#include <Arduino.h>
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>
#include <FastLED.h>

SoftwareSerial mySerial(10, 11); // RX, TX

#define BUTT_RST 2
#define BUTT_STRT 3
#define RING 4

#define NUM_LEDS 60
#define DATA_LED_PIN 5

CRGB leds[NUM_LEDS];

uint8_t part, modeLight;
uint32_t tm, tm2, cnt;
bool playState, p1, en;

DFPlayerMini_Fast myMP3;

void setup()
{
    Serial.begin(115200);
    mySerial.begin(9600);

    FastLED.addLeds<WS2812B, DATA_LED_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(60);

    myMP3.begin(mySerial);
    delay(100);
    myMP3.volume(30);
    delay(100);

    pinMode(BUTT_RST, INPUT_PULLUP);
    pinMode(BUTT_STRT, INPUT_PULLUP);
    pinMode(RING, INPUT_PULLUP);

    Serial.println();
    Serial.println("Ready");
}

void ws2812bHeadler()
{
    switch (modeLight)
    {
    case 0:
        for (size_t i = 0; i < NUM_LEDS; i++)
            leds[i] = CRGB::Black;

        FastLED.show();

        cnt = 0;
        en = false;

        break;
    case 1:
        if (millis() - tm > 10)
        {
            leds[cnt] = CRGB::Black;
            leds[cnt + 1] = CRGB::Black;
            leds[cnt + 2] = CRGB::Black;
            leds[cnt + 3] = CRGB::Black;
            leds[cnt + 4] = CRGB::Black;
            leds[cnt + 5] = CRGB::Black;
            leds[cnt + 6] = CRGB::Black;

            FastLED.show();

            if ((cnt + 6) == (NUM_LEDS - 1))
            {
                leds[NUM_LEDS - 1] = CRGB::Black;
                leds[NUM_LEDS - 2] = CRGB::Black;
                leds[NUM_LEDS - 3] = CRGB::Black;
                leds[NUM_LEDS - 4] = CRGB::Black;
                leds[NUM_LEDS - 5] = CRGB::Black;
                leds[NUM_LEDS - 6] = CRGB::Black;

                cnt = 0;
                en = false;
            }
            else
            {
                if (en)
                    cnt++;

                en = true;

                leds[cnt] = CRGB::Red;
                leds[cnt + 1] = CRGB::Red;
                leds[cnt + 2] = CRGB::Red;
                leds[cnt + 3] = CRGB::Red;
                leds[cnt + 4] = CRGB::Red;
                leds[cnt + 5] = CRGB::Red;
                leds[cnt + 6] = CRGB::Red;
            }

            FastLED.show();

            tm = millis();
        }
        break;
    case 2:
        if (millis() - tm2 > 250)
        {
            if (p1)
            {
                for (size_t i = 0; i < NUM_LEDS; i++)
                    leds[i] = CRGB::Green;
            }
            else
            {
                for (size_t i = 0; i < NUM_LEDS; i++)
                    leds[i] = CRGB::Black;
            }

            FastLED.show();

            p1 = !p1;

            tm2 = millis();
        }
        break;
    }
}

void loop()
{
    ws2812bHeadler();

    switch (part)
    {
    case 0:
        if (!digitalRead(BUTT_STRT))
        {
            Serial.println("Game Started");
            modeLight = 1;

            part++;
        }
        break;
    case 1:
        if (!digitalRead(RING))
        {
            Serial.println("Collision Detected");
            modeLight = 2;

            if (!playState)
            {
                playState = true;
                myMP3.loop(1);
            }
        }
        break;
    }

    if (!digitalRead(BUTT_RST))
    {
        Serial.println("System Reseted");
        modeLight = 0;

        if (playState)
        {
            playState = false;
            myMP3.stop();
        }

        part = 0;
    }
}

// #define NUM_LEDS 60

// #include "FastLED.h"

// #define PIN 2

// CRGB leds[NUM_LEDS];

// byte counter;

// void setup()
// {
//     FastLED.addLeds<WS2812B, PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//     FastLED.setBrightness(60);
// }

// void loop()
// {
//     for (int i = 0; i < NUM_LEDS; i++)
//     {                                              // от 0 до первой трети
//         leds[i] = CHSV(counter + i * 2, 255, 255); // HSV. Увеличивать HUE (цвет)
//                                                    // умножение i уменьшает шаг радуги
//     }

//     counter++; // counter меняется от 0 до 255 (тип данных byte)
//     FastLED.show();

//     delay(5); // скорость движения радуги
// }