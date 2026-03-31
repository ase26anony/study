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

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int tag;
    /* This will trigger case '{': */
    union {
        int as_int;
        float as_float;
        char* as_string;
    } data;
    /* Another nested struct */
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
    struct StructWithParens* items[100];
    
    /* Braces for nested anonymous struct */
    struct {
        int counter;
        /* Array within nested struct */
        char buffer[256];
    } state;
    
    /* Function pointer returning pointer to array */
    int (*(*complex_callback)(void))[10];
    
    /* Flexible array member */
    unsigned char extra_data[];
};

/* Test case 5: Pointer to struct with GTY marker */
typedef struct GTY(()) Node {
    int id;
    struct Node* GTY((skip)) next;  /* Skip this field for GC */
    struct Node* children[4];
    /* Function pointer array */
    void (*handlers[5])(struct Node*);
} Node_t;

/* Test case 6: Union with all bracket types */
union GTY(()) AllBracketsUnion {
    /* Parentheses in bitfield */
    struct {
        unsigned int a: (8);
        unsigned int b: (16);
    } bits;
    
    /* Brackets in array */
    long array[(sizeof(struct StructWithParens) + 7) / 8];
    
    /* Braces for anonymous struct */
    struct {
        void (*func)(void);
        char str[50];
    } compound;
};

#endif /* TEST_GTY_H */
