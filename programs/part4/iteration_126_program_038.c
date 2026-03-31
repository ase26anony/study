#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This triggers case '(': */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* These trigger case '[': */
    int fixed_array[10];
    char string_array[256];
    double matrix[3][3];
    /* Flexible array member */
    int flexible_array[];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* This triggers case '{': with anonymous union */
    union {
        int int_value;
        float float_value;
        void* ptr_value;
    } data;
    /* Another nested struct with braces */
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses: function pointer */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets: multi-dimensional array */
    int (*process_matrix[5])(int matrix[][10]);
    
    /* Braces: nested struct */
    struct {
        /* Nested with all bracket types */
        union {
            int (*handlers[3])(void);
            struct {
                char name[50];
                int (*validator)(const char*);
            } validators[2];
        } u;
        
        /* Array of function pointers */
        float (*calculations[10])(int, float);
    } operations;
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        char* (*get_name)(void);
    } items[];
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*FuncPtrArray[10])(int, char**);

/* Test case 6: Struct with bitfield using parentheses */
struct GTY(()) BitfieldStruct {
    /* Parentheses in bitfield width */
    unsigned int a: (8 - 1);
    unsigned int b: (16 / 2);
    unsigned int c: (sizeof(int) * 8);
};

/* Test case 7: Union with nested anonymous struct in array */
union GTY(()) NestedUnion {
    struct {
        int x;
        int y;
    } points[100];
    
    struct {
        /* Array of structs with function pointers */
        struct {
            char* name;
            int (*compare)(const void*, const void*);
        } comparators[5];
    } cmp_data;
};

#endif /* TEST_GTY_H */
