#ifndef TEST_GTY_COVERAGE_H
#define TEST_GTY_COVERAGE_H

/* Include standard headers for types */
#include <stddef.h>

/* Type 1: Struct with parentheses (function pointers) */
struct GTY(()) StructWithParens {
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    
    /* Complex function pointer with nested parentheses */
    void (*(*signal_handler)(int (*)(void)))(void);
    
    /* Bitfield with parenthesized expression */
    unsigned int bits: (sizeof(int) * 8 - 1);
    
    /* Pointer to array (mixing parentheses and brackets) */
    int (*array_ptr)[10];
};

/* Type 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* Static array */
    int static_array[100];
    
    /* Multi-dimensional array */
    double matrix[3][3];
    
    /* Flexible array member (requires C99) */
    char flexible_array[];
    
    /* Array of pointers */
    struct StructWithParens* ptr_array[5];
};

/* Type 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
    /* Anonymous nested union with braces */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } GTY((tag("0"))) data;
    
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
    
    /* Another level of nesting */
    struct inner {
        union {
            char c[4];
            int i;
        } GTY((desc("%1.i"))) value;
    } nested;
};

/* Type 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array (parentheses + brackets) */
    int (*(*get_matrix)(void))[10][10];
    
    /* Nested struct with arrays and function pointers */
    struct {
        /* Array of function pointers */
        void (*handlers[5])(struct ComplexType*);
        
        /* Pointer to flexible array */
        char (*dynamic_string)[];
    } GTY((skip)) operations;
    
    /* Union containing various bracket types */
    union {
        /* Function pointer array */
        int (*func_array[3])(int);
        
        /* Pointer to array of structs */
        struct StructWithBraces (*struct_array)[];
        
        /* Direct nested struct */
        struct {
            int counter;
            char name[50];
        } info;
    } GTY((desc("%0.operations.handlers[0] ? 1 : 0"))) container;
    
    /* Bitfield with complex expression */
    unsigned int flags: (sizeof(unsigned int) * 8 - (1 << 2));
};

/* Type 5: Even more complex nested case */
typedef struct GTY(()) TreeNode {
    /* Self-referential pointer */
    struct TreeNode* GTY((skip)) left;
    struct TreeNode* GTY((skip)) right;
    
    /* Function pointer with array parameter */
    void (*visitor)(int data[], int length);
    
    /* Anonymous struct with array of function pointers */
    struct {
        int (*comparators[3])(const void*, const void*);
        void (*cleanup)(void**);
    } GTY((skip)) callbacks;
    
    /* Flexible array of pointers to functions returning pointers to arrays */
    int (*(*dynamic_func_array[]))(int)[];
} TreeNode;

/* Type 6: Template-like structure (simulating generic programming patterns) */
#define DECLARE_GTY_ARRAY(TYPE, SIZE) \
    struct GTY(()) Array_##TYPE { \
        TYPE data[SIZE]; \
        int (*validator)(TYPE (*)[SIZE]); \
        struct { \
            TYPE min; \
            TYPE max; \
        } GTY((skip)) bounds; \
    }

/* Instantiate template-like macros to generate more parsing opportunities */
DECLARE_GTY_ARRAY(int, 10);
DECLARE_GTY_ARRAY(double, 20);
DECLARE_GTY_ARRAY(void*, 5);

/* Type 7: Structure with all bracket types in single member */
struct GTY(()) UltimateBracketTest {
    /* This declaration contains: 
       - Parentheses for function pointer
       - Brackets for arrays  
       - Braces for nested anonymous struct
    */
    int (*(*complex_member[3])(struct {
        int x[10];
        void (*action)(void);
    }*))[5];
    
    /* Another complex example with nested parentheses */
    void (*(*(*nested_func_ptr)(int (*)(char))[2])(float))[10];
};

#endif /* TEST_GTY_COVERAGE_H */
