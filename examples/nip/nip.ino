#include <NetIP.hh>
#include <Arduino.h>
#include <EEPROM.h>

class Uino : public IP_TimerClient, public IP_Connection::EventListener {
  IP_Connection * udp;
  bool bOn;
  bool bConnected;

public:
  Uino (IP_Connection * con) :
    udp(con),
    bOn(false),
    bConnected(false)
  {
    udp->set_event_listener (this);

    pinMode (LED_BUILTIN, OUTPUT);
    digitalWrite (LED_BUILTIN, LOW);
  }

  virtual void connection_has_opened () {
    bConnected = true;
  }

  virtual void connection_has_closed () {
    bConnected = false;
  }

  virtual bool buffer_received (const IP_Connection & connection, const IP_Buffer & buffer_incoming) {
    // ...
    return false; // we don't handle these
  }

  virtual bool buffer_to_send (const IP_Connection & connection, IP_Buffer & buffer_outgoing) {
    if (bOn) {
      buffer_outgoing.append ("    ... tock.");
    } else {
      buffer_outgoing.append ("tick...");
    }
    return true; // we do handle these
  }

  virtual bool timeout () { // should be called once a second
    if (!bConnected) {
      udp->connect (IP_Manager::manager().gateway, 0xBCCB); // 48331 (in the range 48130-48555 currently unassigned by IANA)
    }
    if (bConnected) {
      udp->request_to_send ();
    }

    if (bOn) {
      digitalWrite (LED_BUILTIN, LOW);
      bOn = false;
      udp->print ("The quick brown fox jumps over the lazy dog.");
    } else {
      digitalWrite (LED_BUILTIN, HIGH);
      bOn = true;
      udp->print ("Aberystwyth, mon amour.");
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

  IP_Connection udp(p_UDP, IP.available_port ());
  IP.connection_add (&udp);

  Uino uino(&udp);

  IP_Timer timer(&uino);  // set up a periodic callback
  timer.start (IP, 1000); // once a second

  IP.run (); // runs forever
}

void loop() {
  // unreachable
}

