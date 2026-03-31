/* simple.h - Basic delimiter cases for gengtype parser coverage */

#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case - function prototype */
void simple_function(int param);

/* Trigger '[' case - array declaration */
extern int simple_array[10];

/* Trigger '{' case - structure definition */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Trigger default case - identifiers and keywords */
static const volatile unsigned long counter = 42;

#endif /* SIMPLE_H */
