#include <Arduino.h>
#include <SPI.h>
#include <RH_NRF905.h>
#include <FastLED.h>

#define CARIER_DETECT_PIN 2
#define LED_CONTROL_PIN 5
#define BUTTON_PIN 7

#define NUM_LEDS 12
#define BTN_REQUEST_TIMER 2000
#define RF_CHANNEL 100
#define BTN_ID 6

// #define RD_DIF 10
// #define RD_MIN 15
// #define RD_MAX 36

uint32_t colorBase[3] = {CRGB::Green, CRGB::Red, CRGB::Black}; // red green yellow black
uint32_t tm;
// int rnd, rndOld;

CRGB leds[NUM_LEDS];

RH_NRF905 nrf905;

struct dataStruct
{
    uint8_t id;
    uint8_t ledColor;
    bool enable;
} sendData;

void SetLed(uint8_t color);
void ReturnConfirmation();
// int DelayAction();

void setup()
{
    Serial.begin(57600);

    FastLED.addLeds<WS2812B, LED_CONTROL_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(60);

    pinMode(CARIER_DETECT_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // randomSeed(analogRead(A5));

    if (!nrf905.init())
        Serial.println("nrf905 init failed");
    else
        Serial.println("nrf905 init OK");

    // Defaults after init are 433.2 MHz (channel 108), -10dBm
    nrf905.setRF(RH_NRF905::TransmitPower10dBm);
    delay(10);
    nrf905.setChannel(RF_CHANNEL); // 434.0 MHz (116)
    delay(10);

    // Start initialisation-----------------------------
    sendData.id = BTN_ID;   // this button ID
    sendData.ledColor = 1;  // start color green
    sendData.enable = true; // button read enable
    //--------------------------------------------------

    SetLed(sendData.ledColor); // green

    tm = millis() + BTN_REQUEST_TIMER;
}

void loop()
{
    if (sendData.enable)
    {
        if (!digitalRead(BUTTON_PIN) && millis() - tm > BTN_REQUEST_TIMER)
        {
            Serial.println("\nSend data to server:");
            Serial.println("ID: " + String(sendData.id));
            Serial.println("Color: " + String(sendData.ledColor));
            Serial.println("Enable: " + String(sendData.enable));

            // int rd = DelayAction();
            // Serial.println("\nDelay: " + String(rd));

            // delay(rd);

            if (!digitalRead(CARIER_DETECT_PIN))
            {
                nrf905.send((uint8_t *)&sendData, sizeof(sendData));

                delay(10);

                if (nrf905.waitPacketSent())
                    Serial.println("\nData sended");
            }
            else
                Serial.println("\nCollision. Aborted");

            tm = millis();
        }
    }

    if (nrf905.available())
    {
        delay(10);

        uint8_t buf[RH_NRF905_MAX_MESSAGE_LEN];
        uint8_t buflen = sizeof(buf);

        uint8_t idOld = sendData.id;
        uint8_t ledColorOld = sendData.ledColor;
        bool enableOld = sendData.enable;

        if (nrf905.recv((uint8_t *)(&sendData), &buflen))
        {
            Serial.println("\nRecieved message from server");
            Serial.println("ID: " + String(sendData.id));
            Serial.println("Color: " + String(sendData.ledColor));
            Serial.println("Enable: " + String(sendData.enable));

            if (sendData.id != idOld) // data not for me?
            {
                sendData.id = idOld;
                sendData.ledColor = ledColorOld;
                sendData.enable = enableOld;
            }
            else // else is for me
            {
                SetLed(sendData.ledColor); // change for new color

                switch (sendData.ledColor)
                {
                case 0:
                    Serial.println("\nChange color on RED");
                    break;
                case 1:
                    Serial.println("\nChange color on GREEN");
                    break;
                case 2:
                    Serial.println("\nChange color OFF");
                    break;
                }

                ReturnConfirmation();
            }
        }
    }
}

void SetLed(uint8_t color)
{
    for (size_t i = 0; i < NUM_LEDS; i++)
        leds[i] = colorBase[color];

    FastLED.show();
}

void ReturnConfirmation()
{
    while (digitalRead(CARIER_DETECT_PIN))
        ;
    delay(10);

    Serial.println("\nSend confirmation");

    nrf905.send((uint8_t *)&sendData, sizeof(sendData));

    if (nrf905.waitPacketSent())
        Serial.println("\nConfirmation sended");
}

// int DelayAction()
// {
//     rnd = random(RD_MIN, RD_MAX);

//     while (abs(rnd - rndOld) < RD_DIF)
//         rnd = random(RD_MIN, RD_MAX);

//     rndOld = rnd;

//     return rnd;
// }