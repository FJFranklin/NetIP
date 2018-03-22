/* Copyright (c) 2018 Francis James Franklin
 * 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and
 *    the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// included from source file ip_serial.cpp

#include <cstdio>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

IP_SerialChannel::IP_SerialChannel (const char * device_name) :
  device_fd(-1)
{
  device_fd = open (device_name, O_RDWR | O_NOCTTY | O_NONBLOCK /* O_NDELAY */);
  if (device_fd == -1) {
    fprintf (stderr, "Failed to open \"%s\" - exiting.\n", device_name);
    return;
  }
  // ...
}

IP_SerialChannel::~IP_SerialChannel () {
  if (device_fd >= 0) {
    close (device_fd);
  }
}

void IP_SerialChannel::update () {
  if (device_fd < 0) {
    return;
  }

  int  count;

  u8_t flags;

  const u8_t * bytes;

  while (slip_next_to_send (bytes, flags)) {
    count = (flags & IP_SLIP_ESCAPE) ? 2 : 1;

    ssize_t result = write (device_fd, bytes, count);

    if (result == -1) {
      fprintf (stderr, "IP_SerialChannel: Failed to write to device\n");
    } else if (result < count) {
      fprintf (stderr, "IP_SerialChannel: Incomplete write to device: %d bytes of %d written.\n", (int) result, count);
    }
  }

  u8_t byte;

  while (slip_can_receive ()) {
    count = read (device_fd, &byte, 1);

    if (count < 0) {
      if (errno == EAGAIN) {
	break;
      } else {
	fprintf (stderr, "IP_SerialChannel: Failed to read from device.\n");
	break;
      }
    } else if (count == 0) {
      break;
    }

    slip_receive (byte);
  }
}
