#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types we'll use */
#include <stddef.h>

/* Struct with parentheses - function pointer type */
struct GTY(()) StructWithParens {
    /* Function pointer with parentheses */
    int (*callback)(int, char*);
    
    /* Bitfield with parenthesized expression */
    unsigned int flags: (sizeof(int) * 8 - 1);
    
    /* Pointer to function returning pointer */
    char* (*(*complex_callback)(void))(int);
};

/* Union with brackets - array declarations */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Multi-dimensional array */
    double matrix[3][3];
    
    /* Zero-length array (GCC extension) */
    char flexible_array[0];
    
    /* Array with computed size */
    long dynamic_size[sizeof(void*) == 8 ? 16 : 8];
};

/* Struct with braces - nested anonymous union */
struct GTY(()) StructWithBraces {
    int type;
    
    /* Anonymous union with braces */
    union {
        int int_value;
        float float_value;
        void* ptr_value;
    } GTY((skip)) data;  /* skip annotation has parentheses too */
    
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has both () and [] */
    int (*(*get_array)(int size))[];
    
    /* Array of function pointers - has both [] and () */
    void (*handlers[10])(void);
    
    /* Struct containing union containing array - nested braces and brackets */
    struct {
        union {
            int ints[5];
            char chars[20];
        } container;
        int count;
    } GTY((tag("container_tag"))) nested;  /* tag annotation with parentheses */
    
    /* Bitfield with parenthesized size calculation */
    unsigned int status: (sizeof(unsigned int) * 8 - 4);
};

/* Another struct with even more complex nested patterns */
struct GTY(()) VeryComplex {
    /* Pointer to array of function pointers */
    int (*(* (*callbacks)[5])(void))[10];
    
    /* Anonymous struct with bitfield */
    struct {
        unsigned int a: 3;
        unsigned int b: (8 - 3);
    };
    
    /* Union with array of structs */
    union {
        struct {
            int x;
            int y;
        } points[100];
        float coords[200];
    } GTY((desc("%1.type"))) geometry;
};

/* Typedef with GTY annotation */
typedef struct GTY(()) {
    int id;
    char name[50];
    struct ComplexType* GTY((skip)) next;
} NamedStruct;

#endif /* TEST_GTY_H */
