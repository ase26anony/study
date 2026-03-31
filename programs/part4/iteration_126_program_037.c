#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types */
#include <stddef.h>

/* Struct with parentheses - function pointer type */
struct GTY(()) StructWithParens {
    int value;
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    /* Bitfield with parenthesized expression */
    unsigned int bits: (sizeof(int) * 8 - 1);
    /* Pointer to function returning pointer */
    void* (*allocator)(size_t);
};

/* Union with brackets - array declarations */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    /* Multi-dimensional array */
    double matrix[3][3];
    /* Flexible array member (C99) */
    char flexible_array[];
    /* Pointer to array */
    int (*array_ptr)[5];
};

/* Struct with braces - nested anonymous union */
struct GTY(()) StructWithBraces {
    int tag;
    /* Anonymous union with braces */
    union {
        int int_value;
        float float_value;
        /* Nested struct within union */
        struct {
            char c;
            short s;
        } nested_struct;
    } data;
    /* Struct with bitfields using braces */
    struct {
        unsigned int a: 4;
        unsigned int b: 4;
        unsigned int c: 8;
    } flags;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - uses () and [] */
    int* (*get_array)(void)[10];
    
    /* Array of function pointers - uses [] and () */
    void (*handlers[5])(int);
    
    /* Nested struct with all bracket types */
    struct {
        /* Parentheses in function pointer */
        void (*cleanup)(struct ComplexType*);
        /* Brackets in array */
        char name[32];
        /* Braces for anonymous union */
        union {
            long l;
            double d;
        } value;
    } metadata;
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        void (*action)(void);
    } actions[];
};

/* Another type with deeply nested brackets */
typedef struct GTY(()) TreeNode {
    int key;
    /* Pointer to function with array parameter */
    void (*visit)(int path[], int depth);
    /* Self-referential pointers */
    struct TreeNode* GTY((skip)) left;
    struct TreeNode* GTY((skip)) right;
    /* Array of pointers to functions returning pointers */
    struct TreeNode* (*finders[3])(struct TreeNode*, int);
} TreeNode;

#endif /* TEST_GTY_H */
