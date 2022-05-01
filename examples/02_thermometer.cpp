#include "MicroBit.h"

MicroBit uBit;
MicroBitSerial serial(USBTX, USBRX);
MicroBitStorage storage;
MicroBitThermometer thermometer(storage);

int main()
{
    uBit.init();

    serial.send("Started\n");
    do {
	char buf[256];
	sprintf(buf, "temp %d\r\n", thermometer.getTemperature());
	serial.send(buf);
	uBit.sleep(1000);
    } while (1);

    release_fiber();
    return 0;
}

