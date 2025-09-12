// StringParser.h

#ifndef STRINGPARSER_H_
#define STRINGPARSET_H_

#include <stdint.h>  // uint8_t

#include "PaddedData.h"

#define CR 0x0d  // テキストの終わり
#define LF 0x0a
#define BS 0x08
#define ARROW_LEFT 37

class StringParser {
   public:
    enum Command { Id, Password, Int, Double, NoEntry };

    StringParser(int ptr);
    void add(uint8_t* buf, int count);
    void search_stx(uint8_t c);
    bool search_etx(uint8_t c);
    bool has_stx();
    bool has_etx();
    bool has_data();
    void dec_ptr();
    int length();
    Command get_command_first();
    uint8_t* get_ptr(int n);
    uint8_t get_char(int n);
    uint8_t get_char_with_dec_ptr();
    void set_char(int n, char c);
    void append(char c);
    void clear();
    void delete_lf();

   private:
    uint8_t buffer_[1024];
    int ptr_;
    bool has_stx_;
    bool has_etx_;
};

#endif  // STRINGPARSER_H_
