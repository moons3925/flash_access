#include "Stack.h"

#include <stdlib.h>

#include "pico/stdlib.h"  // assert()

Stack::Stack(int size) {
    this->size = size;
    stack = new uint8_t[size];
    clear();
}

Stack::~Stack() { delete[] stack; }

void Stack::clear() { index = 0; }

void Stack::push(uint8_t c) {
    assert(index < size);
    stack[index++] = c;
}

uint8_t Stack::pop() {
    assert(index > 0);
    return stack[--index];
}
