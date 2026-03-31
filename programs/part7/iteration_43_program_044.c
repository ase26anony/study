/* complex-types.h - Header with complex nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef void (*complex_fp1)(int (*)(char), double);
typedef int (*(*complex_fp2)(int (*)(char*), void*))(float);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(3)];
};

/* Test 3: Flexible array member in nested struct */
struct Outer {
    int count;
    struct {
        int id;
        char data[];
    } inner;
};

/* Test 4: Function returning pointer to array */
typedef int (*(*callback_func)(void))[10];
typedef int (*(*(*nested_callback)(int))[5])(void);

/* Test 5: Struct with array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced[3])(int))[2];
};

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*wrap_##t)(t (*)(t)))(t)

/* Test 7: Union with nested struct initializers */
union NestedUnion {
    struct {
        int x;
        struct {
            int a;
            int b;
        } inner;
    } s;
    long arr[2][3];
};

/* Test 8: Type with all three delimiters combined */
typedef struct {
    int (*get_value)(int index);
    int values[10];
    struct {
        int (*compare)(int a, int b);
        int result;
    } helper;
} CombinedType;

/* Test 9: Deeply nested parentheses in function pointer */
typedef int (*(*(*(*deep_nested)(int (*(*)(int[2]))(float))))(void))[3];

/* Test 10: Array of pointers to functions returning pointers to arrays */
int (*(*func_array[5])(int))[2];

/* External declarations for multi-file testing */
extern CombinedType* create_combined(void);
extern void process_operations(struct Operations* ops);

#endif /* COMPLEX_TYPES_H */
