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
    /* These will trigger case '[': */
    int fixed_array[10];
    char variable_array[];
    int multi_dim[5][3];
    /* Array of function pointers - combines both ( and [ */
    void (*func_array[5])(void);
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': */
    union {
        int as_int;
        float as_float;
        char as_char;
    } GTY((tag("0"))) data;
    /* Another nested struct with braces */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    char* (*allocator)(size_t);
    
    /* Brackets in array declaration */
    struct StructWithParens* GTY((length("count"))) items[];
    
    /* Braces for nested anonymous struct */
    struct {
        /* Nested array with parentheses in size expression */
        int matrix[2 * (3 + 1)][4];
        
        /* Function pointer array */
        int (*operations[3])(int, int);
    } GTY((skip)) container;
    
    int count;
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*ComplexFuncPtr)(int, char*)[];
struct GTY(()) ContainerOfArrays {
    /* Multi-level combination: ( ) [ ] { } */
    struct {
        ComplexFuncPtr* GTY((length("func_count"))) funcs;
        int func_count;
    } GTY((skip)) func_container;
    
    /* Array of structs with function pointers */
    struct StructWithParens GTY((length("item_count"))) item_list[];
    int item_count;
};

/* Test case 6: Union with all bracket types */
union GTY(()) AllBracketsUnion {
    /* Parentheses */
    void (*func)(void);
    
    /* Brackets */
    double matrix[3][3];
    
    /* Braces */
    struct {
        int tag;
        union {
            int i;
            float f;
        } value;
    } tagged;
};

#endif /* TEST_GTY_H */
