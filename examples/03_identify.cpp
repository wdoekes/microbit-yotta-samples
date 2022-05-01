#include "MicroBit.h"

template< typename T, size_t N >
constexpr size_t countof( const T (&)[N] ) { return N; }

struct Device {
    ManagedString name;
    uint32_t serial;
};

const Device KnownDevices[3] = {
    Device{ .name = "Jesper", .serial = 0x14255506 },
    Device{ .name = "Seb", .serial = 0x569A7D70 },
    Device{ .name = "Vera", .serial = 0x61091A15 },
};

MicroBit uBit;
const Device *thisDevice;
MicroBitSerial serial(USBTX, USBRX);

int main()
{
    // Find current device
    uint32_t serial_no = microbit_serial_number(); /* NRF_FICR->DEVICEID[1] */
    for (uint8_t idx = 0; idx < countof(KnownDevices); ++idx) {
	if (KnownDevices[idx].serial == serial_no) {
	    thisDevice = &KnownDevices[idx];
	}
    }

    uBit.init();
    do {
	if (thisDevice) {
	    serial.send(thisDevice->name);
	    uBit.display.scroll(thisDevice->name);
	} else {
	    char buf[16];
	    sprintf(buf, "{0x%08lX}", serial_no);
	    serial.send(buf);
	    uBit.display.scroll(buf);
	}
	serial.sendChar('\n');
        uBit.display.scroll(" ");
    } while (1);

    // Fibers are lightweight threads used by the runtime to perform
    // operations asynchronously. We're done with ours.
    release_fiber();
    return 0;
}

