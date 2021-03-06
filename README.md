NetIP - TCP/IP networking multiple devices over serial
======================================================

For project website and documentation, see [here](https://fjfranklin.github.io/NetIP/).

## Overview

The aim of NetIP is to be able to link multiple devices into a network using serial
connections, so that any device in - or connected to - the network can talk to any
other. This is achieved using Serial Line Internet Protocol (SLIP) between devices,
and forwarding packets transparently at intermediate nodes. UDP is supported, and
also a subset of TCP/IP. (There is no support for fragmentation and reassembly.)

The principal target of NetIP is embedded devices, so the program code is small and
memory requirements small, with no dynamic allocation of memory that would lead to
fragmentation. The two 'architectures' currently supported are 'Unix' (tested on
Linux and Mac OS X) and 'Arduino' (tested on the Uno, Due and Teensy 3.5).

## TODO

* TCP/IP: Connect works; need to implement data & connection-close
* Add an interface for the gateway device to handle external routing
* Test with multiple devices
* Doxygen documentation + GitHub Pages site documentation
* Q. What happens when a serial link blocks?

# License

Unless otherwise stated, the code and examples here are
provided under the MIT License:

Copyright (c) 2018 Francis James Franklin

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
