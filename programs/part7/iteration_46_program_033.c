/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Function prototype - triggers '(' case */
extern void simple_function(int parameter);

/* Array declaration - triggers '[' case */
extern int simple_array[10];

/* Structure definition - triggers '{' case */
struct SimpleStruct {
    int member1;
    char member2;
    double member3;
};

/* Union definition - another '{' case */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
};

/* More default case material */
typedef unsigned long ulong_t;
static const volatile int global_counter = 42;

#endif /* SIMPLE_H */
