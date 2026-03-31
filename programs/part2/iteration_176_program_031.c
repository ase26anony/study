/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{'.
 */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define COMPLEX_MACRO(x) (sizeof((x)) + __alignof__(typeof(x)))

/* 2. Complex declarators with parentheses */
int (*global_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
extern int incomplete[];
const int const_array[] = {1, 2, 3};
int var_len[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Nested struct/union with brace initializers */
struct Outer {
    int a;
    union {
        int b;
        double c;
    } inner;
    struct {
        int d[3];
        char e;
    } nested;
};

/* Global instance with complex initializer */
struct Outer global_outer = {
    .a = 42,
    .inner = { .c = 3.14 },
    .nested = { .d = {1, 2, 3}, .e = 'x' }
};

/* 6. Another struct with designated initializers */
struct Point {
    int x, y, z;
};

struct Point points[] = {
    [0] = { .x = 1, .y = {2}, .z = 3 },
    [1] = { 4, 5, 6 },
    { 7, 8, 9 }
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. Complex type with __typeof__ */
__typeof__(*global_func_ptr) func_return_type;

/* 9. Enum with array size */
enum { ARRAY_SIZE = 100 };
char large_buffer[ARRAY_SIZE];

/* Main function containing various balanced constructs */
int main(void) {
    /* Compound literal with braces */
    int *dynamic = (int[]){10, 20, 30, 40};
    
    /* Nested function calls with parentheses */
    int result = ADD(MAX(global_outer.a, 50), dynamic[1]);
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        __builtin_constant_p(result),
        result * 2,
        result / 2
    );
    
    /* Local struct with initializer */
    struct Outer local = {
        .a = chosen,
        .inner = { .b = 100 },
        .nested.d[0] = {99}
    };
    
    /* Array with computed index in brackets */
    int idx = (chosen > 50) ? 0 : 1;
    local.nested.d[idx] = points[idx].x;
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int lambda_result = ({
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            sum += local.nested.d[i];
        }
        sum;
    });
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = lambda_result;
    
    /* Complex declarator in local scope */
    void (*local_func)(int) = (void (*)(int))0;
    
    /* Use all variables to avoid dead code elimination */
    return global_outer.a + result + chosen + local_attr;
}

/* 10. Additional top-level constructs */
/* Function declaration with complex return type */
int (*(*complex_return(void))[5])(void);

/* Union with anonymous struct */
union Mixed {
    struct {
        int a;
        char b;
    };
    double c;
};

/* Initializer with nested braces */
union Mixed mixed = { .a = 1, .b = 'a' };

/* 11. More macro expansions */
#define NESTED_PARENS(x) (((x) + 1) * ((x) - 1))
int macro_expanded = NESTED_PARENS(global_outer.a);

/* 12. __alignof__ with parentheses */
size_t alignment = __alignof__(struct Outer);

/* 13. Offsetof with parentheses */
#include <stddef.h>
size_t offset = offsetof(struct Outer, nested.d[1]);

/* 14. Static assertion (C11) */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 15. Asm statement with braces (GCC extension) */
void asm_example(void) {
    __asm__ volatile (
        "mov %0, %%eax\n"
        "add $1, %%eax"
        : /* outputs */
        : "r" (global_outer.a) /* inputs */
        : "%eax" /* clobbers */
    );
}
