/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// FlashAccessor.h

#ifndef FLASHACCESSOR_H_
#define FLASHACCESSOR_H_

#include <stdint.h>  // uint8_t

// picoのフラッシュ2Mのうちの最後のセクタ(4KByte)の先頭アドレス
#define FLASH_TARGET_OFFSET 0x1ff000  // 2097152 - 4096 = 2093056 (== 0x1ff000)

class FlashAccessor {
   public:
    void erase_last_sector();
    void write_last_sector(uint8_t* ptr);
    void read_last_sector(uint8_t* dst);

   private:
    static void call_flash_range_erase(
        void* param);  // Cの関数 flash_safe_execute()に引数として渡すために
                       // static
    static void call_flash_range_program(
        void* param);  // Cの関数 flash_safe_execute()に引数として渡すために
                       // static
};

#endif  // FLASHACCESSOR_H_
