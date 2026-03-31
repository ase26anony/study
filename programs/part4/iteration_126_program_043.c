#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    /* Bitfield with parentheses in expression */
    unsigned int bits: (sizeof(int)*8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    /* Flexible array member */
    char flexible_array[];
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* Nested anonymous union with braces */
    union {
        int int_val;
        float float_val;
        char* string_val;
    } data;
    /* Another nested struct */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array (parentheses and brackets) */
    int (*(*complex_callback)[5])(void);
    
    /* Struct with bitfield and nested union */
    struct {
        unsigned int flags: (8);
        union {
            int i;
            void* p;
        } variant;
    } GTY((skip)) inner;  /* GTY marker with parentheses */
    
    /* Array of function pointers */
    void (*func_array[10])(struct ComplexType*);
    
    /* Flexible array of structs with bitfields */
    struct {
        int id: (16);
        int value;
    } items[];
};

/* Test case 5: Pointer to array of function pointers */
typedef void (*GTY((ptr)) FuncPtr)(void);
struct GTY(()) Container {
    /* Array of pointers with GTY tag */
    FuncPtr* GTY((length("count"))) functions;
    int count;
    
    /* Nested struct with all bracket types */
    struct GTY(()) Nested {
        int (*compare)(const void*, const void*);
        char buffer[256];
        union {
            struct {
                int x, y;
            } pos;
            float coords[2];
        } location;
    } *nested;
};

#endif /* TEST_GTY_H */
