/* complex-types.h - Header file with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(void (*(*)(int))(double)))(char);

/* 2. Multi-dimensional and variable-length arrays */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
typedef struct {
    int len;
    int arr[];
} flexible_array_t;

/* 3. Struct with nested initializer-style declaration */
struct Point {
    int x;
    int y;
    int z;
};

struct NestedStruct {
    struct Point points[3];
    int (*operations[2])(struct Point, struct Point);
};

/* 4. Combined patterns - function pointer returning array pointer */
typedef int (*(*callback_t)(void))[10];

/* 5. Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*signal[3])(int, void (*)(int)))(int);
};

/* 6. Deeply nested parentheses in function declarations */
extern int (*(*(*deep_nested)(int (*(*)(double))[3]))(float))[5];

/* 7. Macro that expands to complex type with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
#define NESTED_MACRO(x) ((x) + ((x) * (x)))

/* 8. Union with nested struct */
union ComplexUnion {
    struct {
        int (*func)(int[((sizeof(int)*8)/2)]);
        float matrix[2][(2+3)];
    } nested;
    long long data;
};

/* 9. Typedef with all three delimiter types deeply nested */
typedef struct {
    int (*(*get_callback)(int arg))[(arg > 0) ? 10 : 5];
    union {
        struct Point p;
        int arr[3][2];
    } data;
    void (*cleanup)(struct Operations*);
} MasterType;

/* 10. Function prototypes with complex parameter types */
extern void process_matrix(int matrix[][(10+5)], int (*compare)(int, int));
extern struct NestedStruct* create_nested(int (*(*factory)(void))[3]);

#endif /* COMPLEX_TYPES_H */
