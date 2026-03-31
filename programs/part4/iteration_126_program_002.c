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
    double matrix[5][5];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
    int id;
    /* Anonymous nested union with braces */
    union {
        int as_int;
        float as_float;
    } GTY((tag("0"))) data;
    
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Test case 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has '(' and '[' */
    int (*(*complex_func)(int))[10];
    
    /* Struct with bitfield using parentheses */
    struct {
        unsigned int flags: (8 * sizeof(unsigned int) - 4);
        /* Array member */
        char name[32];
    } GTY((desc("%1.flags"))) info;
    
    /* Union with anonymous struct */
    union {
        struct {
            int a;
            int b;
        };
        long long combined;
    } GTY((union)) nested_union;
};

/* Test case 5: Pointer to function with complex parameters */
typedef int (*GTY((skip)) ComplexCallback)(
    struct StructWithParens* GTY((skip)), 
    union UnionWithBrackets* GTY((skip)),
    int array[][10]
);

/* Test case 6: Struct with all bracket types in one member */
struct GTY(()) AllInOne {
    /* This has: '(' for function, '[' for array, '{' would be in definition */
    void (*operations[5])(struct AllInOne*);
    
    /* Nested type definition with braces */
    struct GTY(()) Nested {
        int values[ (sizeof(int) > 2) ? 10 : 5 ];
        void (*helper)(void);
    } *nested_ptr;
};

/* Test case 7: Template-like structure with function pointer array */
struct GTY(()) TemplateStruct {
    /* Array of function pointers with parentheses */
    int (*handlers[10])(char *buffer[], int count);
    
    /* Conditional array size with parentheses */
    char buffer[(sizeof(void*) == 8) ? 128 : 64];
    
    /* Anonymous struct with bitfield */
    struct {
        unsigned int : (16);  /* unnamed bitfield */
        unsigned int field: 8;
    } flags;
};

#endif /* TEST_GTY_H */
