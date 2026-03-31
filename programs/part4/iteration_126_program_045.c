#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int bits: (sizeof(int)*8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* This will trigger case '[': */
    int fixed_array[10];
    char variable_array[];
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* This will trigger case '{': */
    union {
        int int_val;
        float float_val;
        char* ptr_val;
    } data;
    /* Another nested struct */
    struct {
        short x;
        short y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer */
    void (*init)(struct ComplexType*);
    
    /* Brackets in array declaration */
    struct StructWithParens* items[5];
    
    /* Braces for nested anonymous struct */
    struct {
        int counter;
        /* Array within nested struct */
        char buffer[256];
    } state;
    
    /* Function pointer returning pointer to array */
    int (*(*complex_func)(void))[10];
    
    /* Flexible array member */
    unsigned char extra_data[];
};

/* Test case 5: Typedef with GTY annotation */
typedef struct GTY(()) {
    int id;
    char name[50];
    /* Pointer to function taking array and returning struct */
    struct ComplexType* (*processor)(int[], size_t);
} NamedType;

/* Test case 6: Union with all bracket types */
union GTY(()) UnionAll {
    /* Parentheses in bitfield with expression */
    struct {
        unsigned int a: (8);
        unsigned int b: (16);
    } bits;
    
    /* Array of function pointers */
    int (*func_array[5])(void);
    
    /* Nested anonymous union */
    union {
        long long big;
        /* Array in nested union */
        int parts[2];
    } split;
};

#endif /* TEST_GTY_H */
