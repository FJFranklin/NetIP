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

#ifndef __ip_connection_hh__
#define __ip_connection_hh__

#include "ip_buffer.hh"
#include "ip_timer.hh"

class IP_Channel;

class IP_Connection : public Link, public IP_TimerClient {
public:
  /* The EventListener interface provides a mechanism for sending data in whole-packet form rather than
   * using IP_Connection's default streaming.
   */
  class EventListener {
  public:
    /* sent every time a buffer is received
     * 
     * if buffer_received() returns false, the buffer will be processed as normal;
     * if it returns true, the buffer will be treated as if already processed.
     */
    virtual bool buffer_received (const IP_Connection & connection, const IP_Buffer & buffer) = 0;

    /* sent in response to a call to request_to_send()
     * 
     * if buffer_to_send() returns false, the buffer will be discarded
     * if it returns true, the buffer will be finalised and sent (ignoring the FIFO).
     */
    virtual bool buffer_to_send (const IP_Connection & connection, IP_Buffer & buffer) = 0;

    virtual ~EventListener () {
      // ...
    }
  };

private:
  IP_Timer     timer;

  IP_Buffer * buffer_in;

  u16_t data_in_offset;
  u16_t data_in_length;

  u8_t fifo_read_buffer[IP_Connection_FIFO];
  FIFO fifo_read;

  u8_t fifo_write_buffer[IP_Connection_FIFO];
  FIFO fifo_write;

  EventListener * EL;

  bool bSendRequested;

#if 0
  /* IP header - we support one or the other, not both
   */
#if IP_USE_IPv6
    struct IP_Header_IPv6 ip;
#else
    struct IP_Header_IPv4 ip;
#endif

  /* Protocol-specific fields
   */
  union {
    struct IP_Header_TCP  tcp;
    struct IP_Header_UDP  udp;
    struct IP_Header_ICMP icmp;
  } proto;

  struct {
    ns32_t rcv_nxt;     /**< The sequence number that we expect to receive next. */
    ns32_t snd_nxt;     /**< The sequence number that was last sent by us. */

    u16_t length;       /**< Length of the data that was previously sent. */
    u16_t mss;          /**< Current maximum segment size for the connection. */
    u16_t mss_initial;  /**< Initial maximum segment size for the connection. */

    u8_t sa;            /**< Retransmission time-out calculation state variable. */
    u8_t sv;            /**< Retransmission time-out calculation state variable. */
    u8_t rto;           /**< Retransmission time-out. */
    u8_t tcpstateflags; /**< TCP state and flags. */
    u8_t timer;         /**< The retransmission timer. */
    u8_t nrtx;          /**< The number of retransmissions for the last segment sent. */
  } tcp;
#endif
  IP_Address remote;

  ns16_t port_local;    // port 0 is reserved; can't send to or receive at; it can be used as a do-not-reply
  ns16_t port_remote;

  u16_t flags; // internal flags

#define IP_Connection_Open             0x8000
#define IP_Connection_Busy             0x4000
#define IP_Connection_Protocol_TCP     0x2000
#define IP_Connection_RemoteSpecified  0x1000
#define IP_Connection_TimeoutSet       0x0800

public:
  inline bool is_open () const {
    return (flags & IP_Connection_Open);
  }
  inline bool is_busy () const {
    return (flags & IP_Connection_Busy);
  }
  inline bool is_TCP () const {
    return (flags & IP_Connection_Protocol_TCP);
  }
  inline bool has_remote () const {
    return (flags & IP_Connection_RemoteSpecified);
  }
  inline bool timeout_set () const {
    return (flags & IP_Connection_TimeoutSet);
  }

  inline u16_t read (u8_t * ptr, u16_t length) {
    // if (is_open ())
    return fifo_read.read (ptr, length); // TODO
  }
  inline u16_t write (const u8_t * ptr, u16_t length) {
    // if (is_open ())
    return fifo_write.write (ptr, length); // TODO
  }
  inline u16_t print (const char * str) {
    return write ((const u8_t *) str, strlen (str));
  }

  /* Note: reset() closes the connection and doesn't open the new one.
   */
  void reset (IP_Protocol p = p_TCP, u16_t port = 0);

  /* Note: A new connection is created, but not opened.
   */
  IP_Connection (IP_Protocol p = p_TCP, u16_t port = 0) :
    timer(this),
    buffer_in(0),
    fifo_read(fifo_read_buffer, IP_Connection_FIFO),
    fifo_write(fifo_write_buffer, IP_Connection_FIFO),
    EL(0),
    bSendRequested(false)
  {
    reset (p, port);
  }

  virtual ~IP_Connection () {
    // ...
  }

  inline void set_event_listener (EventListener * listener) {
    EL = listener;
  }

  inline void request_to_send () { // get a buffer ready for output, then notify
    if (is_open () && has_remote ()) {
      bSendRequested = true;
    }
  }

  void update (); // internal management of connection & buffers - call frequently!

private:
  bool open_tcp ();
  bool open_udp ();
public:
  /* Note: Open connection
   */
  bool open ();

  /* Note: Close connection
   */
  void close ();

  /* Note: Returns true if the connection is listening on local port
   *       ignoring open/busy state; this function is to help IP_Manager
   *       allocate unique port numbers in the dynamic range.
   */
  inline bool listening (const ns16_t & port) const {
    return (port_local && (port_local == port));
  }

private:
  bool accept_tcp (IP_Buffer * buffer, bool bNewConnection);
  bool accept_udp (IP_Buffer * buffer);
public:
  /* Note: Returns true if the connection can & will handle the incoming buffer
   */
  bool accept (IP_Buffer * buffer);

  /* Note: Open connection to remote address/port
   */
  bool connect (const IP_Address & remote_address, u16_t remote_port);

protected:
  virtual bool timeout (); // return true if the timer should be reset & retained
};

#endif /* ! __ip_connection_hh__ */
