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

#include <cstdio>

#include <netip/ip_manager.hh>
#include <netip/ip_serial.hh>

#include <signal.h>

#include "tests.hh"

class Uino : public IP_TimerClient {
private:
  u8_t number;

public:
  Uino () :
    number(0)
  {
    // ...
  }

  void test (const char * info, const u8_t * buffer, u16_t length) {
    fputs ("\n", stderr);
    fputs (info, stderr);
    fputs ("\n", stderr);

    IP_Buffer B;
    B.append (buffer, length);

    IP_Header H;
    switch (H.sniff (B)) {
    case IP_Header::hs_Okay:
      fputs ("hs_Okay\n", stderr);
      break;
    case IP_Header::hs_EchoRequest:
      fputs ("hs_EchoRequest\n", stderr);
      break;
    case IP_Header::hs_EchoReply:
      fputs ("hs_EchoReply\n", stderr);
      break;
    case IP_Header::hs_IPv4:
      fputs ("hs_IPv4\n", stderr);
      break;
    case IP_Header::hs_IPv4_FrameError:
      fputs ("hs_IPv4_FrameError\n", stderr);
      break;
    case IP_Header::hs_IPv4_PacketTooShort:
      fputs ("hs_IPv4_PacketTooShort\n", stderr);
      break;
    case IP_Header::hs_IPv4_Checksum:
      fputs ("hs_IPv4_Checksum\n", stderr);
      break;
    case IP_Header::hs_IPv6:
      fputs ("hs_IPv6\n", stderr);
      break;
    case IP_Header::hs_IPv6_FrameError:
      fputs ("hs_IPv6_FrameError\n", stderr);
      break;
    case IP_Header::hs_IPv6_PacketTooShort:
      fputs ("hs_IPv6_PacketTooShort\n", stderr);
      break;
    case IP_Header::hs_Protocol_Unsupported:
      fputs ("hs_Protocol_Unsupported\n", stderr);
      break;
    case IP_Header::hs_Protocol_PacketTooShort:
      fputs ("hs_Protocol_PacketTooShort\n", stderr);
      break;
    case IP_Header::hs_Protocol_Checksum:
      fputs ("hs_Protocol_Checksum\n", stderr);
      break;
    }
  }

  virtual bool timeout () { // should be called once a second
    fprintf (stderr, "tick...\n");

    if (number < test_count) {
      test (tests[number].info, tests[number].data, tests[number].size);
      ++number;
    } else {
      IP_Manager::manager().stop ();
    }
    return true; // keep going
  }
};

void interrupt (int /* dummy */) {
  IP_Manager::manager().stop ();
}

int main (int /* argc */, char ** /* argv */) {
  signal (SIGINT, interrupt);

  Uino uino;

  IP_Manager & IP = IP_Manager::manager ();
  
  IP_SerialChannel ser0("/dev/ttyACM0");
  IP.channel_add (&ser0);

  IP_Connection con;
  IP.connection_add (&con);

  IP_Timer timer(&uino);  // set up a periodic callback
  timer.start (IP, 1000); // once a second

  IP.run (); // runs forever

  fprintf (stderr, "\nbye bye!\n");

  return 0;
}

