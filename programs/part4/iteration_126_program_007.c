#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': consume_balanced('(', ')') */
    int (*callback)(int, char*);
    unsigned int bits: (sizeof(int)*8);  /* More parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* This will trigger case '[': consume_balanced('[', ']') */
    int fixed_array[10];
    char variable_array[];
    int (*array_of_pointers[5])(void);
    size_t sized_array[sizeof(int) * 2];  /* Parentheses inside brackets */
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': consume_balanced('{', '}') */
    union {
        int as_int;
        float as_float;
        struct {
            char a;
            char b;
        } nested;  /* Nested struct with braces */
    } data;
    struct {
        int x;
        int y;
    } point;  /* Another struct with braces */
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses in function pointer type */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets for array declaration */
    struct StructWithParens* items[20];
    
    /* Braces for nested anonymous struct */
    struct {
        int counter;
        /* Function pointer with parentheses */
        int (*validate)(int GTY((skip)), float);
        /* Array with brackets */
        char name[50];
    } metadata;
    
    /* Flexible array member with brackets */
    unsigned char flexible_data[];
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY((user)) (*func_ptr_t)(void);
struct GTY(()) Container {
    /* Complex type: pointer to array of function pointers */
    func_ptr_t (*get_func_table(void))[10];
    
    /* Nested struct with all bracket types */
    struct GTY(()) {
        int (*comparator)(const void*, const void*);
        void* data_array[100];
        union {
            long l;
            double d;
        } value;
    } helper;
};

#endif /* TEST_GTY_H */
