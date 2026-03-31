/* complex-types.h - Test file for gengtype balanced delimiter coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* 2. Multi-dimensional and variable-length arrays */
typedef int matrix_t[10][(sizeof(int) > 2) ? 5 : 3];
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
typedef Callback (*(*MetaCallback)(int, Callback))[5];

/* 5. Union with nested initializer-capable struct */
union NestedContainer {
    struct {
        int (*compute)(int, int);
        int values[2][3];
    } calculator;
    struct {
        void (*action)(union NestedContainer *);
        int data;
    } processor;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*wrap_##t)(t (*)(t)))(t)

/* 7. Apply the macro */
DECLARE_COMPLEX_ARRAY(3);
CREATE_NESTED_TYPE(int);

/* 8. Struct with all delimiter types combined */
struct AllDelimiters {
    int (*func_ptr)(int, int);                     /* parentheses */
    int multi_array[2][(sizeof(int) + 1)];         /* brackets */
    struct {
        int x;
        int y;
    } point;                                        /* braces */
    union {
        int i;
        float f;
        int (*array_of_ptrs[2])(void);
    } variant;
};

/* 9. Typedef chain with nested parentheses */
typedef int (*level1)(int);
typedef level1 (*level2)(level1);
typedef level2 (*level3)(level2, level1 (*)(level2));

/* 10. Complex const/volatile qualified types */
typedef int (*(* const volatile cv_fp)(const int, volatile void *))[10];

/* Function prototypes using complex types */
extern complex_fp get_complex_function(void);
extern void use_all_delimiters(struct AllDelimiters *ad);
extern MetaCallback create_meta_callback(int seed);

#endif /* COMPLEX_TYPES_H */
