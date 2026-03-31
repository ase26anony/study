/* Complex type declarations with deeply nested delimiters */
/* This aims to trigger consume_balanced calls in gengtype-parse.cc */

/* 1. Function pointers with complex signatures */
int (*(*complex_callback)(int (*)(float)))[10];
int (*(*(*nested_func_ptr)(char (*)[5]))(void))[3];

/* 2. Multi-dimensional arrays with initializers */
int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};
int cube[2][3][4] = {
    {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
    {{13,14,15,16}, {17,18,19,20}, {21,22,23,24}}
};

/* 3. Structures with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        long d;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        int (*nested[2])(char (*)[5]);
        struct Inner {
            float matrix[2][2];
        } inner;
    } extra;
};

/* 4. Type definitions with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);
typedef int (*(*complex_array_ptr)[10])(float, double);
typedef struct {
    int (*(*member)[5])(void);
} StructWithFuncPtr;

/* 5. GCC attributes with nested parentheses */
int __attribute__((aligned(32))) aligned_var;
int __attribute__((format(printf, 2, 3))) printf_func(int, const char *, ...);
void __attribute__((constructor(101))) init_func(void);
void __attribute__((destructor(101))) cleanup_func(void);

/* 6. Macros generating delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define NESTED_ARRAY_TYPE(dim) int (*(*nested_array_##dim)[dim][dim])(void)

MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);
NESTED_ARRAY_TYPE(3);
NESTED_ARRAY_TYPE(4);

/* 7. Complex expressions with nested delimiters in sizeof */
size_t sizes[] = {
    sizeof(int[10][20]),
    sizeof(int (*(*)[5])(void)),
    sizeof(struct Outer),
    sizeof(((struct Outer*)0)->func_ptr_arr),
    sizeof(int (*[3])(char (*)[5]))
};

/* 8. Compound literals */
struct Outer create_complex(void) {
    return (struct Outer){
        .a = 1,
        .b = 2,
        .func_ptr_arr = {NULL, NULL},
        .extra = {
            .nested = {NULL, NULL},
            .inner = {.matrix = {{1.0, 2.0}, {3.0, 4.0}}}
        }
    };
}

/* 9. Cast expressions with nested parentheses */
void* complex_cast = (void*)(int (*(*)[5])(char (*)[3]));

/* 10. Function with complex parameter types */
void process_complex(
    int (*(*callback)(int (*)(float)))[10],
    struct Outer (*processor)(int, ...),
    __attribute__((aligned(16))) int aligned_param
) {
    /* Nested block with local complex type */
    {
        int (*(*local_var)[5])(char (*)[3]);
        local_var = NULL;
        
        /* sizeof with cast expression */
        size_t s = sizeof((int (*(*)[5])(char (*)[3]))0);
        (void)s;
    }
    
    /* Array of function pointers initialization */
    array_of_5_funcs funcs = {NULL, NULL, NULL, NULL, NULL};
    (void)funcs;
}

/* 11. Union with nested struct and bitfields */
union MegaUnion {
    struct {
        int a : 3;
        int b : 5;
        struct {
            int x : 2;
            int y : 6;
        } nested;
    } bits;
    long long as_long;
    double as_double;
    int (*(*as_func_ptr)[3])(void);
};

/* 12. Enum with last value for array sizing */
enum { ARRAY_SIZE = 100 };

/* 13. Variable length array in parameter */
void vla_function(int n, int m, int arr[n][m]) {
    /* Compound literal with designators */
    int (*ptr)[m] = (int (*)[m])&arr[0][0];
    (void)ptr;
}

/* 14. Nested switch with compound statements */
int nested_switch_test(int x) {
    switch (x) {
        case 1: {
            int matrix[2][2] = {{1,2},{3,4}};
            return matrix[0][0];
        }
        case 2: {
            struct { int a[3]; } s = {{5,6,7}};
            return s.a[1];
        }
        default:
            return 0;
    }
}

/* Main function - minimal but uses complex types */
int main(void) {
    /* Declare variables of complex types */
    struct Outer outer_instance = create_complex();
    array_of_5_funcs func_array = {NULL};
    
    /* Use sizeof on complex types */
    size_t s1 = sizeof(int (*(*)[5])(char (*)[3]));
    size_t s2 = sizeof(outer_instance.func_ptr_arr);
    
    /* Compound literal */
    int (*array_ptr)[4] = (int (*)[4])&matrix[0];
    (void)array_ptr;
    
    /* Call function with complex signature */
    process_complex(NULL, NULL, 42);
    
    /* Test nested switch */
    int result = nested_switch_test(1);
    
    /* Use VLA */
    int vla_arr[5][10];
    vla_function(5, 10, vla_arr);
    
    return result;
}
