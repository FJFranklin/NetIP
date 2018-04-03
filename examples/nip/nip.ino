#include <NetIP.hh>
#include <Arduino.h>
#include <EEPROM.h>

class Uino : public IP_TimerClient, public IP_Connection::EventListener {
  IP_Connection * udp;
  IP_Connection * tcp_server;
  IP_Connection * tcp_client;

  bool bOn;
  bool bFirst;
  bool bConnected_UDP;
  bool bConnected_TCP_Server;
  bool bConnected_TCP_Client;

public:
  Uino (IP_Connection * con, IP_Connection * server, IP_Connection * client) :
    udp(con),
    tcp_server(server),
    tcp_client(client),
    bOn(false),
    bFirst(true),
    bConnected_UDP(false),
    bConnected_TCP_Server(false),
    bConnected_TCP_Client(false)
  {
    udp->set_event_listener (this);
    udp->open (); // just listen; don't connect

    tcp_server->set_event_listener (this);
    tcp_server->open (); // just listen; don't connect

    tcp_client->set_event_listener (this);

    pinMode (LED_BUILTIN, OUTPUT);
    digitalWrite (LED_BUILTIN, LOW);
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
      // ...
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
      if (bOn) {
	buffer_outgoing.append ("    ... tock.");
      } else {
	buffer_outgoing.append ("tick...");
      }
      bHandled = true; // we do handle these
    }
    return bHandled;
  }

  virtual bool timeout () { // should be called once a second
    if (bFirst) {
      bFirst = false;

      udp->connect (IP_Manager::manager().gateway, 0xBCCB);        // 48331 (in the range 48130-48555 currently unassigned by IANA)
      tcp_client->connect (IP_Manager::manager().gateway, 0xBCCB);
    }
    if (bConnected_UDP) {
      udp->request_to_send ();
    }

    if (bOn) {
      digitalWrite (LED_BUILTIN, LOW);
      bOn = false;
      if (bConnected_UDP) {
	udp->print ("The quick brown fox jumps over the lazy dog.");
      }
    } else {
      digitalWrite (LED_BUILTIN, HIGH);
      bOn = true;
      if (bConnected_UDP) {
	udp->print ("Aberystwyth, mon amour.");
      }
    }
    return true; // keep going
  }
};

void setup () {
  Serial.begin (115200);

  IP_Manager & IP = IP_Manager::manager ();

  IP.host.set_local_network_id (EEPROM.read (0));

  IP_SerialChannel ser0(Serial);
  IP.channel_add (&ser0);

  IP_Connection udp(p_UDP, 0xBCCB);
  IP.connection_add (&udp);

  IP_Connection tcp_server(p_TCP, 0xBCCB);
  IP.connection_add (&tcp_server);

  IP_Connection tcp_client(p_TCP, IP.available_port ());
  IP.connection_add (&tcp_client);

  Uino uino(&udp, &tcp_server, &tcp_client);

  IP_Timer timer(&uino);  // set up a periodic callback
  timer.start (IP, 1000); // once a second

  IP.run (); // runs forever
}

void loop() {
  // unreachable
}

