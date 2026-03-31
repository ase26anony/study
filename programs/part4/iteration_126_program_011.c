#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case for parentheses - function pointer */
struct GTY(()) StructWithParens {
    int (*callback)(int, char*);  /* Triggers case '(' */
    unsigned int bits: (sizeof(int)*8);  /* More parentheses */
    void (*array_of_funcs[5])(void);  /* Combined brackets and parentheses */
};

/* Test case for brackets - arrays */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];  /* Triggers case '[' */
    char variable_array[];  /* Flexible array member */
    struct GTY(()) {
        int nested_array[3][4];  /* Multi-dimensional array */
    } inner;
};

/* Test case for braces - nested anonymous struct/union */
struct GTY(()) StructWithBraces {
    int id;
    union {  /* Triggers case '{' */
        int as_int;
        float as_float;
        struct {
            char byte_array[8];
        } as_struct;
    } data;
    struct GTY(()) {
        int x;
        int y;
    } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has both '(' and '[' */
    int (*(*complex_func)(int))[10];
    
    /* Array of function pointers */
    void (*func_array[5])(void);
    
    /* Nested struct with bitfield using parentheses */
    struct GTY(()) {
        unsigned int flags: (sizeof(unsigned int)*8 - 1);
        int matrix[2][2];
    } nested;
    
    /* Union with anonymous struct */
    union {
        int a;
        struct {
            char str[20];
        } b;
    } choice;
};

/* Pointer chain with GTY marker */
typedef struct GTY(()) BaseType {
    int value;
    struct BaseType* GTY((skip)) next;  /* GTY annotation with parentheses */
} BaseType;

#endif /* TEST_GTY_H */
