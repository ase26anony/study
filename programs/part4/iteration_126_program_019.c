#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* This will trigger case '[': */
    int fixed_array[10];
    char variable_array[];
    /* Multi-dimensional array */
    double matrix[5][5];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* This will trigger case '{': with anonymous union */
    union {
        int int_val;
        float float_val;
        char* ptr_val;
    } data;
    /* Another nested struct */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    void (*handler)(void);
    
    /* Brackets in array declaration */
    struct GTY(()) Node* children[100];
    
    /* Braces for nested struct definition */
    struct {
        /* Nested parentheses in bitfield */
        unsigned int flags: (8);
        /* Nested brackets */
        char name[50];
    } metadata;
    
    /* Function pointer returning pointer to array */
    int (*(*complex_func)(int))[10];
    
    /* Flexible array member with nested struct */
    struct GTY(()) {
        int id;
        char* name;
    } items[];
};

/* Test case 5: Typedef with GTY annotation */
typedef struct GTY(()) {
    /* Mix of all brackets */
    int (*methods[5])(int, char*);
    union {
        int i;
        float f;
    } value;
} AnonStruct;

/* Test case 6: Pointer to array of function pointers */
struct GTY(()) Container {
    /* Complex type: pointer to array of function pointers */
    int (*(*func_table)[10])(int, char*);
    
    /* Nested struct with all bracket types */
    struct GTY(()) {
        /* Parentheses in cast-like context in sizeof */
        char buffer[(sizeof(int) + 3) & ~3];
        /* Function pointer array */
        void (*callbacks[5])(void);
    } nested;
};

#endif /* TEST_GTY_H */
