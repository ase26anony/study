/* complex_types.h - Header with complex type declarations for gengtype testing */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(15)];
    int (*vla)[DYNAMIC_SIZE(20)];
};

/* 3. Combined: function pointer returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* 4. Struct with array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*handlers[3])(struct Operations *);
};

/* 5. Deeply nested parentheses in macros */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
#define NESTED_CAST(t) (t)((void*)(0))

/* 6. Union with nested struct initializer */
union Container {
    struct {
        int (*func)(int[((sizeof(int)*8))]);
        char data[];
    } s;
    long align;
};

/* 7. Typedef with all three delimiters combined */
typedef struct Node {
    struct Node *children[((2+3)*2)];
    int (*compare)(struct Node *a, struct Node *b);
    union {
        int i;
        double d;
    } value;
} Node;

/* 8. Function prototype with complex return type */
int (*(*get_callback_matrix(void))[5])(int, int);

/* 9. Variable declaration with nested sizeof */
extern int global_table[sizeof(int[(DYNAMIC_SIZE(5)+3)])];

/* 10. Struct with flexible array member containing function pointers */
struct FlexStruct {
    int count;
    int (*funcs[])(int (*)(char), double[2]);
};

#endif /* COMPLEX_TYPES_H */
