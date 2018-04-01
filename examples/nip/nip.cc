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

class Uino : public IP_TimerClient, public IP_Connection::EventListener {
private:
  u8_t buffer[32];

  IP_Connection * udp;

  u8_t number;

  bool bTesting;
  bool bOn;
  bool bConnected;

public:
  Uino (IP_Connection * con, bool bTest) :
    udp(con),
    number(0),
    bTesting(bTest),
    bOn(false),
    bConnected(false)
  {
    udp->set_event_listener (this);
    udp->open (); // just listen; don't connect
  }

  virtual void connection_has_opened () {
    bConnected = true;
  }

  virtual void connection_has_closed () {
    bConnected = false;
  }

  virtual bool buffer_received (const IP_Connection & connection, const IP_Buffer & buffer_incoming) {
    u16_t offset = buffer_incoming.udp_data_offset ();
    u16_t length = buffer_incoming.udp_data_length ();
#if 0
    fprintf (stderr, "==========> \"");

    for (u16_t c = 0; c < length; c++) {
      fputc (buffer_incoming[offset+c], stderr);
    }
    fprintf (stderr, "\"\n");
#endif
    // ...
    return false; // we don't handle these
  }
  virtual bool buffer_to_send (const IP_Connection & connection, IP_Buffer & buffer_outgoing) {
    // ...
    return false; // we don't handle these
  }

  void test (const char * info, const u8_t * buffer, u16_t length) {
    fputs ("\n", stderr);
    fputs (info, stderr);
    fputs ("\n", stderr);

    IP_Buffer B;
    B.append (buffer, length);

    switch (B.sniff ()) {
    case IP_Buffer::hs_Okay:
      fputs ("hs_Okay\n", stderr);
      break;
    case IP_Buffer::hs_FrameError:
      fputs ("hs_FrameError\n", stderr);
      break;
    case IP_Buffer::hs_EchoRequest:
      fputs ("hs_EchoRequest\n", stderr);
      break;
    case IP_Buffer::hs_EchoReply:
      fputs ("hs_EchoReply\n", stderr);
      break;
    case IP_Buffer::hs_IPv4:
      fputs ("hs_IPv4\n", stderr);
      break;
    case IP_Buffer::hs_IPv4_FrameError:
      fputs ("hs_IPv4_FrameError\n", stderr);
      break;
    case IP_Buffer::hs_IPv4_PacketTooShort:
      fputs ("hs_IPv4_PacketTooShort\n", stderr);
      break;
    case IP_Buffer::hs_IPv4_Checksum:
      fputs ("hs_IPv4_Checksum\n", stderr);
      break;
    case IP_Buffer::hs_IPv6:
      fputs ("hs_IPv6\n", stderr);
      break;
    case IP_Buffer::hs_IPv6_FrameError:
      fputs ("hs_IPv6_FrameError\n", stderr);
      break;
    case IP_Buffer::hs_IPv6_PacketTooShort:
      fputs ("hs_IPv6_PacketTooShort\n", stderr);
      break;
    case IP_Buffer::hs_Protocol_Unsupported:
      fputs ("hs_Protocol_Unsupported\n", stderr);
      break;
    case IP_Buffer::hs_Protocol_FrameError:
      fputs ("hs_Protocol_FrameError\n", stderr);
      break;
    case IP_Buffer::hs_Protocol_PacketTooShort:
      fputs ("hs_Protocol_PacketTooShort\n", stderr);
      break;
    case IP_Buffer::hs_Protocol_Checksum:
      fputs ("hs_Protocol_Checksum\n", stderr);
      break;
    }
  }

  virtual bool timeout () {
    if (false /* !bConnected */) {
      udp->connect (IP_Manager::manager().gateway, 0xBCCB); // 48331 (in the range 48130-48555 currently unassigned by IANA)
    }

    if (bOn) {
      bOn = false;
      // udp->print ("The quick brown fox jumps over the lazy dog.");
    } else {
      bOn = true;
      // udp->print ("Aberystwyth, mon amour.");
    }

    if (bTesting) {
      if (number < test_count) {
	test (tests[number].info, tests[number].data, tests[number].size);
	++number;
      } else {
	IP_Manager::manager().stop ();
      }
    }

    u16_t count = udp->read (buffer, 32);

    if (count) {
      fprintf (stderr, "UDP-0xBCCB: \"");

      while (count) {
	for (u16_t c = 0; c < count; c++) {
	  fputc (buffer[c], stderr);
	}
	count = udp->read (buffer, 32);
      }
      fprintf (stderr, "\"\n");
    }
    return true; // keep going
  }
};

void interrupt (int /* dummy */) {
  IP_Manager::manager().stop ();
}

int main (int argc, char ** argv) {
  bool bTesting = false;

  const char * device = "/dev/ttyACM0";

  for (int arg = 1; arg < argc; arg++) {
    if (strcmp (argv[arg], "--help") == 0) {
      fprintf (stderr, "\nnip [--help] [--test] [/dev/<ID>]\n\n");
      fprintf (stderr, "  --help     Display this help.\n");
      fprintf (stderr, "  --test     Sample IP packet testing.\n");
      fprintf (stderr, "  /dev/<ID>  Connect to /dev/<ID> instead of default [/dev/ttyACM0].\n\n");
      return 0;
    }
    if (strncmp (argv[arg], "/dev/", 5) == 0) {
      device = argv[arg];
    } else if (strcmp (argv[arg], "--test") == 0) {
      bTesting = true;
    } else {
      fprintf (stderr, "nip [--help] [--test] [/dev/<ID>]\n");
      return -1;
    }
  }

  signal (SIGINT, interrupt);

  IP_Manager & IP = IP_Manager::manager ();
  
  IP_SerialChannel ser0(device);
  IP.channel_add (&ser0);

  IP_Connection con;
  IP.connection_add (&con);

  IP_Connection udp(p_UDP, 0xBCCB);
  IP.connection_add (&udp);

  Uino uino(&udp, bTesting);

  IP_Timer timer(&uino); // set up a periodic callback
  timer.start (IP, 10);  // once every 10 milliseconds

  IP.run (); // runs forever

  fprintf (stderr, "\nbye bye!\n");

  return 0;
}
