#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    /* Bitfield with parentheses in size expression */
    unsigned int flags: (sizeof(int) * 8 - 1);
    /* Complex function pointer type */
    void (*complex_callback)(int (*)(double), char*);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    /* Multi-dimensional array */
    double matrix[3][3];
    /* Array with computed size */
    char dynamic_like[(sizeof(int) + 7) & ~7];
    /* Pointer to array */
    int (*ptr_to_array)[5];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
    int id;
    /* Anonymous nested union with braces */
    union {
        int as_int;
        float as_float;
        struct {
            char a;
            char b;
        } as_struct;
    } data;
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array (parentheses and brackets) */
    int (*(*get_array_ptr)(void))[10];
    
    /* Array of function pointers */
    void (*handlers[5])(int);
    
    /* Nested struct with all bracket types */
    struct {
        /* Bitfield with parentheses */
        unsigned int mask: (8 * sizeof(unsigned int) - 1);
        /* Flexible array member */
        int items[];
    } container;
    
    /* Union with array and function pointer */
    union {
        int (*func)(int[3]);
        char buffer[256];
    } variant;
};

/* Test case 5: Pointer to struct with GTY marker */
typedef struct GTY(()) Node {
    struct Node* GTY((skip)) next;  /* Skip this field for GC */
    struct Node* GTY((tag("NODE_TAG"))) child;  /* Tagged pointer */
    int data;
    /* Array with parentheses in size (macro expansion) */
    char name[(256 + 7) & ~7];
} Node;

/* Test case 6: Template-like structure with conditional members */
#ifdef SPECIAL_FEATURE
struct GTY(()) SpecialStruct {
    int special_data;
    /* Function with complex prototype */
    void (*special_init)(struct SpecialStruct*, int (*)(void));
};
#endif

#endif /* TEST_GTY_H */
