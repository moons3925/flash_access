#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>  // uint8_t

class Stack {
   public:
    Stack(int size);
    ~Stack();
    void push(uint8_t c);
    uint8_t pop();
    bool is_empty() { return index <= 0; };
    void clear();

   private:
    int size;
    int index;
    uint8_t* stack;
};

#endif  // STACK_H_
