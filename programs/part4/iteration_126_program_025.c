#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types */
#include <stddef.h>

/* Struct with parentheses - triggers case '(' */
struct GTY(()) StructWithParens {
    /* Function pointer with parentheses */
    int (*callback)(int, char);
    
    /* Bitfield with parenthesized expression */
    unsigned int bits: (sizeof(int) * 8 - 1);
    
    /* Pointer to function returning pointer */
    char *(*allocator)(size_t);
};

/* Union with brackets - triggers case '[' */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Multi-dimensional array */
    double matrix[3][3];
    
    /* Array with computed size */
    char variable_array[sizeof(int) * 2];
    
    /* Flexible array member */
    long flexible_array[];
};

/* Struct with braces - triggers case '{' */
struct GTY(()) StructWithBraces {
    int id;
    
    /* Anonymous nested union with braces */
    union {
        int as_int;
        float as_float;
        void *as_ptr;
    } GTY((tag("0"))) value;
    
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_callback)(void))[10];
    
    /* Array of function pointers - has '[' and '(' */
    void (*handlers[5])(struct ComplexType *);
    
    /* Nested struct with bitfield - has '{' and '(' */
    struct {
        unsigned int flags: (8);
        char name[20];
    } GTY((skip)) metadata;
    
    /* Pointer to array of structs with flexible array */
    struct GTY(()) InnerStruct {
        int count;
        union {
            short s;
            long l;
        } data;
        char payload[];
    } **inner_array;
};

/* Another type with deeply nested brackets */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *GTY((skip)) right;
    
    /* Array of function pointers with complex signature */
    int (*(*operations[3])(struct TreeNode *, int))[];
    
    /* Anonymous struct with bitfield */
    struct {
        unsigned int : 4;
        unsigned int visited: 1;
        unsigned int depth: (sizeof(int) * 8 - 5);
    } GTY((skip)) state;
} TreeNode;

#endif /* TEST_GTY_H */
