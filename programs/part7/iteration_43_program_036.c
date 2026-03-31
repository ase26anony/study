/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

typedef int matrix_t[10][(sizeof(int) * 8)];
typedef char buffer_t[DYNAMIC_SIZE(16)][DYNAMIC_SIZE(32)];

/* Test 3: Struct with flexible array member containing function pointers */
struct FlexibleContainer {
    int count;
    int (*handlers[])(struct FlexibleContainer *, int);
};

/* Test 4: Union with nested struct containing arrays */
union NestedUnion {
    struct {
        int (*compare)(const void *, const void *);
        void *data[((sizeof(void *) * 8) / 2)];
    } s;
    struct {
        union NestedUnion *next;
        int values[3][(4 + 1)];
    } u;
};

/* Test 5: Function returning pointer to array of function pointers */
typedef int (*(*CallbackFactory)(int mode))[5];

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*func_array##n[(n)])(void))[n]

/* Test 7: Struct with all delimiter types combined */
struct AllDelimiters {
    /* Parentheses in function pointer */
    int (*operation)(int, int);
    
    /* Brackets in multi-dimensional array */
    double matrix[3][(2 * 2)];
    
    /* Nested struct with initializer-style comment */
    struct {
        char *name;
        int (*validators[2])(const char *);
    } config;
};

/* Test 8: Deeply nested function pointer type */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(char *))(void);

/* Test 9: Type with sizeof/alignof expressions in array bounds */
typedef struct {
    unsigned char data[sizeof(long double)];
    int padding[__alignof__(long double) / sizeof(int)];
} AlignedBuffer;

/* Test 10: Complex const/volatile qualified function pointer */
typedef const int *(*(*cv_fp)(volatile int *))[10];

/* Forward declarations for cross-file type references */
struct ForwardDecl;
typedef struct ForwardDecl *(*ForwardProcessor)(struct ForwardDecl **, int);

#endif /* COMPLEX_TYPES_H */
