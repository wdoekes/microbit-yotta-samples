#include "MicroBit.h"

#if CONFIG_ENABLED(MICROBIT_DBG)
// See: https://lancaster-university.github.io/microbit-docs/advanced/
//# error '''{"microbit-dal": {"debug": 1}}''' from config.json works
#endif

MicroBit uBit;

// The MicroBitSerial serial must(!) be created as global or async
// fibers will fail.
MicroBitSerial serial(USBTX, USBRX);
// READ: stty -F /dev/ttyACM0 115200 raw -clocal -echo && cat /dev/ttyACM0

int main()
{
    // The scheduler, memory allocator and Bluetooth stack are
    // initialised.
    uBit.init();

    do {
	serial.send("Hello serial!\n");
	uBit.display.scroll("Hello world! ");
    } while (0);

    // Fibers are lightweight threads used by the runtime to perform
    // operations asynchronously. We're done with ours.
    release_fiber();
    return 0;
}

