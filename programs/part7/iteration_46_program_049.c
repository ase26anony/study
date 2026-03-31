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
};

/* Union definition - another '{' case */
union SimpleUnion {
    int int_val;
    float float_val;
};

/* Enum - uses default case for identifiers */
enum SimpleEnum {
    VALUE1,
    VALUE2,
    VALUE3
};

#endif /* SIMPLE_H */
