/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// UsbCDC.h

#ifndef USBCDC_H_
#define USBCDC_H_

#include <stdint.h>  // uint8_t

#include "Stack.h"
#include "StringParser.h"

class UsbCDC {
   public:
    UsbCDC();
    void init();
    void loop(PaddedData* p);

   private:
    Stack st_;
    StringParser sp_;
    void clear_buffer();
    void cleanup_if_needed();
};

#endif  // USB_CDC_H_
