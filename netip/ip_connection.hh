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

#include "ip_header.hh"
#include "ip_timer.hh"

class IP_Channel;

class IP_Connection : public Link, public IP_TimerClient {
private:
  IP_Header    header;
  IP_Timer     timer;

  IP_Buffer * buffer_in;
  IP_Buffer * buffer_out;

  u16_t data_in_offset;
  u16_t data_out_offset;

  u8_t fifo_read_buffer[IP_Connection_FIFO];
  FIFO fifo_read;

  u8_t fifo_write_buffer[IP_Connection_FIFO];
  FIFO fifo_write;

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

  u16_t flags; // internal flags

#define IP_Connection_Open             0x1000
#define IP_Connection_LocalSpecified   0x2000
#define IP_Connection_RemoteSpecified  0x4000
#define IP_Connection_TimeoutSet       0x8000

public:
  inline u16_t read (u8_t * ptr, u16_t length) {
    return fifo_read.read (ptr, length); // TODO
  }
  inline u16_t write (const u8_t * ptr, u16_t length) {
    return fifo_write.write (ptr, length); // TODO
  }

  /* Note: reset() closes the connection and doesn't open the new one.
   */
  void reset (IP_Header::Protocol p = IP_Header::p_TCP, u16_t port = 0); // port 0 => find an unused port in range 4096-32000

  /* Note: A new connection is created, but not opened.
   */
  IP_Connection (IP_Header::Protocol p = IP_Header::p_TCP, u16_t port = 0) :
    timer(this),
    buffer_in(0),
    buffer_out(0),
    fifo_read(fifo_read_buffer, IP_Connection_FIFO),
    fifo_write(fifo_write_buffer, IP_Connection_FIFO)
  {
    reset (p, port);
  }

  virtual ~IP_Connection () {
    // ...
  }

  /* Note: Open connection
   */
  void open ();

  /* Note: Close connection
   */
  void close ();

  /* Note: Returns true if the connection is listening on local port
   */
  inline bool listening (const ns16_t & port) const {
    return ((flags & IP_Connection_Open) /* && (header.port_source == port) */);
  }

  /* Note: Returns true if the connection can & will handle the incoming buffer
   */
  bool accept (IP_Buffer * buffer);

  /* Note: Open connection to remote address/port
   */
  bool connect (const IP_Address & remote_address, u16_t remote_port);

  void update () { // opportunity for low-level management; not time-specific
    // ...
  }

protected:
  virtual bool timeout (); // return true if the timer should be reset & retained
};

#endif /* ! __ip_connection_hh__ */
