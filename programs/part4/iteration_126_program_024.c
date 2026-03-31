#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include for size_t */
#include <stddef.h>

/* Struct triggering case '(': function pointer with parentheses */
struct GTY(()) StructWithParens {
    int (*callback)(int, char);          /* Function pointer */
    unsigned int bits: (sizeof(int)*8);  /* Bitfield with parentheses */
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
    } GTY((skip)) container;
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (sizeof(unsigned int)*8 - 1);
};

#endif /* TEST_GTY_H */
