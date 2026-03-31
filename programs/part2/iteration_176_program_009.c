/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   logic in gengtype-parse.cc */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_a + _b) * (c); \
})

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_array_of_func_ptrs[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim_array[10][20];
extern int incomplete_array[];
const int const_size = 30;
int var_size_array[const_size];
enum { ARRAY_SIZE = 15 };
int enum_sized_array[ARRAY_SIZE];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(32)));

/* 5. Struct/union definitions with nested braces */
struct Outer {
    int a;
    struct Inner {
        int x;
        union {
            int i;
            double d;
        } u;
    } inner;
    int b;
};

/* 6. Complex initializers with nested braces */
struct Outer global_struct = { 
    .a = 1, 
    .inner = { 
        .x = 2, 
        .u = { 
            .d = 3.14 
        } 
    }, 
    .b = 4 
};

union Data {
    int i;
    float f;
    char str[20];
    struct {
        int x;
        int y;
    } point;
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. Compound literals */
typedef struct Point {
    int x, y;
} Point;

/* 9. __typeof__ usage */
__typeof__(global_struct) another_struct;

/* Main function containing various constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with complex size expression */
    int dynamic_like_array[__builtin_constant_p(1) ? 10 : 20];
    
    /* Nested initializers */
    struct Outer local_struct = {
        .a = FOO(5),
        .inner = {
            .x = BAR(2, 3),
            .u = { .i = 42 }
        },
        .b = COMPLEX_MACRO(1, 2, 3)
    };
    
    /* Compound literal in expression */
    Point *points = (Point[]){{1, 2}, {3, 4}, {5, 6}};
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        (int){42},
        (long){99}
    );
    
    /* Lambda-like expression using statement expression */
    int result = COMPLEX_MACRO(
        local_struct.a,
        points[0].x,
        chosen
    );
    
    /* Multi-dimensional array access with brackets */
    multi_dim_array[FOO(1)][BAR(2, 3)] = result;
    
    /* Complex expression with all bracket types */
    int complex_expr = (multi_dim_array[0][0] + 
                       ((var_size_array[5] * 
                         (global_struct.inner.u.i - 
                          local_struct.b))));
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(&complex_expr));
    
    return complex_expr > 0 ? 0 : 1;
}

/* Additional C++-like constructs (valid in GNU C) */
#ifdef __cplusplus
template<typename T>
T template_func(T x) { return x; }

namespace test {
    class TestClass {
    public:
        TestClass() : value{0} {}
        int value;
    };
}
#else
/* C-specific: designated initializers with array indices */
int array_init[10] = { [0] = 1, [5] = 2, [9] = 3 };
#endif

/* Final struct with bitfield and attribute */
struct Final {
    unsigned int flag:1;
    unsigned int value:31;
} __attribute__((packed)) final_var = {0, 0x7FFFFFFF};

/* Array of function pointers with initializer */
int (*func_array[])(void) = {
    (int (*)(void))main,
    NULL
};
