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
    int static_array[10];                /* Static array */
    char variable_array[];               /* Flexible array member */
    double *ptr_array[5];                /* Array of pointers */
};

/* Struct triggering case '{': nested anonymous union */
struct GTY(()) StructWithBraces {
    int id;
    union {                              /* Braces for nested union */
        float f;
        long l;
    } data;
    struct {                             /* Braces for nested struct */
        short x, y;
    } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer (parentheses) returning pointer to array (brackets) */
    int (*(*complex_callback)(void))[4];
    
    /* Nested struct (braces) containing array (brackets) */
    struct {
        int values[10];
    } container;
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (sizeof(unsigned) - 1);
};

#endif /* TEST_GTY_H */
