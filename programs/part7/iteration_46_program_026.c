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

/* Enum - uses '{' but also lots of default case characters */
enum SimpleEnum {
    VALUE_ZERO,
    VALUE_ONE,
    VALUE_TWO = 2
};

/* More default case triggers */
static const volatile unsigned long counter = 0UL;
typedef struct SimpleStruct SimpleType;

#endif /* SIMPLE_H */
