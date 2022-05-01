#include "MicroBit.h"

template< typename T, size_t N >
constexpr size_t countof( const T (&)[N] ) { return N; }

class Levels : public MicroBitImage {
    // We can use gradients, but only if we set:
    // uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
    const uint8_t gradient[10] = {
	255, 255, 255, 255, 255,
	//192, 128, 64,
	0, 0, 0, 0, 0,
    };
    const uint8_t max_level = countof(gradient) - 5;
public:
    Levels() : MicroBitImage(5, 5) {};
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
MicroBitSerial serial(USBTX, USBRX);

int main()
{
    uBit.init();
    serial.send("Started\n");

    Levels levels;
    levels.setLevel(0, 5);
    levels.setLevel(1, 0);
    levels.setLevel(2, 3);
    levels.setLevel(3, 2);
    levels.setLevel(4, 4);

    uBit.display.print(levels);

    // Fibers are lightweight threads used by the runtime to perform
    // operations asynchronously. We're done with ours.
    release_fiber();
    return 0;
}

