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
    double matrix[5][5];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': with anonymous union */
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

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer */
    void (*init)(struct ComplexType*);
    
    /* Brackets in array declaration */
    struct StructWithParens* GTY((length("count"))) items[];
    
    /* Braces for nested anonymous struct */
    struct {
        int counter;
        /* Array within nested struct */
        char buffer[256];
    } GTY((skip)) state;
    
    size_t count;
};

/* Test case 5: Pointer to array of function pointers */
typedef int (*GTY((skip)) func_ptr_t)(int);
struct GTY(()) ArrayOfCallbacks {
    /* Multiple bracket types combined */
    func_ptr_t GTY((length("callback_count"))) callbacks[];
    int callback_count;
};

/* Test case 6: Struct with all bracket types in one member */
struct GTY(()) UltimateTest {
    /* Complex declaration: pointer to array of pointers to functions */
    int (*(*GTY((skip)) complex_array[5])(int))[];
    
    /* Another complex example */
    struct {
        int (*compare)(const void*, const void*);
        void* GTY((length("size"))) data[];
    } GTY((skip)) container;
    
    int size;
};

#endif /* TEST_GTY_H */
