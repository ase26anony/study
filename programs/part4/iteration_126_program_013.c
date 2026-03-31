#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger the '(' case */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* Static array - triggers '[' case */
    int static_array[10];
    /* Flexible array member */
    char flexible_array[];
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* Nested anonymous union with braces */
    union {
        int as_int;
        float as_float;
        char as_char;
    } GTY((tag("0"))) data;
    
    /* Another nested struct */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_func)(int))[10];
    
    /* Struct with bitfield using parentheses */
    struct {
        unsigned int flags: (8 * sizeof(unsigned int) - 4);
        /* Array member */
        int values[5];
    } GTY((skip)) nested;
    
    /* Union with anonymous struct */
    union {
        struct {
            int a;
            int b;
        };
        long long combined;
    } GTY((desc("%1.combined"))) variant;
};

/* Test case 5: Pointer to function with complex parameters */
typedef int (*GTY((skip)) ComplexCallback)(
    struct StructWithParens*,  /* Parameter with struct type */
    union UnionWithBrackets[5], /* Parameter with array type */
    void (*)(int)              /* Nested function pointer */
);

/* Test case 6: Template-like structure with all brackets */
struct GTY(()) AllBrackets {
    /* Array of function pointers */
    void (*func_array[5])(void);
    
    /* Nested struct with bitfield */
    struct GTY(()) InnerStruct {
        int x: (sizeof(int)*4);
        int y: (sizeof(int)*4);
        int array[(sizeof(int) * 2)];
    } *inner;
    
    /* Anonymous union */
    union {
        char str[100];
        int nums[25];
    };
};

#endif /* TEST_GTY_H */
