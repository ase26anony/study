/* { dg-do compile } */
/* Complex type definitions to test gengtype's consume_balanced function */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
typedef struct {
    int len;
    int arr[];
} flexible_array_t;

/* 3. Deeply nested parentheses for function pointers */
typedef int (*(*(*deep_nested_fp)(void))(int, int))(char);

/* 4. Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*handlers[3])(struct Operations *);
};

/* 5. Combined delimiters: function returning pointer to array */
typedef int (*(*callback_ret_array_t)(void))[10];

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 7. Union with nested struct initializer pattern */
union Container {
    struct {
        int (*compare)(const void *, const void *);
        void *data[((sizeof(void*) * 8) / 2)];
    } s;
    long long alignment;
};

/* 8. Typedef with all three delimiters combined */
typedef struct Node {
    struct Node *(*get_next)(struct Node *);
    int values[((sizeof(struct Node*) + 3) / 4)];
    union {
        int (*as_int)(void);
        void (*as_void)(int);
    } converter;
} Node_t;

/* 9. Function prototype with complex parameter */
extern void register_callback(callback_t cb, int priority[(callback == 0) ? 1 : 2]);

/* 10. Variable declaration with cast-like parentheses */
extern int (*(*global_table)[(sizeof(int)*8)]);

#endif /* FILE1_H */
