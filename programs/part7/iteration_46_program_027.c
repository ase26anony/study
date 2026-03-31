/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Function prototype - triggers '(' case */
extern void simple_function(int param);

/* Array declaration - triggers '[' case */
extern int numbers[10];

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

/* Enum - uses default case extensively */
enum Color {
    RED,    /* default case for identifiers and commas */
    GREEN,
    BLUE
};

/* Typedef - default case for keywords */
typedef unsigned long ulong;

/* Pointer declaration - default case for '*' */
const volatile char *special_ptr;

#endif /* SIMPLE_H */
