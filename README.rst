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


Documentation
-------------

Using *C/C++* and *yotta*:
`<https://lancaster-university.github.io/microbit-docs/ubit/>`_

Using the *C/C++* and the *Aduino IDE* instead:
`<https://github.com/wdoekes/microbit-arduino-samples>`_
