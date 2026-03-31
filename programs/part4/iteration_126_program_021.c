#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case for parentheses: function pointer type */
struct GTY(()) StructWithParens {
    int (*callback)(int, char*);  /* Parentheses for function pointer */
    unsigned int bits: (sizeof(int) * 8);  /* Parentheses in bitfield */
    void (*complex_func)(int (*)(double), char);  /* Nested parentheses */
};

/* Test case for brackets: array declarations */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];  /* Fixed size array */
    char* ptr_array[5];   /* Array of pointers */
    double matrix[3][4];  /* Multi-dimensional array */
    int flexible_array[]; /* Flexible array member (requires struct to be last) */
};

/* Test case for braces: nested struct/union */
struct GTY(()) StructWithBraces {
    int id;
    union {  /* Anonymous union with braces */
        int as_int;
        float as_float;
        struct {  /* Nested anonymous struct */
            char c;
            short s;
        } nested;
    } data;
    struct {  /* Another anonymous struct */
        long x;
        long y;
    } point;
};

/* Comprehensive test case combining all bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer returning pointer to array */
    int (*(*complex_callback)[5])(void);
    
    /* Array of function pointers */
    void (*func_array[3])(int);
    
    /* Nested struct with all bracket types */
    struct {
        int (*nested_func)(int[10]);  /* Function taking array */
        union {
            char* str;
            int values[4];
        } choice;
    } nested;
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (8 - 1);
    
    /* Multi-dimensional array with computed size */
    char buffer[(sizeof(void*) == 8) ? 64 : 32];
};

/* Additional test: pointer to array */
typedef int (*ArrayPtr)[10];
struct GTY(()) StructWithArrayPtr {
    ArrayPtr ptr;  /* Uses typedef with brackets */
    int (*direct_ptr)[5];  /* Direct pointer to array */
};

/* Test case for union with all bracket types */
union GTY(()) UnionAll {
    struct {
        int (*func)(int[]);  /* Function taking flexible array */
        struct {
            char data[100];
        } buffer;
    } s;
    int (*array_of_funcs[2])(void);  /* Array of function pointers */
    void* generic;
};

#endif /* TEST_GTY_H */
