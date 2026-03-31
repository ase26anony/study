/* complex-types.h - Test file for gengtype balanced delimiter coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*signal_handler)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)

struct ArrayTest {
    int matrix[10][DYNAMIC_SIZE(15)];
    int (*ptr_matrix)[(sizeof(int) * 8)][5];
};

/* 3. Flexible array member in nested struct */
struct Outer {
    int id;
    struct {
        int len;
        int data[];
    } inner;
};

/* 4. Complex function pointer returning pointer to array */
typedef int (*(*Callback)(void))[10];
typedef char (*(*(*nested_callback)(int))[5])(double);

/* 5. Struct containing array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*(*advanced[3])(struct Operations *))[2];
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 7. Union with nested struct and array */
union Container {
    struct {
        int type;
        union {
            int i;
            double d;
            void *p;
        } value;
    } tagged;
    char buffer[sizeof(struct { int a; double b; })];
};

/* 8. Typedef chain with nested parentheses */
typedef struct Node Node;
struct Node {
    Node *next;
    void *data;
    int (*compare)(Node *, Node *);
};

/* 9. Function prototype with complex parameter */
extern void register_callback(int (*(*get_cb)(void))(int), 
                              char (*(*names)[])[20]);

/* 10. Variable declaration with cast-like type */
extern int (*(*global_table)[(sizeof(int)*8)]);

/* 11. Nested anonymous struct in union */
struct AnonymousTest {
    union {
        struct {
            int x;
            int y;
        };
        struct {
            double a;
            double b;
        } dbl;
    } coords;
};

/* 12. Bitfield with sizeof in array bound */
struct BitfieldStruct {
    unsigned int flags : (sizeof(int) * 8 - 4);
    char array[(sizeof(struct BitfieldStruct) + 15) & ~15];
};

#endif /* COMPLEX_TYPES_H */
