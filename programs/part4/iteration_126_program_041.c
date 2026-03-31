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
    double matrix[5][5];
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
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_callback))(int)[10];
    
    /* Array of function pointers */
    void (*func_array[5])(void);
    
    /* Nested struct with all bracket types */
    struct GTY(()) {
        /* Bitfield with parentheses */
        unsigned flag: (8);
        /* Array member */
        int values[4];
        /* Function pointer member */
        void (*helper)(void);
    } nested;
    
    /* Union with anonymous struct */
    union {
        struct {
            int a;
            int b;
        };
        long long combined;
    } GTY((tag("1"))) optional;
};

/* Test case 5: Pointer to array of function pointers */
typedef int (*GTY(()) FuncPtr)(void);
struct GTY(()) Container {
    /* Array of pointers with parentheses in type */
    FuncPtr* GTY((length("count"))) functions;
    int count;
    
    /* Pointer to array */
    int (*matrix_ptr)[3][3];
};

#endif /* TEST_GTY_H */
