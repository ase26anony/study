/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*complex_fp1)(int (*)(char), double);
typedef int (*(*nested_fp)(int (*)(char), double))(float);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define ARRAY_SIZE (10 + (sizeof(int) * 2))
extern int multi_dim[5][(ARRAY_SIZE > 15) ? 10 : 5][3];
typedef int matrix_t[10][(sizeof(double) + 2)];

/* Test 3: Structs with flexible array members and nested structs */
struct Outer {
    int id;
    struct {
        int x;
        int y;
        int (*calc)(struct Outer*, int);
    } inner;
    int data[];
};

/* Test 4: Union with function pointer array */
union ComplexUnion {
    int (*func_array[3])(int, int);
    struct {
        int (*nested_func)(union ComplexUnion*);
        char name[20];
    } info;
};

/* Test 5: Typedef combining function pointer returning array pointer */
typedef int (*(*Callback)(void))[10];
typedef char (*(*(*triple_indirect)(int, char(*)[5]))[3])(float);

/* Test 6: Struct with array of function pointers */
struct Operations {
    const char* name;
    int (*ops[5])(int, int);
    void (*init)(struct Operations*, int);
};

/* Test 7: Macro generating complex types */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX((2+3));

/* Test 8: Nested anonymous structs/unions */
struct Container {
    struct {
        union {
            int (*func)(int);
            long value;
        } u;
        int arr[2][3];
    } nested;
};

/* Function prototypes using complex types */
extern void process_matrix(int (*mat)[][10], int rows);
extern struct Outer* create_outer(int size, int (*init_func)(int));
extern Callback get_callback(int id);

#endif /* COMPLEX_TYPES_H */
