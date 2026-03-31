/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*func_ptr_simple)(int, char);
typedef void (*(*complex_func_ptr)(int (*)(char), double))(float);
typedef int (*(*signal_proto)(int, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)
extern int multi_dim_array[10][DYNAMIC_SIZE(5)][(sizeof(int) * 8)];

/* Test 3: Struct with flexible array member and function pointer array */
struct Container {
    int length;
    int data[];
};

struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced[3])(struct Container*))[10];
};

/* Test 4: Deeply nested typedef chain */
typedef int (*(*Callback)(void))[10];
typedef Callback (*(*Factory)(int, Callback(*)(int)))(char**);

/* Test 5: Union with nested struct initializer pattern */
union NestedUnion {
    struct {
        int x;
        int y[3];
    } point;
    struct {
        void (*action)(int, int);
        int matrix[2][2];
    } state;
};

/* Test 6: Macro generating complex types */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*fp_type##n)(int[(n)]))[n]; \
    fp_type##n var##n

DECLARE_COMPLEX_TYPE(5);
DECLARE_COMPLEX_TYPE(10);

/* Test 7: Type with all three delimiters deeply nested */
typedef struct {
    int (*(*get_matrix)(int size))[(sizeof(int) * 2)][(sizeof(int) * 3)];
    void (*operations[3])(int (*)(char), double);
    union {
        int (*simple)(void);
        struct {
            int count;
            int items[];
        } list;
    } data;
} MasterType;

/* Test 8: Function returning pointer to array of function pointers */
int (*(*get_operations(void))[5])(int, int);

/* Test 9: Const volatile qualified complex pointer */
typedef int (*(* const volatile cv_fp)(const int, volatile char))[10];

/* Test 10: Anonymous struct with bitfields and arrays */
struct {
    unsigned int flags : 3;
    signed int values[2][3];
    void (*handler)(struct Container *c, int (*(*)(void))[5]);
} global_anon;

#endif /* COMPLEX_TYPES_H */
