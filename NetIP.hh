/*! \file NetIP.hh
    \brief The library header file to be included by Arduino sketches.
    
    NetIP class headers are kept in the netip sub-folder, and are not visible to the Arduino IDE.
    This top-level header file includes all the relevant NetIP class headers for Arduino sketches.
*/

#include "netip/ip_manager.hh"
#include "netip/ip_serial.hh"

/* ==== End of Header File - the rest is documentation ==== */

/*! \mainpage NetIP Index Page
 *
 * \section intro_sec Introduction
 *
 * NetIP enables multiple Unix- and Arduino-type devices to be linked into a network using only
 * device-to-device serial connections. A subset of TCP/IP is supported, along with UDP and ICMP
 * Echo Request / Reply (i.e., ping).
 *
 * The principal aims of NetIP are:
 * - a clean, flexible, programmer-friendly interface (which is, inevitably, subjective)
 * - minimal use of program and dynamic memory (with the aim of working even on the Uno)
 * - no dynamic allocation of memory (to avoid problems with memory fragmentation)
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *  
 * etc...
 */
