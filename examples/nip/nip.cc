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

#include <stdlib.h>
#include <signal.h>

#include "tests.hh"

class Uino : public IP_TimerClient, public IP_Connection::EventListener, public IP_Manager::Listener {
private:
  u8_t buffer[32];

  IP_Address device;

  IP_Connection * udp;
  IP_Connection * tcp_server;
  IP_Connection * tcp_client;

  u8_t  remote;
  u8_t  number;
  u16_t pingno;

  bool bTesting;
  bool bOn;
  bool bFirst;
  bool bConnected_UDP;
  bool bConnected_TCP_Server;
  bool bConnected_TCP_Client;

public:
  Uino (IP_Connection * con, IP_Connection * server, IP_Connection * client, u8_t remote_id, bool bTest) :
    udp(con),
    tcp_server(server),
    tcp_client(client),
    remote(remote_id),
    number(0),
    pingno(0),
    bTesting(bTest),
    bOn(false),
    bFirst(true),
    bConnected_UDP(false),
    bConnected_TCP_Server(false),
    bConnected_TCP_Client(false)
  {
    device = IP_Manager::manager().host;
    device.set_local_network_id (remote);

    udp->set_event_listener (this);
    udp->open (); // just listen; don't connect

    tcp_server->set_event_listener (this);
    tcp_server->open (); // just listen; don't connect

    tcp_client->set_event_listener (this);
  }

  virtual void debug_print (const char * message) {
    fputs (message, stderr);
  }

  virtual void pong (const IP_Address & address, u32_t round_trip, u16_t seq_no) {
    fprintf (stderr, "ping (%02x) %lu ms [#%u]\n", (unsigned) address.local_network_id (), (unsigned long) round_trip, (unsigned) seq_no);
  }

  virtual void connection_has_data (const IP_Connection & connection) {
    if (connection.is_TCP ()) {
      if (connection.tcp_server ()) {
	// ...
      } else {
	// ...
      }
    } else {
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
    }
  }

  virtual void connection_has_opened (const IP_Connection & connection) {
    if (connection.is_TCP ()) {
      if (connection.tcp_server ()) {
	fprintf (stderr, "TCP Server: Connection open!\n");
	bConnected_TCP_Server = true;
      } else {
	fprintf (stderr, "TCP Client: Connection open!\n");
	bConnected_TCP_Client = true;
      }
    } else {
      bConnected_UDP = true;
    }
  }

  virtual void connection_has_closed (const IP_Connection & connection) {
    if (connection.is_TCP ()) {
      if (connection.tcp_server ()) {
	bConnected_TCP_Server = false;
      } else {
	bConnected_TCP_Client = false;
      }
    } else {
      bConnected_UDP = false;
    }
  }

  virtual bool buffer_received (const IP_Connection & connection, const IP_Buffer & buffer_incoming) {
    bool bHandled = false;

    if (connection.is_TCP ()) {
      if (connection.tcp_server ()) {
	// ...
      } else {
	// ...
      }
    } else {
      u16_t offset = buffer_incoming.udp_data_offset ();
      u16_t length = buffer_incoming.udp_data_length ();

      fprintf (stderr, "==========> \"");

      for (u16_t c = 0; c < length; c++) {
	fputc (buffer_incoming[offset+c], stderr);
      }
      fprintf (stderr, "\"\n");

      bHandled = true;
    }
    return bHandled; // we don't handle these
  }

  virtual bool buffer_to_send (const IP_Connection & connection, IP_Buffer & buffer_outgoing) {
    bool bHandled = false;

    if (connection.is_TCP ()) {
      if (connection.tcp_server ()) {
	// ...
      } else {
	// ...
      }
    } else {
      // ...
    }
    return bHandled; // we don't handle these
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
    if (bFirst) {
      bFirst = false;

      tcp_client->connect (device, 0xBCCB); // 48331 (in the range 48130-48555 currently unassigned by IANA)
    }

    IP_Manager::manager().ping (device, pingno++);

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

    return true; // keep going
  }
};

void interrupt (int /* dummy */) {
  IP_Manager::manager().stop ();
}

int main (int argc, char ** argv) {
  bool bTesting = false;

  const char * device = "/dev/ttyACM0";

  u8_t id = 0x0a; // 0x77 for the UNO

  for (int arg = 1; arg < argc; arg++) {
    if (strcmp (argv[arg], "--help") == 0) {
      fprintf (stderr, "\nnip [--help] [--test] [/dev/<ID>]\n\n");
      fprintf (stderr, "  --help         Display this help.\n");
      fprintf (stderr, "  --test         Sample IP packet testing.\n");
      fprintf (stderr, "  --remote=<id>  Specify local network id [1-254] of the remote device.\n");
      fprintf (stderr, "  /dev/<ID>      Connect to /dev/<ID> instead of default [/dev/ttyACM0].\n\n");
      return 0;
    }
    if (strncmp (argv[arg], "/dev/", 5) == 0) {
      device = argv[arg];
    } else if (strcmp (argv[arg], "--test") == 0) {
      bTesting = true;
    } else if (strncmp (argv[arg], "--remote=", 9) == 0) {
      int rid = atoi (argv[arg] + 9);
      if ((rid > 0) && (rid < 255)) {
	id = rid;
      } else {
	fprintf (stderr, "local network id for remote device must be in the range 1-254\n");
	return -1;
      }
    } else {
      fprintf (stderr, "nip [--help] [--test] [--remote=<id>] [/dev/<ID>]\n");
      return -1;
    }
  }

  signal (SIGINT, interrupt);

  IP_Manager & IP = IP_Manager::manager ();
  
  IP_SerialChannel ser0(device);
  IP.channel_add (&ser0);

  IP_Connection udp(p_UDP, 0xBCCB);
  IP.connection_add (&udp);

  IP_Connection tcp_server(p_TCP, 0xBCCB);
  IP.connection_add (&tcp_server);

  IP_Connection tcp_client(p_TCP, IP.available_port ());
  IP.connection_add (&tcp_client);

  Uino uino(&udp, &tcp_server, &tcp_client, id, bTesting);

  IP_Timer timer(&uino);  // set up a periodic callback
  timer.start (IP, 1000); // once every second

  IP.set_event_listener (&uino);

  IP.run (); // runs forever

  fprintf (stderr, "\nbye bye!\n");

  return 0;
}
