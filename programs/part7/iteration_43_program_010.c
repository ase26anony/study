/* { dg-do compile } */
/* Complex type declarations to stress gengtype's balanced delimiter parsing */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 10)

struct ArrayStruct {
    int matrix[10][(sizeof(int) * 8)];
    int vla[][DYNAMIC_SIZE(5)];
};

/* 3. Nested struct with function pointer array */
struct Operations {
    int (*ops[5])(int, int);
    struct {
        void (*callback)(void);
        int data;
    } nested;
};

/* 4. Deeply nested typedef chain */
typedef int (*(*Callback)(void))[10];
typedef Callback (*(*MetaCallback)(int, Callback))[5];

/* 5. Union with nested initializer-style type */
union ComplexUnion {
    struct {
        int (*get_value)(union ComplexUnion *);
        void (*set_value)(union ComplexUnion *, int);
    } methods;
    int data[((sizeof(int) + 3) & ~3)];
};

/* 6. Macro generating complex types */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 7. Struct with flexible array member */
struct Flexible {
    int len;
    int arr[];  /* Uncovered: balanced [] parsing */
};

/* 8. Function returning pointer to array of function pointers */
int (*(*get_operations(void))[5])(int, int);

/* 9. Nested parentheses in type casts */
typedef int *(*cast_example)((int (*)(void)), (void *));

/* 10. Struct with bitfield and nested type */
struct BitfieldStruct {
    unsigned int flags : ((sizeof(int) * 8) - 1);
    struct Operations *(*get_ops)(struct BitfieldStruct *);
};

#endif /* COMPLEX_TYPES_H */
