#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Trigger parentheses handling with function pointers */
struct GTY(()) StructWithParens {
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    
    /* Bitfield with parenthesized expression */
    unsigned int bits: (sizeof(int) * 8 - 1);
    
    /* Nested function pointer */
    void (**nested_callback)(double);
};

/* Test case 2: Trigger brackets handling with arrays */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[100];
    
    /* Multi-dimensional array */
    double matrix[10][20];
    
    /* Array with computed size */
    char variable_array[sizeof(struct StructWithParens)];
    
    /* Flexible array member */
    int flexible_array[];
};

/* Test case 3: Trigger braces handling with nested structs/unions */
struct GTY(()) StructWithBraces {
    int id;
    
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
    } GTY(()) point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array (parentheses and brackets) */
    int (*(*complex_callback)(void))[10];
    
    /* Array of function pointers */
    void (*callbacks[5])(int);
    
    /* Nested anonymous struct with bitfield */
    struct {
        unsigned int flags: (8);
        char name[50];
    } GTY(()) info;
    
    /* Union with array member */
    union {
        int ints[4];
        double doubles[2];
    } GTY((desc("%1.ints[0]"))) values;
};

/* Test case 5: Pointer chain with all bracket types */
typedef struct GTY(()) Node {
    struct Node* GTY((skip)) next;
    
    /* Array of pointers with parenthesized size */
    void* GTY((length("(count)"))) *items;
    
    /* Function pointer array */
    int (*handlers[3])(struct Node*);
    
    /* Nested type definition */
    union {
        struct {
            int start;
            int end;
        } range;
        int single;
    } GTY(()) data_union;
    
    int count;
} Node;

/* Test case 6: Template-like structure with macros */
#define DECLARE_VECTOR(TYPE) \
    struct GTY(()) vector_##TYPE { \
        TYPE* GTY((length("size"))) elements; \
        size_t size; \
        size_t capacity; \
        void (*resize)(struct vector_##TYPE*, size_t); \
    }

DECLARE_VECTOR(int);
DECLARE_VECTOR(double);
DECLARE_VECTOR(Node*);

#endif /* TEST_GTY_H */
