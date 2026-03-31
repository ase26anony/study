/* test-gengtype-coverage.c */
/* This file is specifically crafted to exercise the balanced character
   parsing logic in gengtype-parse.cc lines 341-352 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_a + _b) * (c); \
})

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))[10];

/* 3. Array declarations with brackets (multi-dimensional and variable) */
int multi_array[10][20];
enum { ARRAY_SIZE = 15 };
const int const_size = 20;
int var_array[ARRAY_SIZE][const_size];
int runtime_array[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(32)));

/* 5. C++-like alignas (C11/C++11) */
#ifdef __cplusplus
alignas(32) double aligned_double;
#else
_Alignas(32) double aligned_double;
#endif

/* 6. Nested structures with braces */
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

/* 7. Complex initializer with nested braces */
struct Outer global_struct = {
    .a = FOO(1),
    .inner = {
        .x = 42,
        .u = { .f = 3.14f }
    },
    .b = BAR(2, 3)
};

/* 8. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
} data = { .f = 2.718f };

/* 9. Array with nested initializer */
int matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};

/* 10. Compound literals */
int *compound_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer *struct_literal = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .b = 40
};

/* 11. Preprocessor conditionals with balanced characters */
#ifdef TEST_CONDITIONAL
int conditional_array[FOO(5)][BAR(2, 3)];
struct Conditional {
    int (*func)(int[][FOO(3)]);
} cond_var = { NULL };
#endif

/* 12. __typeof__ with parentheses */
__typeof__(*compound_ptr) typed_val;
__typeof__(global_struct.inner.u.f) float_val;

/* 13. GCC builtins with complex expressions */
int chosen = __builtin_choose_expr(1, FOO(10), BAR(20, 30));
long builtin_array[__builtin_types_compatible_p(int, long) ? 5 : 10];

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_array[chosen][2] = {{1, 2}, {3, 4}};
    
    /* Nested compound literal */
    int *nested_literal = (int[]){FOO(1), BAR(2, 3), COMPLEX_MACRO(1, 2, 3)};
    
    /* Structure with brace initializer */
    struct Outer local_struct = {
        .a = 100,
        .inner = {
            .x = 200,
            .u = { .i = 300 }
        },
        .b = 400
    };
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += compound_ptr[i];
        }
        sum;
    });
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = result;
    
    /* Use __typeof__ in declaration */
    __typeof__(local_struct.a) copy_a = local_struct.a;
    
    /* Return computation using all constructs */
    return (FOO(copy_a) + 
            local_array[0][0] + 
            nested_literal[0] + 
            local_aligned + 
            global_struct.a) % 256;
}

/* 14. Additional top-level constructs */
/* Function declaration with array parameter */
void process_matrix(int mat[][10], int rows);
/* Pointer to array */
int (*array_ptr)[10];
/* Anonymous struct in union */
union Anon {
    struct {
        int a, b;
    };
    long long ll;
};

/* 15. More complex macro with nested parentheses */
#define NESTED_PARENS(a) ((((a) + 1) * 2) - 3)
int nested_result = NESTED_PARENS(FOO(BAR(2, 3)));

/* 16. Array with attribute */
int attributed_array[10] __attribute__((aligned(64)));

/* 17. Struct with flexible array member */
struct Flex {
    int count;
    int data[];
};

/* 18. Initializer with nested designators */
struct Deep {
    struct {
        struct {
            int deepest;
        } middle;
    } top;
} deep = { .top.middle.deepest = 999 };

/* 19. Switch case in function (contains braces) */
void dummy_func(void) {
    switch (deep.top.middle.deepest) {
        case 999:
            /* Empty case with braces */
            {
                int temp = 0;
            }
            break;
        default:
            break;
    }
}

/* 20. Multiple bracket pairs in declaration */
int (*(*complex_array[5])(int))[10];
