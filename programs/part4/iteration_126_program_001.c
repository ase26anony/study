#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': consume_balanced('(', ')') */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* This will trigger case '[': consume_balanced('[', ']') */
    int fixed_array[10];
    char variable_array[];
    /* Multi-dimensional array */
    double matrix[5][5];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* This will trigger case '{': consume_balanced('{', '}') */
    union {
        int int_val;
        float float_val;
        struct {
            char c;
            short s;
        } nested;
    } data;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets in array declaration */
    struct StructWithParens* items[100];
    
    /* Braces for nested struct definition */
    struct {
        int count;
        /* Nested parentheses in function pointer array */
        int (*handlers[5])(void);
        /* Nested brackets in flexible array member */
        char name[];
    } GTY((skip)) metadata;
    
    /* More complex: function pointer returning pointer to array */
    int (*(*complex_callback)(int))[10];
};

/* Test case 5: Typedef with GTY annotation and parentheses */
typedef struct GTY(()) {
    int x;
    int y;
    /* Pointer to function with parameters in parentheses */
    void (*draw)(int, int);
} Point;

/* Test case 6: Union with all bracket types mixed */
union GTY(()) MixedUnion {
    /* Array of function pointers */
    int (*func_array[5])(char*);
    
    struct {
        /* Bitfield with parentheses */
        unsigned int flags: (8);
        /* Flexible array member */
        int data[];
    } header;
    
    /* Anonymous struct with initializer-like syntax (in comments to show variety) */
    /* Note: gengtype might see these tokens during parsing */
    struct {
        int a;
        int b;
    };
};

#endif /* TEST_GTY_H */
