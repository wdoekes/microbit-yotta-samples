#include "MicroBit.h"

#if CONFIG_ENABLED(MICROBIT_BLE_ENABLED)
// See: https://lancaster-university.github.io/microbit-docs/advanced/
# error Disable BLE (config.json:{"microbit-dal":{"bluetooth":"enabled":0}}})
#endif

#define MY_APP_FREQ 33
#define MY_APP_GROUP 0x73

template< typename T, size_t N >
constexpr size_t countof( const T (&)[N] ) { return N; }

struct Device {
    ManagedString name;
    uint32_t serial;
    uint8_t row;
};

Device KnownDevices[6] = {
    Device{ .name = "W", .serial = 0x12345678, .row = 255 },
    Device{ .name = "M", .serial = 0x12345679, .row = 255 },
    Device{ .name = "Jesper", .serial = 0x14255506, .row = 255 },
    Device{ .name = "Seb", .serial = 0x569A7D70, .row = 255 },
    Device{ .name = "Vera", .serial = 0x61091A15, .row = 255 },
    Device{ .name = "Mats", .serial = 0x1234567A, .row = 255 },
};

const Device *find_device_by_serial(uint32_t serial)
{
    for (uint8_t idx = 0; idx < countof(KnownDevices); ++idx) {
	if (KnownDevices[idx].serial == serial) {
	    return &KnownDevices[idx];
	}
    }
    return NULL;
}

void set_device_rows(const Device *skip_device)
{
    uint8_t row = 0;
    for (uint8_t idx = 0; idx < countof(KnownDevices); ++idx) {
	if (row >= 5) {
	    KnownDevices[idx].row = 255;
	} else if (&KnownDevices[idx] == skip_device) {
	    KnownDevices[idx].row = 255;
	} else {
	    KnownDevices[idx].row = row++;
	}
    }
}

class Levels : public MicroBitImage
{
    // We can use gradients, but only if we set:
    // uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
    // (That does require more CPU = more power.)
    const uint8_t gradient[10] = {
	255, 255, 255, 255, 255,
	//192, 128, 64,
	0, 0, 0, 0, 0,
    };
    const uint8_t max_level = countof(gradient) - 5;

public:
    Levels() : MicroBitImage(5, 5) {};

    void decay(uint8_t amount) {
	for (uint8_t row = 0; row < 5; ++row) {
	    uint8_t row_amount = amount;
	    for (uint8_t col = 4; row_amount && col != (uint8_t) -1; --col) {
		uint8_t val = getPixelValue(col, row);
		if (val > row_amount) {
		    setPixelValue(col, row, (val - row_amount));
		    row_amount = 0;
		} else {
		    setPixelValue(col, row, 0);
		    row_amount -= val;
		}
	    }
	}
    }

    void setLevel(uint8_t row, uint8_t level) {
	if (row < 5) {
	    if (level > max_level) {
		level = max_level;
	    }
	    uint8_t c;
	    const uint8_t *v;
	    for (c = 0, v = &gradient[max_level - level]; c < 5; ++c, ++v) {
		setPixelValue(c, row, *v);
	    }
	}
    }
};

MicroBit uBit;
uint32_t this_serial;
const Device *this_device;
MicroBitSerial serial(USBTX, USBRX);
Levels levels;
FrameBuffer *rx;

// Must be called with MESSAGE_BUS_LISTENER_IMMEDIATE; otherwise the
// handler has deleted the framebuffer already.
// NOTE: This means that we're not allowed to do much here, as we're
// called from an interrupt handler!
void on_radio(MicroBitEvent)
{
    FrameBuffer *fbuf = uBit.radio.recv();
    if (fbuf == NULL) {
#if 0
	serial.printf("event src %u value %u no-framebuffer\n",
		e.source, e.value);
#endif
	return;
    }
#if 0
    serial.printf(
	"event src %u value %u len %hu ver %hu group %hu "
	"proto %hu rssi %d vals 0x%08lX\n",
	    e.source, e.value,
	    fbuf->length, fbuf->version, fbuf->group, fbuf->protocol,
	    fbuf->rssi, fbuf->length, remote_serial);
#endif
    
    // Quickly store the framebuffers (in reverse order).
    fbuf->next = rx;
    rx = fbuf;
}

