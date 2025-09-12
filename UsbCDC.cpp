/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "UsbCDC.h"

#include <errno.h>   // errno
#include <stdlib.h>  // strtod()

#include "FlashAccessor.h"
#include "bsp/board_api.h"
#include "cdc_device.h"
#include "tusb.h"
#include "tusb_fifo.h"

UsbCDC::UsbCDC() : sp_(0), st_(64) {}

void UsbCDC::init() {
    board_init();
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE,
                                   .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
}

void UsbCDC::loop(PaddedData* ptr) {
    uint8_t buf[64];
    FlashAccessor fa;
    char* endptr;
    int length;

    tud_task();

    // PCからのキーボード入力待ち
    while (tud_cdc_available()) {
        uint32_t count = tud_cdc_read(buf, sizeof(buf));

        // streamの場合インチキかも知れないが
        // 例えば3バイトのコードは3バイトまとめて届くことを前提にしている

        // ファンクションキー等、不要なキーは読み捨てる
        if (buf[0] == 27) {
            if ((4 == count) || (5 == count)) {
                continue;
            }
        }

        // 文字を入力した後にバックスペースか左矢印キーで左にカーソルを移動した場合
        // 右矢印キーで前に入力した値を復活できるようにスタックを使う
        if ((count == 1) && (buf[0] == BS)) {  // バックスペースキー
            if (sp_.has_data()) {
                st_.push(sp_.get_char_with_dec_ptr());  // 退避しておく
                continue;
            }
        } else if (count == 1) {
            if ((32 <= buf[0]) && (buf[0] <= 126)) {
                ;  // 取り込む値
                if (!st_.is_empty()) {
                    st_.pop();  // 取り込んだ分を捨てる
                }
            } else {
                continue;
            }
        }
        // 右矢印キーで前に入力した値を復活できるようにスタックを使う
        if (count == 3) {
            if ((buf[0] == 27) && (buf[1] == 91)) {
                if (buf[2] == 67) {  // 右矢印(→)キーは3バイト
                    if (!st_.is_empty()) {
                        sp_.append(st_.pop());  // 復旧する
                        continue;
                    } else {
                        // カーソルが右に移動してしまうのでスタックが空なら
                        // 半角スペースを追加する
                        sp_.append(' ');
                    }
                    continue;
                } else if (buf[2] == 68) {  // 左矢印(←)キーは3バイト
                    if (sp_.has_data()) {
                        st_.push(sp_.get_char_with_dec_ptr());  // 退避しておく
                    }
                    continue;
                }
            } else if ((buf[0] == 27) &&
                       (buf[1] == 79)) {  // テンキーの numlock, /, *, -
                                          // キーは3バイトで来るので捨てる
                continue;
            }
        }

        sp_.add(buf, count);
        sp_.search_stx('#');
        if (sp_.has_stx()) {
            switch (sp_.get_command_first()) {
                case StringParser::Id:
                    if (4 <= sp_.length() && sp_.search_etx(CR)) {
                        sp_.delete_lf();
                        length = sp_.length();
                        if ((strncmp("#id ", (const char*)sp_.get_ptr(0), 4) ==
                             0) &&
                            (sp_.get_char(length - 1) == CR) &&
                            (length == 13)) {
                            strncpy((char*)ptr->data.id,
                                    (const char*)sp_.get_ptr(4), 8);
                            ptr->data.id[ID_LENGTH] = '\0';  // 終端しておく
                            tud_cdc_write("OK\r\n", 4);
                            tud_cdc_write_flush();
                            fa.write_last_sector((uint8_t*)ptr);  // 書く
                            fa.read_last_sector((uint8_t*)ptr);   // 読む
                            printf(
                                "id=%s, password=%s, int_val=%d, "
                                "double_val=%f\r\n",
                                ptr->data.id, ptr->data.password,
                                ptr->data.int_val, ptr->data.double_val);
                        } else {
                            tud_cdc_write("NG\r\n", 4);
                            tud_cdc_write_flush();
                        }
                        clear_buffer();  // 連続したコマンドは受け付けない仕様
                    } else {
                        return;
                    }
                    break;
                case StringParser::Double:
                    double value;
                    if (4 <= sp_.length() && sp_.search_etx(CR)) {
                        sp_.delete_lf();
                        length = sp_.length();
                        if ((strncmp("#double ", (const char*)sp_.get_ptr(0),
                                     8) == 0) &&
                            (sp_.get_char(length - 1) == CR) &&
                            (10 <= length)) {
                            sp_.set_char(length - 1, '\0');
                            value =
                                strtod((const char*)sp_.get_ptr(8), &endptr);
                            if (endptr == (const char*)sp_.get_ptr(
                                              8)) {  // 変換が行われなかった場合
                                clear_buffer();
                                tud_cdc_write("NG\r\n", 4);
                                tud_cdc_write_flush();
                                continue;
                            } else if (
                                // 文字の途中で変換が終了した場合
                                *endptr != '\0') {
                                clear_buffer();  // これも一応捨てることにする
                                tud_cdc_write("NG\r\n", 4);
                                tud_cdc_write_flush();
                                continue;
                            }
                            ptr->data.double_val = value;
                            tud_cdc_write("OK\r\n", 4);
                            tud_cdc_write_flush();
                            fa.write_last_sector((uint8_t*)ptr);  // 書く
                            fa.read_last_sector((uint8_t*)ptr);   // 読む
                            printf(
                                "id=%s, password=%s, int_val=%d, "
                                "double_val=%f\r\n",
                                ptr->data.id, ptr->data.password,
                                ptr->data.int_val, ptr->data.double_val);
                        } else {
                            tud_cdc_write("NG\r\n", 4);
                            tud_cdc_write_flush();
                        }
                        clear_buffer();  // 連続したコマンドは受け付けない仕様
                    } else {
                        return;
                    }
                    break;
                case StringParser::Password:
                    if (4 <= sp_.length() && sp_.search_etx(CR)) {
                        sp_.delete_lf();
                        length = sp_.length();
                        if ((strncmp("#password ", (const char*)sp_.get_ptr(0),
                                     10) == 0) &&
                            (sp_.get_char(length - 1) == CR) &&
                            (length == 19)) {
                            strncpy((char*)ptr->data.password,
                                    (const char*)sp_.get_ptr(10), 8);
                            ptr->data.password[PASSWORD_LENGTH] =
                                '\0';  // 終端しておく
                            tud_cdc_write("OK\r\n", 4);
                            tud_cdc_write_flush();
                            fa.write_last_sector((uint8_t*)ptr);  // 書く
                            fa.read_last_sector((uint8_t*)ptr);   // 読む
                            printf(
                                "id=%s, password=%s, int_val=%d, "
                                "double_val=%f\r\n",
                                ptr->data.id, ptr->data.password,
                                ptr->data.int_val, ptr->data.double_val);
                        } else {
                            tud_cdc_write("NG\r\n", 4);
                            tud_cdc_write_flush();
                        }
                        clear_buffer();  // 連続したコマンドは受け付けない仕様
                    } else {
                        return;
                    }
                    break;
                case StringParser::Int:
                    long value2;
                    errno = 0;
                    if (4 <= length && sp_.search_etx(CR)) {
                        sp_.delete_lf();
                        length = sp_.length();
                        if ((strncmp("#int ", (const char*)sp_.get_ptr(0), 5) ==
                             0) &&
                            (sp_.get_char(length - 1) == CR) && (7 <= length)) {
                            sp_.set_char(length - 1, '\0');
                            value2 = strtol((const char*)sp_.get_ptr(5),
                                            &endptr, 10);
                            if (errno == ERANGE) {
                                clear_buffer();
                                tud_cdc_write("NG\r\n", 4);
                                tud_cdc_write_flush();
                                continue;
                            } else if ((endptr ==
                                        (const char*)sp_.get_ptr(5)) ||
                                       (*endptr !=
                                        '\0')) {  // 変換が行われなかった場合
                                clear_buffer();
                                tud_cdc_write("NG\r\n", 4);
                                tud_cdc_write_flush();
                                continue;
                            }
                            ptr->data.int_val = (int)value2;
                            tud_cdc_write("OK\r\n", 4);
                            tud_cdc_write_flush();
                            fa.write_last_sector((uint8_t*)ptr);  // 書く
                            fa.read_last_sector((uint8_t*)ptr);   // 読む
                            printf(
                                "id=%s, password=%s, int_val=%d, "
                                "double_val=%f\r\n",
                                ptr->data.id, ptr->data.password,
                                ptr->data.int_val, ptr->data.double_val);
                        } else {
                            tud_cdc_write("NG\r\n", 4);
                            tud_cdc_write_flush();
                        }
                        clear_buffer();  // 連続したコマンドは受け付けない仕様
                    } else {
                        return;
                    }
                    break;
                case StringParser::NoEntry:
                default:
                    cleanup_if_needed();
                    break;
            }
        } else {
            cleanup_if_needed();
        }
    }
}

void UsbCDC::clear_buffer() {
    sp_.clear();
    st_.clear();
}

void UsbCDC::cleanup_if_needed() {
    if (sp_.has_data() && sp_.search_etx(CR)) {
        sp_.clear();
        st_.clear();
        tud_cdc_write("NG\r\n", 4);
        tud_cdc_write_flush();
    } else if (20 <= sp_.length()) {
        sp_.clear();
        st_.clear();
        tud_cdc_write("NG\r\n", 4);
        tud_cdc_write_flush();
    }
}
