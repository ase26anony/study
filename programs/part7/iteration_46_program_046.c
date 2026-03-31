/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Function prototype - triggers '(' case */
extern void simple_function(int param);

/* Array declaration - triggers '[' case */
extern int simple_array[10];

/* Structure definition - triggers '{' case */
struct SimpleStruct {
    int member1;
    char member2;
    double member3;
};

/* Union definition - also triggers '{' case */
union SimpleUnion {
    int int_val;
    float float_val;
    char char_val;
};

/* Enum with default case keywords */
enum SimpleEnum {
    VALUE1,  /* comma triggers default case */
    VALUE2,
    VALUE3
};

/* Typedef with pointer - asterisk triggers default case */
typedef struct SimpleStruct* SimplePtr;

/* Const and volatile qualifiers - keywords trigger default case */
const volatile int special_var = 42;

#endif /* SIMPLE_H */
