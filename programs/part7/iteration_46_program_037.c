/* simple.h - Basic delimiter cases to trigger all switch branches */

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
    float member3;
};

/* Union definition - also triggers '{' case */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
};

/* Enum with default case characters */
enum SimpleEnum {
    VALUE1,    /* comma triggers default */
    VALUE2 = 5,
    VALUE3
};

/* More default case triggers */
static const volatile unsigned long counter = 100UL;
typedef struct SimpleStruct SimpleType;

#endif /* SIMPLE_H */
