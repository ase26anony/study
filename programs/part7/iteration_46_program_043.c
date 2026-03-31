/* simple.h - Basic delimiter cases for coverage testing */

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

/* Enum - uses default case for identifiers */
enum SimpleEnum {
    VALUE1,
    VALUE2 = 10,
    VALUE3
};

/* Typedef - default case for identifiers */
typedef unsigned long ulong_t;

/* Multiple declarations with various delimiters */
void func1(void);
int func2(char *str, int len);
float func3(double d, int i, char c);

int matrix[5][10];
struct Point { int x; int y; };

#endif /* SIMPLE_H */
