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
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': - anonymous nested union */
    union {
        int as_int;
        float as_float;
    } GTY((tag("0"))) data;
    
    /* Nested struct with braces */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Test case 4: Complex type combining all bracket types */
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
    } GTY((skip)) state;
    
    /* Function pointer returning pointer to array */
    int (*(*complex_func)(void))[10];
    
    /* Flexible array member at the end */
    unsigned char extra_data[];
};

/* Test case 5: Typedef with function pointer (more parentheses) */
typedef int (*comparator_fn)(const void*, const void*);

struct GTY(()) Container {
    comparator_fn cmp;
    void** GTY((length("%h.size"))) elements;
    size_t size;
};

/* Test case 6: Union with bitfield containing parenthesized expression */
union GTY(()) BitfieldUnion {
    unsigned long full;
    struct {
        /* Parentheses in bitfield width */
        unsigned int low: (sizeof(int) * 4);
        unsigned int high: (sizeof(int) * 4);
    } parts;
};

#endif /* TEST_GTY_H */
