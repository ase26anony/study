#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types */
#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* Function pointer member - triggers '(' case */
    int (*callback)(int, char*);
    /* Bitfield with parentheses in expression */
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
    /* Nested anonymous union - triggers '{' case */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;
    /* Another nested struct */
    struct {
        short x;
        short y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_callback)[5])(void);
    
    /* Struct with bitfield and nested union */
    struct {
        unsigned int flags: (8 * sizeof(unsigned int) - 4);
        union {
            long long big;
            int small[2];
        } value;
    } GTY((skip)) metadata;  /* Nested GTY marker with parentheses */
    
    /* Array of function pointers */
    void (*handlers[3])(struct ComplexType*);
    
    /* Pointer to flexible array struct */
    struct GTY(()) FlexStruct {
        size_t len;
        int data[];
    } *flex;
};

/* Test case 5: Typedef with GTY annotation */
typedef struct GTY(()) {
    int tag;
    union {
        int ival;
        double dval;
        char* sval;
    } u;
} AnonymousTypedef;

/* Test case 6: Pointer chain with various brackets */
struct GTY(()) PointerChain {
    /* Pointer to array of pointers to functions */
    int (*(*func_table)[10])(int);
    
    /* Pointer to struct with flexible array */
    struct GTY(()) HasFlexArray {
        int count;
        struct PointerChain* items[];
    } *container;
};

#endif /* TEST_GTY_H */
