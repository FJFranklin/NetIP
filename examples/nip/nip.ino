#include <NetIP.hh>
#include <Arduino.h>
#include <EEPROM.h>

class Uino : public IP_TimerClient {
  IP_Connection * udp;
  bool bOn;
public:
  Uino (IP_Connection * con) :
    udp(con),
    bOn(false)
  {
    pinMode (LED_BUILTIN, OUTPUT);
    digitalWrite (LED_BUILTIN, LOW);
  }

  virtual bool timeout () { // should be called once a second
    if (bOn) {
      digitalWrite (LED_BUILTIN, LOW);
      bOn = false;
      udp->print ("tock");
    } else {
      digitalWrite (LED_BUILTIN, HIGH);
      bOn = true;
      udp->print ("tick");
    }
    // Serial.print (millis ());
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
  udp.connect (IP.gateway, 0xBCCB); // 48331 (in the range 48130-48555 currently unassigned by IANA)
  IP.connection_add (&udp);

  Uino uino(&udp);

  IP_Timer timer(&uino);  // set up a periodic callback
  timer.start (IP, 1000); // once a second

  IP.run (); // runs forever
}

void loop() {
  // unreachable
}

