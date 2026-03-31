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
    int fixed_array[10];
    /* Flexible array member */
    char flexible_array[];
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* Anonymous union with braces - triggers '{' case */
    union {
        int as_int;
        float as_float;
        char as_char;
    } data;
    /* Another nested struct */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_callback)[5])(int, char*);
    
    /* Struct with bitfield using parentheses */
    struct {
        unsigned int flags: (8 * sizeof(unsigned int) - 4);
        /* Array within nested struct */
        int values[4];
    } nested;
    
    /* Union with all bracket types */
    union {
        /* Function pointer array */
        void (*func_array[3])(void);
        /* Struct with flexible array */
        struct {
            size_t len;
            char data[];
        } dynamic;
    } variant;
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*FuncPtr)(void);
struct GTY(()) Container {
    /* Array of function pointers - triggers both '[' and '(' */
    FuncPtr functions[10];
    
    /* Pointer to array with parentheses for size calculation */
    int* dynamic_array GTY((length("((size + 7) & ~7)")));
};

#endif /* TEST_GTY_H */
