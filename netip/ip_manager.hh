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

class IP_Manager : public IP_Clock {
private:
  IP_Buffer buffers[IP_Buffer_Extras];

  Chain<IP_Buffer> chain_buffers_spare;
  Chain<IP_Buffer> chain_buffers_pending;

  Chain<IP_Connection> chain_connection; // IP connections across network
  Chain<IP_Channel>    chain_channel;    // Hardware connections to neighbouring devices

  u16_t  last_port; // counter for generating free port numbers

  ns32_t tcp_iss;   // TCP initial sequence number

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

  inline const ns32_t & iss () const {
    return tcp_iss;
  }

  inline void connection_add (IP_Connection * connection) {
    chain_connection.chain_prepend (connection);
  }

  inline void connection_remove (IP_Connection * connection) {
    chain_connection.chain_remove (connection);
  }

  IP_Connection * connection_for_port (const ns16_t & port); // returns 0 if none found

  void channel_add (IP_Channel * channel); // Note: no option to remove channels!

  IP_Channel * channel (u8_t number);

  inline bool is_local_network (const IP_Address & address) const {
    return host.compare (address, netmask);
  }

  ns16_t available_port ();

  /* 
   * returns false if unable to queue (i.e., if no spare buffer for the exchange)
   */
  bool queue (IP_Buffer *& buffer);

  /* 
   * adds a free buffer to the spares
   */
  inline void add_to_spares (IP_Buffer * buffer) {
    chain_buffers_spare.chain_prepend (buffer);
  }

private:
  void forward (IP_Buffer * buffer);

  /* clock functions
   */
  void tick ();
  void every_millisecond ();
  void every_second ();
};

#endif /* ! __ip_manager_hh__ */
