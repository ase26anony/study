/* { dg-do compile } */
/* Complex type declarations with nested delimiters for gengtype coverage */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* 2. Multi-dimensional and variable-length arrays */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
struct flexible_array {
    int len;
    int arr[];
};

/* 3. Nested struct with function pointer array */
struct Operations {
    int (*ops[5])(int, int);
    void (*handlers[3])(struct Operations *);
};

/* 4. Deeply nested function pointer returning array pointer */
typedef int (*(*Callback)(void))[10];
typedef Callback (*(*MetaCallback)(int))[5];

/* 5. Struct with nested anonymous unions/structs */
struct Container {
    union {
        struct {
            int x;
            int y;
        } point;
        struct {
            int (*compute)(int, int);
            int result;
        } calc;
    } data;
    int flags;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(8);
DECLARE_COMPLEX(16);

/* 7. Type with all three delimiters combined */
typedef struct {
    int (*get_value)(int index);
    int values[10];
    struct {
        int count;
        int (*process[3])(int, int);
    } processor;
} CombinedType;

/* 8. Function pointer with array parameter */
typedef void (*array_handler)(int arr[][10], int (*callback)(int));

/* 9. Nested typedef chain */
typedef int (*level1)(int);
typedef level1 (*level2)(char);
typedef level2 (*level3)(double);

/* 10. Struct with bitfield and function pointer */
struct BitStruct {
    unsigned int flags : 3;
    int (*action)(struct BitStruct *, int);
    union {
        int ival;
        void *ptr;
    } value;
};

#endif /* COMPLEX_TYPES_H */
