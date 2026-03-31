/* test_gengtype_coverage.c
 * This file is designed to trigger balanced character parsing in gengtype.
 * It contains constructs that cause the parser to encounter parentheses,
 * brackets, and braces in various contexts.
 */

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
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int variable_array[FOO(5)][BAR(2, 3)];
extern int incomplete_array[];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(16)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
#  define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
#  define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct/union definitions with nested initializers */
struct Outer {
    int a;
    struct Inner {
        int x;
        union {
            int i;
            float f;
        } u;
    } inner;
    int b;
};

/* Global instance with complex initializer */
struct Outer global_struct = {
    .a = FOO(1),
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = {4}
};

/* 7. Another struct with array member */
struct WithArray {
    int data[5];
    struct Outer *ptr;
};

/* 8. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 9. Enum with parentheses in array size */
enum { ARRAY_SIZE = (10 + 5) * 2 };
int sized_array[ARRAY_SIZE];

/* 10. Function using all constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_array[GCC_SPECIFIC(1) ? 10 : 20];
    
    /* Nested struct initialization */
    struct Outer local_var = {
        .a = 1,
        .inner = { .x = 2, .u = { .i = 3 } },
        .b = 4
    };
    
    /* Compound literal */
    struct WithArray *wa = &(struct WithArray){
        .data = {5, 6, 7, 8, 9},
        .ptr = &global_struct
    };
    
    /* __typeof__ with parentheses */
    __typeof__(*global_ptr) val = 42;
    
    /* GCC builtins with parentheses */
    int result = __builtin_choose_expr(
        sizeof(int) == 4,
        __builtin_popcount(0xFF),
        0
    );
    
    /* Lambda-like expression using nested braces */
    struct { int x; int y; } point = { .x = 10, .y = 20 };
    
    /* Multi-level array access with brackets */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int elem = matrix[1][2];
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = 0;
    
    /* Alignas specifier (C11/C++11) */
    _Alignas(32) char aligned_buffer[64];
    
    /* Complex expression with all bracket types */
    int complex_expr = (BAR(FOO(1), 2) + matrix[0][(point.x > 5) ? 1 : 0]) 
                       * sizeof(struct Outer[COMPLEX_MACRO(1, 2, 3)]);
    
    /* Prevent dead code elimination */
    if (complex_expr > 0) {
        return complex_expr % 100;
    }
    
    return 0;
}

/* 11. Additional constructs at file scope */
/* Pointer to array */
int (*ptr_to_array)[10];

/* Function returning pointer to array */
int (*func_returning_array(int x))[5] {
    static int arr[5];
    return &arr;
}

/* Nested parentheses in declarator */
int (*(*(*extra_complex)(int))[5])(void);

/* 12. __attribute__ with nested parentheses */
void __attribute__((constructor(101))) init_func(void) {
    /* Empty */
}

/* 13. Designated initializers with nested braces */
int nested_init[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 14. Statement expression (GNU extension) */
int stmt_expr_result = ({
    int sum = 0;
    for (int i = 0; i < 5; i++) sum += global_ptr[i];
    sum;
});

/* 15. __builtin_constant_p in array size */
int builtin_array[__builtin_constant_p(1) ? 10 : 20];
