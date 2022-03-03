/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus/)
 * Copyright (c) 2017 Liviu Ionescu.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose is hereby granted, under the terms of the MIT license.
 *
 * If a copy of the license was not distributed with this file, it can
 * be obtained from https://opensource.org/licenses/MIT/.
 */

#if defined(__riscv)

// ----------------------------------------------------------------------------

#if defined(MICRO_OS_PLUS_INCLUDE_CONFIG_H)
#include <micro-os-plus/config.h>
#endif // MICRO_OS_PLUS_INCLUDE_CONFIG_H

#if defined(MICRO_OS_PLUS_TRACE)

#if defined(MICRO_OS_PLUS_USE_TRACE_UART0)

#include <micro-os-plus/device.h>
#include <micro-os-plus/diag/trace.h>

#define MICRO_OS_PLUS_INTEGER_TRACE_UART0_BAUD_RATE (115200)

#if defined(SIFIVE_E31ARTY)
#define UART_TXCTRL_TXEN SIFIVE_E31ARTY_UART_TXCTRL_TXEN
#define UART_TXDATA_FULL SIFIVE_E31ARTY_UART_TXDATA_FULL
#elif defined(SIFIVE_E51ARTY)
#define UART_TXCTRL_TXEN SIFIVE_E51ARTY_UART_TXCTRL_TXEN
#define UART_TXDATA_FULL SIFIVE_E51ARTY_UART_TXDATA_FULL
#else
#error "Unsupported device"
#endif

// ----------------------------------------------------------------------------

namespace micro_os_plus
{
  namespace trace
  {
    // ------------------------------------------------------------------------

    void
    initialize (void)
    {
      // Set baud rate.
      // Arty boards have the peripheral clock at half the core clock.
      UART0->div = (riscv::core::running_frequency_hz () / 2
                    / MICRO_OS_PLUS_INTEGER_TRACE_UART0_BAUD_RATE)
                   - 1;
      // Enable transmitter.
      UART0->txctrl |= UART_TXCTRL_TXEN;

      // Wait a bit to avoid corruption on the UART.
      // (In some cases, switching to the IOF can lead
      // to output glitches, so need to let the UART
      // receiver time out and resynchronize to the real
      // start of the stream.
      volatile int i = 0;
      while (i < 10000)
        {
          i++;
        }
    }

    // ------------------------------------------------------------------------

    ssize_t
    write (const void* buf, std::size_t nbyte)
    {
      if (buf == nullptr || nbyte == 0)
        {
          return 0;
        }

      const char* cbuf = (const char*)buf;

      for (size_t i = 0; i < nbyte; i++)
        {
          uint8_t ch = (*cbuf++);

          if (ch == '\n')
            {
              // Wait until FIFO is ready...
              // Without handshake, should not block.
              while ((UART0->txdata & UART_TXDATA_FULL) != 0)
                {
                  // Busy wait!
                }
              UART0->txdata = '\r';
            }

          // Wait until FIFO is ready...
          while ((UART0->txdata & UART_TXDATA_FULL) != 0)
            {
              // Busy wait!
            }
          UART0->txdata = ch;
        }

      // All characters successfully sent.
      return (ssize_t)nbyte;
    }

    // ------------------------------------------------------------------------

    void
    flush (void)
    {
      // TODO flush port
    }

    // ------------------------------------------------------------------------
  } // namespace trace
} // namespace micro_os_plus

#endif // defined(MICRO_OS_PLUS_USE_TRACE_UART0)
#endif // defined(MICRO_OS_PLUS_TRACE)

// ----------------------------------------------------------------------------

#endif // defined(__riscv)

// ----------------------------------------------------------------------------
