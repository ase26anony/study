#ifndef TEST_GTY_PARSER_H
#define TEST_GTY_PARSER_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': consume_balanced('(', ')') */
    int (*callback)(int, char*);
    unsigned int bits: (sizeof(int) * 8 - 1);  /* More parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* This will trigger case '[': consume_balanced('[', ']') */
    int fixed_array[10];
    char variable_array[];
    int (*array_of_ptrs[5])(void);
    struct {
        int nested_array[3][4];
    } inner;
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': consume_balanced('{', '}') */
    union {
        int as_int;
        float as_float;
        char as_char;
    } data;
    struct {
        int x;
        int y;
    } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets in array declaration */
    struct StructWithParens* items[20];
    
    /* Braces for nested anonymous struct */
    struct {
        int counter;
        /* Nested array with parentheses in size expression */
        char buffer[(sizeof(int) * 4)];
    } state;
    
    /* Multi-dimensional array */
    int matrix[3][4];
    
    /* Function pointer returning pointer to array */
    int (*(*complex_callback)(void))[10];
};

/* Test case 5: Another struct with bitfield containing parentheses */
struct GTY(()) BitfieldStruct {
    unsigned int a: (8 - 1);
    unsigned int b: (16 / 2);
    unsigned int c: (sizeof(char) * 8);
};

/* Test case 6: Union with function pointer array */
union GTY(()) UnionComplex {
    /* Array of function pointers */
    int (*func_array[5])(int, float);
    
    /* Nested struct with all bracket types */
    struct {
        int (*nested_func)(void);
        char name[50];
        struct {
            int x;
            int y;
        } coord;
    } nested;
};

#endif /* TEST_GTY_PARSER_H */
