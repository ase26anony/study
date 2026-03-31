#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This triggers case '(': consume_balanced('(', ')') */
    int (*callback)(int, char*);
    unsigned int bits: (sizeof(int)*8);  /* More parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* These trigger case '[': consume_balanced('[', ']') */
    int fixed_array[10];
    char variable_array[];
    int (*array_of_pointers[5])(void);
    int multi_dim[3][4];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* This triggers case '{': consume_balanced('{', '}') */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets in array declaration */
    struct StructWithParens* items[20];
    
    /* Braces in nested struct definition */
    struct {
        int count;
        /* Nested parentheses in bitfield */
        unsigned int: (8 - (sizeof(int) % 8));
        /* Nested brackets */
        char name[50];
    } header;
    
    /* Flexible array member with brackets */
    union UnionWithBrackets flexible_array[];
};

/* Test case 5: Typedef with function pointer returning array pointer */
typedef int (*GTY(()) ComplexFuncPtr)(int, char**)[10];

/* Test case 6: Struct with all bracket types in one member */
struct GTY(()) AllInOne {
    /* Function pointer (parentheses) returning pointer to array (brackets) */
    int (*(*complex_member)(int))[5];
    
    /* Anonymous struct (braces) containing array (brackets) */
    struct {
        int (*callbacks[3])(void);  /* Array of function pointers */
    } nested;
};

/* Test case 7: Union with nested parentheses in bitfields */
union GTY(()) BitfieldUnion {
    unsigned int full;
    struct {
        /* Parentheses in bitfield width specification */
        unsigned int low: (16);
        unsigned int high: (sizeof(unsigned int)*8 - 16);
    } parts;
};

#endif /* TEST_GTY_H */
