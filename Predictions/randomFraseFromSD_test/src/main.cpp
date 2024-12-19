#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

File myFile;

uint8_t rnd, oldRnd;

String GetDataString(uint8_t n);

void setup()
{
    Serial.begin(115200);

    randomSeed(analogRead(A0));

    Serial.print("Initializing SD card...");

    if (!SD.begin(10))
    {
        Serial.println("initialization failed!");
        return;
    }

    Serial.println("initialization done.");
}

void loop()
{
    rnd = random(1, 12);

    while (rnd == oldRnd)
        rnd = random(1, 12);

    oldRnd = rnd;

    Serial.print(rnd + String("   "));
    Serial.println(GetDataString(rnd));

    delay(1000);
}

String GetDataString(uint8_t n)
{
    String st = {};

    myFile = SD.open(String(n) + ".txt");

    if (myFile)
    {
        while (myFile.available())
        {
            st += (char)myFile.read();
        }

        myFile.close();
        return st;
    }
    else
    {
        Serial.println("error opening " + String(n) + ".txt");
        return "ERROR";
    }
}