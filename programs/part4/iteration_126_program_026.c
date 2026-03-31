#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case for parentheses - function pointer */
struct GTY(()) StructWithParens {
    int (*callback)(int, char*);  /* Parentheses for function pointer */
    unsigned int bits: (sizeof(int)*8);  /* Parentheses in bitfield */
    void (*complex_cb)(int (*)(void));  /* Nested parentheses */
};

/* Test case for brackets - arrays */
union GTY(()) UnionWithBrackets {
    int fixed_array[10];  /* Fixed size array */
    char* ptr_array[5];   /* Array of pointers */
    int multi_dim[3][4];  /* Multi-dimensional array */
    int flexible_array[]; /* Flexible array member (requires struct hack) */
};

/* Test case for braces - nested structures */
struct GTY(()) StructWithBraces {
    int value;
    union {  /* Anonymous union with braces */
        int as_int;
        float as_float;
    } data;
    struct {  /* Anonymous struct with braces */
        char* name;
        int id;
    } info;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - uses parentheses and brackets */
    int (*(*func_ptr)(int))[10];
    
    /* Array of function pointers - uses brackets and parentheses */
    void (*callbacks[5])(void);
    
    /* Nested struct with all bracket types */
    struct {
        int (*nested_cb)(int array[5]);  /* Parentheses and brackets */
        union {
            int x;
            float y;
        } values;  /* Braces */
    } nested;
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (8 - 1);
};

/* Another complex case: pointer to array of structs with function pointers */
typedef int (*Comparator)(const void*, const void*);

struct GTY(()) Container {
    Comparator comparators[3];  /* Array of function pointers */
    struct GTY(()) Item* items;  /* Pointer to GTY-marked type */
    size_t count;
};

struct GTY(()) Item {
    int id;
    char* GTY((skip)) name;  /* GTY attribute with parentheses */
    struct Item* GTY((tag("next"))) next;  /* Another GTY attribute */
};

#endif /* TEST_GTY_H */
