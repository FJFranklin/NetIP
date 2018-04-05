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

#ifndef __ip_manager_hh__
#define __ip_manager_hh__

#include "ip_connection.hh"
#include "ip_channel.hh"
#include "ip_timer.hh"

class IP_UDP_Connection;

class IP_Manager : public IP_Clock, public IP_TimerClient {
public:
  class Listener {
  public:
    virtual void debug_print (const char * message) = 0;
    virtual void pong (const IP_Address & address, u32_t round_trip, u16_t seq_no) = 0;

    virtual ~Listener () {
      // ...
    }
  };

private:
  u8_t channel_register[127];

  Listener * EL;

  IP_Buffer buffers[IP_Buffer_Extras];

  Chain<IP_Buffer> chain_buffers_spare;
  Chain<IP_Buffer> chain_buffers_pending;

  Chain<IP_Connection> chain_connection; // IP connections across network
  Chain<IP_Channel>    chain_channel;    // Hardware connections to neighbouring devices

  IP_Timer timer;         // timer for broadcast ping
  u16_t    ping_interval; // how often to broadcast ping on local network

  u16_t  ping_next; // counter for generating ping sequence numbers
  u16_t  last_port; // counter for generating free port numbers

  u8_t   ticker;    // internal cooperative management

public:
  IP_Address host;
  IP_Address gateway;
  IP_Address netmask;

  static IP_Manager & manager ();

  IP_Manager ();

  ~IP_Manager () {
    // ...
  }

  inline void set_event_listener (Listener * listener) {
    EL = listener;
  }

  inline void debug_print (const char * message) {
    if (EL && message) {
      EL->debug_print (message);
    }
  }

  inline void connection_add (IP_Connection * connection) {
    chain_connection.chain_prepend (connection);
  }

  inline void connection_remove (IP_Connection * connection) {
    chain_connection.chain_remove (connection);
  }

  IP_Connection * connection_for_port (const ns16_t & port); // returns 0 if none found

  bool channel_add (IP_Channel * channel); // Note: add up to 15 channels; no option to remove channels.

  IP_Channel * channel (u8_t number);

  inline bool is_local_network (const IP_Address & address) const {
    return host.compare (address, netmask);
  }

  u16_t available_port ();

  /* 
   * returns false if unable to queue (i.e., if no spare buffer for the exchange)
   */
  bool queue (IP_Buffer *& buffer);

  /* 
   * adds a free buffer to the spares
   */
  inline void add_to_spares (IP_Buffer * buffer) {
    if (buffer) {
      if (!buffer->retained ()) {
	// fprintf (stderr, ">> %p\n", buffer);
	chain_buffers_spare.chain_prepend (buffer);
      }
    }
  }

  /* 
   * gets a free buffer from the spares - if there are any
   */
  inline IP_Buffer * get_from_spares () {
    IP_Buffer * B = chain_buffers_spare.chain_pop ();
    // fprintf (stderr, "<< %p\n", B);
    return B;
  }

  void ping (const IP_Address & address, u16_t seq_no);

private:
  u16_t ping_seq_no () {
    return ++ping_next;
  }

  void connection_handover (IP_Buffer * buffer);

  enum RoutingInfo {
    ri_InvalidAddress = 0, // reserved network address, or channel not registered
    ri_Broadcast_Local,    // local network broadcast    
    ri_Destination_Self,   // that's us!
    ri_Destination_Local,  // route through local network to final destination
    ri_Gateway_Self,       // we're the gateway - route to external network
    ri_Gateway_Local       // route through local network to gateway
  };

  void broadcast (IP_Buffer * buffer);
public:
  void forward (IP_Buffer * buffer);
private:
  void register_source (u8_t channel, const IP_Address & source);

  RoutingInfo channel_for_destination (u8_t & channel, const IP_Address & destination) const;

  /* clock functions
   */
  void tick ();
  void every_millisecond ();
  void every_second ();

  virtual bool timeout (); // return true if the timer should be reset & retained
};

#endif /* ! __ip_manager_hh__ */
