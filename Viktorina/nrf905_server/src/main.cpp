#include <Arduino.h>
#include <SPI.h>
#include <RH_NRF905.h>
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>
#include <list.cpp>

#define CARIER_DETECT_PIN 2
#define CHA_PIN 4
#define CHB_PIN 5
#define CHC_PIN 6
#define CHD_PIN 7

#define RF_CHANNEL 100
#define CONFERM_TIME_MS 200

SoftwareSerial mySerial(A1, A0); // RX, TX
RH_NRF905 nrf905;
DFPlayerMini_Fast myMP3;

List availBtns;
uint8_t part, partReset, partExtSound, countOfButtons;

void SendDataToBtn();
void SendResetData();
void AddToList();
bool CheckConfirmation();

struct dataStruct
{
    uint8_t id;
    uint8_t ledColor;
    bool enable;
} incomData;

void setup()
{
    Serial.begin(57600);
    mySerial.begin(9600);

    pinMode(CARIER_DETECT_PIN, INPUT);
    pinMode(CHC_PIN, INPUT);
    pinMode(CHD_PIN, INPUT);
    pinMode(CHA_PIN, INPUT);
    pinMode(CHB_PIN, INPUT);

    if (!nrf905.init())
        Serial.println("nrf905 init failed");
    else
        Serial.println("nrf905 init OK");

    // Defaults after init are 433.2 MHz (channel 108), -10dBm
    nrf905.setRF(RH_NRF905::TransmitPower10dBm);
    delay(10);
    nrf905.setChannel(RF_CHANNEL); // 434.0 MHz (116)
    delay(10);

    myMP3.begin(mySerial);
    delay(100);
    myMP3.volume(30);
    delay(100);
}

void loop()
{
    switch (part)
    {
    case 0:
        if (nrf905.available())
        {
            delay(10);

            uint8_t buf[RH_NRF905_MAX_MESSAGE_LEN];
            uint8_t buflen = sizeof(buf);

            if (nrf905.recv((uint8_t *)(&incomData), &buflen))
            {
                Serial.println("\nGot button:");
                Serial.println("ID: " + String(incomData.id));
                Serial.println("Color: " + String(incomData.ledColor));
                Serial.println("Enable: " + String(incomData.enable));

                myMP3.play(3);

                AddToList();

                incomData.ledColor = 0; // set red light
                SendDataToBtn();        // return red light

                while (!CheckConfirmation())
                    SendDataToBtn();

                part++;
            }
        }

        break;
    case 1:
        if (digitalRead(CHA_PIN)) // right variant
        {
            Serial.println("\nRight answer, reset system");

            myMP3.play(2);

            SendResetData();

            part = 0;
        }
        else if (digitalRead(CHB_PIN)) // wrong variant
        {
            Serial.println("\nWrong answer, block button");

            myMP3.play(1);

            incomData.ledColor = 2;
            incomData.enable = false;

            SendDataToBtn();

            while (!CheckConfirmation())
                SendDataToBtn();

            part = 0;
        }

        break;
    }

    switch (partReset)
    {
    case 0:
        if (digitalRead(CHC_PIN))
        {
            SendResetData();
            part = 0;
            partReset++;
        }

        break;
    case 1:
        if (!digitalRead(CHC_PIN))
            partReset = 0;

        break;
    }

    switch (partExtSound)
    {
    case 0:
        if (digitalRead(CHD_PIN))
        {
            if (!myMP3.isPlaying())
                myMP3.play(4);
            else
                myMP3.stop();

            partExtSound++;
        }

        break;
    case 1:
        if (!digitalRead(CHD_PIN))
            partExtSound = 0;

        break;
    }
}

void SendDataToBtn()
{
    while (digitalRead(CARIER_DETECT_PIN))
        ;
    delay(10);

    Serial.println("\nSend data to button:");
    Serial.println("ID: " + String(incomData.id));
    Serial.println("Color: " + String(incomData.ledColor));
    Serial.println("Enable: " + String(incomData.enable));

    nrf905.send((uint8_t *)&incomData, sizeof(incomData));

    if (nrf905.waitPacketSent())
        Serial.println("\nData sended");
}

void SendResetData()
{
    Serial.println("\nReseting buttons");

    for (size_t i = 0; i < availBtns.length; i++)
    {
        incomData.id = availBtns.data[i];
        incomData.ledColor = 1; // set green color
        incomData.enable = true;

        SendDataToBtn();

        while (!CheckConfirmation())
            SendDataToBtn();

        Serial.println("\nBtn " + String(availBtns.data[i]) + " confirmed");
    }
}

bool CheckConfirmation()
{
    if (nrf905.waitAvailableTimeout(CONFERM_TIME_MS))
    {
        uint8_t buf[RH_NRF905_MAX_MESSAGE_LEN];
        uint8_t buflen = sizeof(buf);

        // Backup native data-------------------------
        uint8_t confId = incomData.id;
        uint8_t confCol = incomData.ledColor;
        bool confEn = incomData.enable;
        //--------------------------------------------

        if (nrf905.recv((uint8_t *)(&incomData), &buflen))
        {
            Serial.println("\nGot confirmation:");
            Serial.println("ID: " + String(incomData.id));
            Serial.println("Color: " + String(incomData.ledColor));
            Serial.println("Enable: " + String(incomData.enable));

            if (incomData.id == confId /*&& incomData.ledColor == confCol && incomData.enable == confEn*/)
                Serial.println("\nConfirmation valid");
            else
            {
                Serial.println("\nAnother incoming data input");

                // Restore native data------------------------
                incomData.id = confId;
                incomData.ledColor = confCol;
                incomData.enable = confEn;
                //--------------------------------------------

                return false;
            }
        }

        return true;
    }
    else
        return false;
}

void AddToList()
{
    for (size_t i = 0; i < availBtns.length; i++)
    {
        if (incomData.id == availBtns.data[i])
        {
            Serial.println("\nAlready in list");

            return;
        }
    }

    Serial.println("\nAppend to list");

    availBtns.append(incomData.id);
    countOfButtons++;

    Serial.println("\nButtons: " + String(countOfButtons));
}