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

#ifndef __ip_arch_hh__
#define __ip_arch_hh__

#include <Arduino.h>

/* Fundamental types for 8-, 16- & 32-bit variables
 */
typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned long  u32_t;

static void ip_arch_usleep (u16_t us) {
  // do nothing
}

static inline u32_t ip_arch_millis () {
  return millis ();
}

#if 0

__AVR_ATmega32U4__  // "Teensy 2.0"
__AVR_AT90USB1286__ // "Teensy++ 2.0"
__MK20DX128__       // "Teensy 3.0"
__MK20DX256__       // "Teensy 3.2" (also Teensy 3.1)
__MKL26Z64__        // "Teensy LC"
__MK64FX512__       // "Teensy 3.5"
__MK66FX1M0__       // "Teensy 3.6"

ARDUINO_AVR_YUN              // "Yun"
ARDUINO_AVR_UNO              // "Uno"
ARDUINO_AVR_DUEMILANOVE      // "Duemilanove"
ARDUINO_AVR_NANO             // "Nano"
ARDUINO_AVR_MEGA2560         // "Mega 2560"
ARDUINO_AVR_MEGA             // "Mega"
ARDUINO_AVR_ADK              // "Mega Adk"
ARDUINO_AVR_LEONARDO         // "Leonardo"
ARDUINO_AVR_LEONARDO_ETH     // "Leonardo Ethernet"
ARDUINO_AVR_MICRO            // "Micro"
ARDUINO_AVR_ESPLORA          // "Esplora"
ARDUINO_AVR_MINI             // "Mini"
ARDUINO_AVR_ETHERNET         // "Ethernet"
ARDUINO_AVR_FIO              // "Fio"
ARDUINO_AVR_BT               // "BT" (Bluetooth)
ARDUINO_AVR_LILYPAD_USB      // "Lilypad USB"
ARDUINO_AVR_LILYPAD          // "Lilypad"
ARDUINO_AVR_PRO              // "Pro"
ARDUINO_AVR_NG               // "NG"
ARDUINO_AVR_ROBOT_CONTROL    // "Robot Control"
ARDUINO_AVR_ROBOT_MOTOR      // "Robot Motor"
ARDUINO_AVR_GEMMA            // "Gemma"
ARDUINO_AVR_CIRCUITPLAY      // "Circuit Playground"
ARDUINO_AVR_YUNMINI          // "Yun Mini"
ARDUINO_AVR_INDUSTRIAL101    // "Industrial 101"
ARDUINO_AVR_LININO_ONE       // "Linino One"
ARDUINO_AVR_UNO_WIFI_DEV_ED  // "Arduino Uno WiFi Developer Edition"

ARDUINO_AVR_ENVIRODIY_MAYFLY // "EnviroDIY Mayfly Data Logger"
ARDUINO_AVR_SODAQ_MBILI      // "SODAQ Mbili"
ARDUINO_AVR_DIGISPARKPRO     // "digiStump Digispark Pro"
ARDUINO_AVR_DIGISPARK        // "digiStump Digispark"
ARDUINO_AVR_TRE              // "Tre"

ARDUINO_SAM_DUE              // "Due"
ARDUINO_SAMD_ZERO            // "Zero"
ARDUINO_ARC32_TOOLS          // "101"

#endif

#endif /* ! __ip_arch_hh__ */
