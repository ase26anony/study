/* complex-types.h - Header with complex nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*signal_handler)(int sig, void (*callback)(int)))(void);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct Matrix {
    int rows;
    int cols;
    int data[DYNAMIC_SIZE(10)][DYNAMIC_SIZE(20)];
};

/* 3. Flexible array member in nested struct */
struct Outer {
    int id;
    struct {
        int len;
        int flexible[];
    } inner;
};

/* 4. Function returning pointer to array */
typedef int (*(*array_returner)(void))[10];

/* 5. Struct with array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*(*advanced[3])(char))[2];
};

/* 6. Deeply nested parentheses in function pointer */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(char))[5];

/* 7. Combined delimiters in single declaration */
union Combined {
    struct {
        int (*func_ptr)(int[2][3]);
        char (*name)[(sizeof(int) * 2)];
    } s;
    void *ptr;
};

/* 8. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[(n)])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*nested_##t##_ptr)(t))[sizeof(t)]

/* 9. Struct with bitfield and nested array */
struct BitfieldStruct {
    unsigned int flags : 3;
    signed int values[4][(8 / sizeof(int))];
    struct BitfieldStruct *next;
};

/* 10. Type with sizeof/alignof in array bounds */
typedef struct SizedType {
    char buffer[sizeof(long double)];
    int aligned[__alignof__(long double)];
} SizedType;

/* 11. Function prototype with all delimiter types */
extern void process_all(
    int (*callback)(int[][3], struct { int x; int y; }), 
    char *names[],
    union Combined data
);

/* 12. Nested anonymous struct/unions */
struct Container {
    int tag;
    union {
        struct {
            int (*compute)(int (*)(int), int);
            float matrix[2][(4 + 1)];
        } numeric;
        struct {
            char *(*formatter)(const char *, ...);
            void *data;
        } text;
    } content;
};

#endif /* COMPLEX_TYPES_H */
