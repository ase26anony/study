#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Struct triggering case '(': function pointer with parentheses */
struct GTY(()) StructWithParens {
    int (*callback)(int, char);          /* Nested parentheses in type */
    unsigned int bits: (sizeof(int)*8);  /* Parentheses in bitfield expression */
};

/* Union triggering case '[': various array declarations */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];                 /* Fixed-size array */
    char variable_array[];               /* Flexible array member */
    double *ptr_array[5];                /* Array of pointers */
};

/* Struct triggering case '{': nested anonymous union */
struct GTY(()) StructWithBraces {
    int tag;
    union {
        int as_int;
        float as_float;
    } GTY((tag("tag"))) value;           /* Nested union with braces */
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer (parentheses) returning pointer to array (brackets) */
    int (*(*complex_callback)(void))[4];
    
    /* Nested struct (braces) containing an array (brackets) */
    struct {
        int nested_array[2][3];
    } GTY(()) container;
    
    /* Array of function pointers (mixed brackets/parentheses) */
    void (*func_array[5])(int);
};

#endif /* TEST_GTY_H */
