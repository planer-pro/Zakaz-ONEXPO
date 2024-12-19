#include <Arduino.h>
#include <FastLED.h>
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define NUM_LEDS 59

#define DATA_LED_PIN A3
#define BUZZ_PIN 8

#define VIRTUAL_GND_PIN 25

#define RED_BTN_BEGIN_PIN 26 // button pins
#define RED_BTN_END_PIN 33
#define GREEN_BTN_BEGIN_PIN 34
#define GREEN_BTN_END_PIN 41
#define BTN_CNT RED_BTN_END_PIN - RED_BTN_BEGIN_PIN + 1 // qantity buttons

#define LED_LINE_ADR_MIN 0 // leds block addresses
#define LED_LINE_ADR_MAX 42
#define RED_LINE_ADR_MIN 43
#define RED_LINE_ADR_MAX 50
#define GREEN_LINE_ADR_MIN 51
#define GREEN_LINE_ADR_MAX 58
#define LED_ZERO LED_LINE_ADR_MAX / 2

#define DELAY_BETWEEN_NEW_STEP 500
#define WINNER_BLINK_DELAY 250
#define INTRO_SHIFT_DELAY 15

#define RED_COLOR CRGB::Green
#define GREEN_COLOR CRGB::Red
#define YELLOW_COLOR CRGB::Yellow
#define BLUE_COLOR CRGB::Blue
#define WHITE_COLOR CRGB::White
#define OFF_COLOR CRGB::Black

CRGB leds[NUM_LEDS];
SoftwareSerial mySerial(A2, A1); // RX, TX
DFPlayerMini_Fast myMP3;

uint8_t rnd = 0;
int ledVar = 0;
bool enBlink = false, enGame = false, enBuzz = true, enIntro = true, enSound = true, enExtRules = true;

bool inRange(int value, int minRange, int maxRange);
void SetLed(int ledNo, uint32_t color);
void OneStep();
int GetKeyNum();
void SetRedLedPlus();
void SetGreenLedPlus();
void SetLedLine();
void BlinkWinner();
void LedsBlinkHeadler();
void BeginIntro();
void SetBuzzFraze(uint8_t n);
void ShowLedLineWiner();
void eepromWrite();
void eepromRead();
void setupPreparation();

void setup()
{
    Serial.begin(115200);
    mySerial.begin(9600);

    for (size_t i = RED_BTN_BEGIN_PIN; i <= GREEN_BTN_END_PIN; i++)
    {
        pinMode(i, INPUT_PULLUP);
        delay(10);
    }

    pinMode(VIRTUAL_GND_PIN, OUTPUT);
    digitalWrite(VIRTUAL_GND_PIN, LOW);

    // eepromWrite();

    setupPreparation(); // get setup configuration

    randomSeed(analogRead(A0));

    FastLED.addLeds<WS2812B, DATA_LED_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(100);

    myMP3.begin(mySerial);
    delay(100);
    myMP3.volume(30);
    delay(100);
    myMP3.stop();
    delay(200);

    if (enIntro)
        myMP3.play(random(1, 3));

    BeginIntro();

    SetLedLine(); // indicate startup led line state

    enGame = true;
}

void loop()
{
    static uint8_t part = 0;
    static uint32_t tm = 0;

    LedsBlinkHeadler();

    if (enGame)
    {
        switch (part)
        {
        case 0:
            if (millis() - tm > DELAY_BETWEEN_NEW_STEP)
            {
                SetBuzzFraze(0); // buzz new step fraze
                OneStep();

                part++;
            }

            break;
        case 1:
            int b = GetKeyNum();

            if (b > 0)
            {
                if (inRange(b, RED_BTN_BEGIN_PIN, RED_BTN_END_PIN)) // is it Red user?
                {
                    if ((b - RED_BTN_BEGIN_PIN) == rnd)
                    {
                        SetBuzzFraze(1); // buzz right fraze
                        SetRedLedPlus();
                    }
                    else
                    {
                        SetBuzzFraze(2); // buzz wrong fraze

                        if (enExtRules)
                            SetGreenLedPlus();
                    }
                }
                else // no, it is Green user
                {
                    if ((b - GREEN_BTN_BEGIN_PIN) == rnd)
                    {
                        SetBuzzFraze(1); // buzz right fraze
                        SetGreenLedPlus();
                    }
                    else
                    {
                        SetBuzzFraze(2); // buzz wrong fraze

                        if (enExtRules)
                            SetRedLedPlus();
                    }
                }

                tm = millis();

                part = 0;
            }

            break;
        }
    }
}

bool inRange(int value, int minRange, int maxRange)
{
    return (value >= minRange && value <= maxRange);
}

void SetLed(int ledNo, uint32_t color)
{
    if (ledNo == -1) // set color to buttons led
        for (size_t i = RED_LINE_ADR_MIN; i <= GREEN_LINE_ADR_MAX; i++)
            leds[i] = color;
    else if (ledNo == -2) // set color to led line
        for (size_t i = LED_LINE_ADR_MIN; i <= LED_LINE_ADR_MAX; i++)
            leds[i] = color;
    else
        leds[ledNo] = color;

    FastLED.show();
}

void OneStep()
{
    static uint8_t rndOld = 0;

    rnd = random(0, BTN_CNT);

    while (rnd == rndOld)
        rnd = random(0, BTN_CNT);

    rndOld = rnd;

    SetLed(-1, OFF_COLOR); // reset all button leds
    SetLed(rnd + RED_LINE_ADR_MIN, RED_COLOR);
    SetLed(rnd + GREEN_LINE_ADR_MIN, GREEN_COLOR);
}