void on_process_framebuffer(FrameBuffer *fbuf)
{
    uint32_t remote_serial = 0xFFFFFFFF;
    if (fbuf->length >= 3 + sizeof(remote_serial)) {
	memcpy(&remote_serial, &fbuf->payload[0], sizeof(remote_serial));
    }

    // rssi can be in -42(close)..-120(far) or something
    int level = (100 + fbuf->rssi); // 58(close)..-20(far)
    level /= 8; // 7(close)..-2(far)
    if (level <= 0) {
	level = 0;
    } else if (level > 5) {
	level = 5;
    }

    serial.printf(
	    ("framebuffer len %hu ver %hu group %hu proto %hu "
	     "rssi %d(%d) vals 0x%08lX\n"),
	    fbuf->length, fbuf->version, fbuf->group, fbuf->protocol,
	    level, fbuf->rssi, fbuf->length, remote_serial);

    const Device* remote_device = find_device_by_serial(remote_serial);
    if (remote_device) {
	levels.setLevel(remote_device->row, level);
    }
}

void on_process_rx()
{
    FrameBuffer *fbuf;
    while ((fbuf = rx)) {
	rx = fbuf->next;
	on_process_framebuffer(fbuf);
	delete fbuf;
    }
}

void send_this_serial()
{
    FrameBuffer fbuf;
    fbuf.length = MICROBIT_RADIO_HEADER_SIZE - 1 + sizeof(this_serial);
    fbuf.length = 3 + sizeof(this_serial);
    fbuf.version = 0;
    fbuf.group = 0;
    // protocol must NOT be MICROBIT_RADIO_EVT_DATAGRAM (1) or
    // MICROBIT_RADIO_PROTOCOL_EVENTBUS (2) because they end up in
    // different handlers. With 0 we end up in the
    // MICROBIT_ID_RADIO_DATA_READY handler.
    fbuf.protocol = 0;
    memcpy(&fbuf.payload[0], &this_serial, sizeof(this_serial));
    fbuf.next = NULL;
    uBit.radio.send(&fbuf);
}

int main()
{
    // Find current device (NRF_FICR->DEVICEID[1])
    this_serial = microbit_serial_number();
    this_device = find_device_by_serial(this_serial);
    set_device_rows(this_device);

    // Init + say hello
    uBit.init();
    uBit.display.image.setPixelValue(2, 2, 255);
    // MESSAGE_BUS_LISTENER_IMMEDIATE is important here.
    uBit.messageBus.listen(MICROBIT_ID_RADIO_DATA_READY,
	MICROBIT_EVT_ANY, on_radio, MESSAGE_BUS_LISTENER_IMMEDIATE);

    serial.printf("Starting as %s [id 0x%08lX]\n",
	(this_device ? this_device->name.toCharArray() : "(null)"),
	this_serial);

    // Start broadcasting
    uBit.radio.enable(); // -> MICROBIT_OK
    uBit.radio.setTransmitPower(3); // (low power) 0..7 (high power)
    uBit.radio.setFrequencyBand(MY_APP_FREQ); // (2400MHz) 0..100 (2500MHz)
    uBit.radio.setGroup(MY_APP_GROUP); // must be same for all devices

    while (1) {
	// An activity dot in the center
	uBit.display.image.setPixelValue(2, 2, 255);
        uBit.sleep(5);

	// Process all received framebuffers and display the current levels
	on_process_rx();
	uBit.display.print(levels);
	levels.decay(128);

	// Broadcast our serial number and then sleep a bit
	send_this_serial();
        uBit.sleep(500);
    }
    
    release_fiber();
    return 0;
}
