#include "StringParser.h"

#include <string.h>

StringParser::StringParser(int ptr) {
    ptr_ = ptr;
    has_stx_ = false;
}

void StringParser::add(uint8_t* buf, int count) {
    for (int i = 0; i < count; i++) {
        buffer_[ptr_ + i] = buf[i];
    }
    ptr_ += count;
}

void StringParser::append(char c) { buffer_[ptr_++] = (uint8_t)c; }

// 文字cを探す
// 見つかれば has_stx_ を true にする
// もし buffer_[] の途中にcが見つかったら先頭に移動する

void StringParser::search_stx(uint8_t c) {
    int count = 0;
    if (has_stx_) {
        return;
    } else {
        if (buffer_[0] == c) {
            has_stx_ = true;
            return;
        } else {
            for (int i = 1; i < ptr_; i++) {
                if (buffer_[i] == c) {
                    for (int j = 0; j < ptr_ - 1; j++) {
                        buffer_[j] = buffer_[i + j];
                        count++;
                    }
                    ptr_ = count;
                    has_stx_ = true;
                    for (int k = 0; k < 20; k++) {  // 20はいいかげんに決めた数
                        if (CR == buffer_[k]) {
                            ptr_ = k + 1;
                        }
                    }
                    return;
                }
            }
        }
    }
}

// ポインターをインクリメントせずバッファに文字をセットする

void StringParser::set_char(int n, char c) { buffer_[n] = c; }

// コマンドの最初の部分でコマンドを区別する
// コマンド文字列全体を確認しているわけではない

StringParser::Command StringParser::get_command_first() {
    if (buffer_[1] == 'p') {
        return Password;
    } else if (buffer_[1] == 'd') {
        return Double;
    } else if (buffer_[1] == 'i') {
        if (buffer_[2] == 'd') {
            return Id;
        } else if (buffer_[2] == 'n') {
            return Int;
        } else {
            return NoEntry;
        }
    }
    return NoEntry;
}

bool StringParser::search_etx(uint8_t c) {
    for (int i = 0; i < ptr_;
         i++) {  // 文字無しでもひっかかるように 0 から検索することにした
        if (buffer_[i] == CR) {
            has_etx_ = true;
            return true;
        }
    }
    has_etx_ = false;
    return false;
}

bool StringParser::has_stx() { return has_stx_; }

bool StringParser::has_etx() { return has_etx_; }

bool StringParser::has_data() {
    if (ptr_ != 0) {
        return true;
    } else {
        return false;
    }
}

void StringParser::dec_ptr() {
    if (ptr_) {
        ptr_--;
        buffer_[ptr_] = '\0';
        if (!ptr_) {
            has_stx_ = false;
        }
    }
}

int StringParser::length() { return ptr_; }

// CR LF が来ていたら LF を削除する

void StringParser::delete_lf() {
    if (buffer_[ptr_ - 2] == CR) {
        if (buffer_[ptr_ - 1] == LF) {
            buffer_[ptr_ - 1] = '\0';
            ptr_--;
        }
    }
}

uint8_t StringParser::get_char_with_dec_ptr() {
    uint8_t c = 0;
    if (ptr_) {
        c = buffer_[--ptr_];
        buffer_[ptr_] = '\0';
        if (!ptr_) {
            has_stx_ = false;
        }
    }
    return c;
}

uint8_t* StringParser::get_ptr(int n) { return &buffer_[n]; }

uint8_t StringParser::get_char(int n) { return buffer_[n]; }

void StringParser::clear() {
    for (int i = 0; i < ptr_; i++) {
        buffer_[i] = 0;
    }
    ptr_ = 0;
    has_stx_ = false;
    has_etx_ = false;
}
