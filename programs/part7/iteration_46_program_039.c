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

/* Enum with default case identifiers */
enum SimpleEnum {
    VALUE_ZERO,    /* default case: identifiers */
    VALUE_ONE,     /* default case: identifiers */
    VALUE_TWO = 2  /* default case: '=', numbers */
};

/* Multiple declarations with various punctuation */
static const volatile int counter = 0;  /* keywords trigger default case */

#endif /* SIMPLE_H */