int GetKeyNum()
{
    for (int i = RED_BTN_BEGIN_PIN; i <= GREEN_BTN_END_PIN; i++)
    {
        if (!digitalRead(i))
            return i;
    }

    return -1;
}

void SetRedLedPlus()
{
    ledVar++;

    if (ledVar < LED_ZERO - 1)
        SetLedLine();
    else
    {
        SetLedLine();

        Serial.println("Red Win");

        enGame = false;

        if (enSound)
            myMP3.play(random(3, 5));

        BlinkWinner();
    }
}

void SetGreenLedPlus()
{
    ledVar--;

    if (ledVar > (LED_ZERO * -1) + 1)
        SetLedLine();
    else
    {
        SetLedLine();

        Serial.println("Green Win");

        enGame = false;

        if (enSound)
            myMP3.play(random(3, 5));

        BlinkWinner();
    }
}

void SetLedLine()
{
    SetLed(-2, OFF_COLOR);

    if (ledVar > 0)
    {
        // SetLed(LED_ZERO + ledVar, RED_COLOR);

        for (int i = -1; i < 2; i++)
        {
            SetLed(LED_ZERO + ledVar + i, RED_COLOR);
        }

        // for (int i = 0; i <= ledVar; i++)
        //     SetLed(LED_ZERO + i, RED_COLOR);
    }
    else if (ledVar < 0)
    {
        // SetLed(LED_ZERO + ledVar, GREEN_COLOR);

        for (int i = -1; i < 2; i++)
        {
            SetLed(LED_ZERO + ledVar + i, GREEN_COLOR);
        }

        // for (int i = 0; i >= ledVar; i--)
        //     SetLed(LED_ZERO + i, GREEN_COLOR);
    }
    else
    {
        SetLed(LED_ZERO + 1, RED_COLOR);
        SetLed(LED_ZERO - 1, GREEN_COLOR);
    }

    SetLed(LED_ZERO, BLUE_COLOR);
}

void ShowLedLineWiner()
{
    SetLed(-2, OFF_COLOR);

    if (ledVar > 0)
    {
        for (int i = 0; i <= ledVar + 1; i++)
            SetLed(LED_ZERO + i, RED_COLOR);
    }
    else if (ledVar < 0)
    {
        for (int i = 0; i >= ledVar - 1; i--)
            SetLed(LED_ZERO + i, GREEN_COLOR);
    }

    SetLed(LED_ZERO, BLUE_COLOR);
}

void BlinkWinner()
{
    enBlink = true;
}

void LedsBlinkHeadler()
{
    static bool state = false;
    static uint32_t tm = 0;

    if (enBlink)
    {
        if (millis() - tm > WINNER_BLINK_DELAY)
        {
            if (state)
            {
                ShowLedLineWiner();

                ledVar > 0 ? SetLed(-1, RED_COLOR) : SetLed(-1, GREEN_COLOR); // indicate winer color
            }
            else
            {
                SetLed(-2, OFF_COLOR);
                SetLed(-1, OFF_COLOR);
            }

            state = !state;

            tm = millis();
        }
    }
}

void BeginIntro()
{
    for (int i = LED_LINE_ADR_MIN; i <= LED_ZERO; i++)
    {
        SetLed(i, BLUE_COLOR);
        SetLed(LED_LINE_ADR_MAX - i, BLUE_COLOR);

        delay(INTRO_SHIFT_DELAY);
    }

    for (int i = LED_ZERO; i >= LED_LINE_ADR_MIN; i--)
    {
        SetLed(i, OFF_COLOR);
        SetLed(LED_LINE_ADR_MAX - i, OFF_COLOR);

        delay(INTRO_SHIFT_DELAY);
    }
}

void SetBuzzFraze(uint8_t n)
{
    if (enBuzz)
    {
        switch (n)
        {
        case 0:
            tone(BUZZ_PIN, 1500, 50); // new step
            delay(50);                //
            tone(BUZZ_PIN, 1200, 50); //
            break;
        case 1:
            tone(BUZZ_PIN, 4000, 50); // right
            break;
        case 2:
            tone(BUZZ_PIN, 2000, 20); // wrong pattern
            delay(30);                //
            tone(BUZZ_PIN, 2000, 20); //
            delay(30);                //
            tone(BUZZ_PIN, 2000, 20); //
            delay(30);                //
            tone(BUZZ_PIN, 2000, 20); //
            break;
        }
    }
}

void eepromWrite()
{
    EEPROM.put(0, enBuzz);
    delay(10);
    EEPROM.put(1, enIntro);
    delay(10);
    EEPROM.put(2, enSound);
    delay(10);
    EEPROM.put(3, enExtRules);
    delay(10);
}

void eepromRead()
{
    EEPROM.get(0, enBuzz);
    delay(10);
    EEPROM.get(1, enIntro);
    delay(10);
    EEPROM.get(2, enSound);
    delay(10);
    EEPROM.get(3, enExtRules);
    delay(10);
}

void setupPreparation()
{
    if (!digitalRead(RED_BTN_BEGIN_PIN))
    {
        eepromRead();
        enBuzz = !enBuzz;
        eepromWrite();
    }
    else if (!digitalRead(RED_BTN_BEGIN_PIN + 1))
    {
        eepromRead();
        enIntro = !enIntro;
        eepromWrite();
    }
    else if (!digitalRead(RED_BTN_BEGIN_PIN + 2))
    {
        eepromRead();
        enSound = !enSound;
        eepromWrite();
    }
    else if (!digitalRead(RED_BTN_BEGIN_PIN + 3))
    {
        eepromRead();
        enExtRules = !enExtRules;
        eepromWrite();
    }
    else
        eepromRead();
}