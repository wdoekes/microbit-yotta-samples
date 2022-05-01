#include "MicroBit.h"

MicroBit uBit;
MicroBitSerial serial(USBTX, USBRX);

void onEvent(MicroBitEvent e)
{
    // You may want to exclude all of these:
    // MICROBIT_ID_MESSAGE_BUS_LISTENER 1021
    // MICROBIT_ID_NOTIFY_ONE           1022
    // MICROBIT_ID_NOTIFY               1023
    if (e.source == MICROBIT_ID_NOTIFY && e.value == 2) {
	;
    } else {
	char buf[256];
	sprintf(buf, "event src %u value %u\n", e.source, e.value);
	serial.send(buf);
    }
}

int main()
{
    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_ANY, MICROBIT_EVT_ANY, onEvent);

    // We don't want to drop out of main!
    while(1) {
        uBit.sleep(100);
    }
}
