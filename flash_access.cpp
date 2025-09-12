/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlashAccessor.h"
#include "PaddedData.h"
#include "UsbCDC.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

extern const char flash_initialized_msg[];

void cdc_task(PaddedData* p);

int main() {
    stdio_init_all();

    FlashAccessor fa;  // Flashメモリーにアクセスするためのラッパークラス
    PaddedData pd;  // Flashメモリーの最後のセクタに対応するワークエリア(RAM)
    UsbCDC uc;  // USB-CDCで通信するためのラッパークラス

    assert(sizeof(PaddedData) ==
           FLASH_SECTOR_SIZE);  // 4096バイトでなければならない

    //    Flashメモリーの定数エリアを初期化したい場合は、この関数を有効にして実行した後に止める、そして無効にしてビルドし直す
    //    fa.erase_last_sector();

    // Flashから読んでみる
    fa.read_last_sector((uint8_t*)&pd);

    const char flash_initialized_msg[] =
        "Pico Flash constant area initialized.";

    // 一番最初はこの if() の中を通る
    if (strcmp(flash_initialized_msg, (const char*)pd.data.initialized_msg) !=
        0) {
        strcpy((char*)pd.data.initialized_msg, flash_initialized_msg);
        strncpy(pd.data.id, "First ID", ID_LENGTH);
        pd.data.id[ID_LENGTH] = '\0';
        strncpy(pd.data.password, "First PW", PASSWORD_LENGTH);
        pd.data.password[PASSWORD_LENGTH] = '\0';
        pd.data.int_val = 98765;
        pd.data.double_val = 3.141592654;

        fa.write_last_sector((uint8_t*)&pd);

        // Flashから読んでみる
        fa.read_last_sector((uint8_t*)&pd);

        printf("Flash memory initialized.\r\n");
    }
    // 終端しておく
    pd.data.id[ID_LENGTH] = '\0';
    pd.data.password[PASSWORD_LENGTH] = '\0';

    printf("ID=%s, password=%s, int_val=%d, double_val=%f\r\n", pd.data.id,
           pd.data.password, pd.data.int_val, pd.data.double_val);

    uc.init();

    while (true) {
        uc.loop(&pd);
    }
}
