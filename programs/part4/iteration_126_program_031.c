#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Struct triggering case '(': function pointer with parentheses */
struct GTY(()) StructWithParens {
    int (*callback)(int, char);          /* Nested parentheses */
    unsigned int bits: (sizeof(int)*8);  /* Parentheses in bitfield */
};

/* Union triggering case '[': various array declarations */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];                 /* Fixed-size brackets */
    char variable_array[];               /* Flexible array brackets */
    struct {
        double matrix[3][3];             /* Multi-dimensional brackets */
    } nested;
};

/* Struct triggering case '{': anonymous nested union with braces */
struct GTY(()) StructWithBraces {
    int tag;
    union {                              /* Braces for nested union */
        int as_int;
        float as_float;
    } value;
    struct {                             /* Braces for nested struct */
        short x, y;
    } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses for function pointer returning pointer to array */
    int (*(*complex_callback)(void))[5];
    
    /* Brackets for array of function pointers (with parentheses) */
    void (*func_array[5])(int);
    
    /* Braces for embedded anonymous struct */
    struct {
        int id;
        char name[20];                   /* Brackets inside braces */
    } metadata;
    
    /* Parentheses in sizeof expression within array */
    size_t sizes[(sizeof(int) + 7)/8];
};

#endif /* TEST_GTY_H */
