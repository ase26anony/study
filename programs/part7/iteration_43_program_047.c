/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(int (*(*)(void))[5]))(void);

/* 2. Multi-dimensional and variable-length arrays */
struct Matrix {
    int data[10][(sizeof(int) > 4) ? 8 : 4];
    int (*ptr_matrix)[3][4];
};

/* Flexible array member with nested size calculation */
struct FlexArray {
    int count;
    int items[/* flexible */];
    struct Matrix matrices[];
};

/* 3. Nested aggregate types with braces in declarations */
union NestedUnion {
    struct {
        int x;
        struct {
            int a;
            int b;
        } inner;
    } data;
    long long raw[2];
};

/* 4. Complex typedef combining all delimiter types */
typedef union NestedUnion (*(*ultra_complex)(int arr[3][(2+1)], 
                                             struct Matrix (*mat)[2]))[10];

/* 5. Struct containing array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*(*signal_handlers[3])(int, void (*)(int)))(int);
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define NESTED_MACRO_TYPE(x) struct { int (*func)(int arr[x][x+1]); }

/* 7. Type with nested parentheses in sizeof expressions */
struct SizeDependent {
    char buffer[sizeof(int[(sizeof(double) > 8) ? 16 : 8])];
    int (*get_size)(void);
};

/* 8. Function prototype with complex return type */
struct Matrix *(*get_matrix_factory(int size))(int rows, int cols);

/* 9. Anonymous struct with nested initializer-style declaration */
struct {
    int (*comparator)(const void *, const void *);
    union {
        int i;
        float f;
        struct { char c; int n; } s;
    } value;
} global_anon;

#endif /* COMPLEX_TYPES_H */
