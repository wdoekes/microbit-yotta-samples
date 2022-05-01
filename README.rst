micro:bit and yotta samples
===========================

(Apparently *yotta* itself is deprecated and no longer maintained by
*ARM*. It is still by far the easiest way to get started coding for the
*micro:bit* in *C/C++*.)

Prerequisites (works on *Ubuntu/Focal*)::

    apt-get install yotta

Setting up build env and building::

    yotta target bbc-microbit-classic-gcc@https://github.com/lancaster-university/yotta-target-bbc-microbit-classic-gcc
    yotta build

Or use the provided Makefile::

    make

Uploading to the *micro:bit*::

    make upload


Reading from the Serial port
----------------------------

Create a tiny shell script::

    sudo tee /usr/local/bin/serial-monitor <<EOF
    #!/bin/sh
    set -eu
    test "\${1:-}" = '-t' && { ts=true; shift; } || ts=false
    dev=\${1:-/dev/ttyACM0}
    baud=\${2:-115200}
    stty -F "\$dev" "\$baud" raw -clocal -echo
    echo "Listening on \$dev with \$baud baud" >&2
    if \$ts; then
        perl -e 'use Time::HiRes qw(gettimeofday);use POSIX qw(strftime);
          my \$nl=1;while(sysread STDIN,\$b,1){
          if(\$nl){(\$t,\$ut)=gettimeofday;syswrite STDOUT,
          (strftime "%H:%M:%S",localtime \$t).(sprintf ".%06d: ",\$ut);\$nl=0}
          \$nl=1 if \$b eq "\\n";syswrite STDOUT,\$b}' <"\$dev"
    else
        cat "\$dev"
    fi
    EOF

    sudo chmod 755 /usr/local/bin/serial-monitor

Now you can read serial communications from the connected *micro:bit*
using ``serial-monitor`` or ``serial-monitor -t``.

The *micro:bit* side might look like::

    #include "MicroBit.h"
    MicroBitSerial serial(USBTX, USBRX); // must be a global

    int main() {
        serial.send("Hi!\r\n");
        return 0;
    }


Documentation
-------------

Using *C/C++* and *yotta*:
`<https://lancaster-university.github.io/microbit-docs/ubit/>`_

Using the *C/C++* and the *Aduino IDE* instead:
`<https://github.com/wdoekes/microbit-arduino-samples>`_
