#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Struct triggering case '(': function pointer with parentheses */
struct GTY(()) StructWithParens {
    int (*callback)(int);                     /* Parentheses in function pointer */
    unsigned int bits: (sizeof(int)*8);       /* Parentheses in bitfield expression */
};

/* Union triggering case '[': various array declarations */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];                      /* Fixed-size array brackets */
    char variable_array[];                    /* Flexible array member brackets */
    void* ptr_array[(sizeof(void*) == 8) ? 2 : 1]; /* Conditional size brackets */
};

/* Struct triggering case '{': nested anonymous union */
struct GTY(()) StructWithBraces {
    int id;
    union {                                   /* Braces for nested anonymous union */
        float f;
        int i;
    } data;
    struct {                                  /* Braces for nested anonymous struct */
        short x;
        short y;
    } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer (parentheses) returning pointer to array (brackets) */
    int (*(*complex_callback)(void))[4];
    
    /* Nested struct (braces) containing array (brackets) */
    struct {
        int values[5];                        /* Brackets inside braces */
    } container;
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (sizeof(unsigned int) - 1);
};

#endif /* TEST_GTY_H */
