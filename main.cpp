#include "MicroBit.h"

MicroBit uBit;

int main()
{
    // The scheduler, memory allocator and Bluetooth stack are
    // initialised.
    uBit.init();

    do {
	uBit.display.scroll("Hello world! ");
    } while (0);

    // Fibers are lightweight threads used by the runtime to perform
    // operations asynchronously. We're done with ours.
    release_fiber();
    return 0;
}

