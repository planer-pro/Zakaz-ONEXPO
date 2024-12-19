#include <Arduino.h>

class List
{
public:
    byte length;
    byte data[256];

    void append(byte item)
    {
        if (length < 256)
            data[length++] = item;
    }

    void remove(byte index)
    {
        if (index >= length)
            return;
        memmove(&data[index], &data[index + 1], length - index - 1);
        length--;
    }
};