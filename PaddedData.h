// PaddedData.h

#ifndef PADDEDDATA_H_
#define PADDEDDATA_H_

#include <stdint.h>  // uint8_t

#include "hardware/flash.h"
#include "pico/flash.h"

#define PASSWORD_LENGTH 8
#define ID_LENGTH 8

// 定数を格納する構造体
// この構造体の容量は 4095 以下でなければならない
// 最低でも PaddedDataのpadding[]配列に 1バイトを確保する必要があるため

// パディングはコンパイラがやってくれるそうです
// メモリー効率を意識せずに いろいろな型を配置しています

const char flash_initialized_msg[] = "Pico Flash constant area initialized.";

struct ConfigData {
    uint8_t initialized_msg[count_of(flash_initialized_msg)];
    char password[PASSWORD_LENGTH + 1];
    char id[ID_LENGTH + 1];
    int int_val;
    double double_val;
};

// 全体で4096バイト（1セクター分の容量）になるように残りをuint8_t型の配列で埋める
// これにより（上限さえ越えなければ）ConfigData構造体のサイズを気にせずに常に4096バイトを書けば良い

struct PaddedData {
    ConfigData data;
    uint8_t padding[FLASH_SECTOR_SIZE - sizeof(struct ConfigData)];
};

#endif  // PADDEDDATA_H_
