/* test-gengtype-coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{'.
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
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_size[FOO(5)][BAR(2, 3)];
extern int incomplete[];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
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
    union {
        int b;
        double c;
    } inner;
    struct {
        int x[3];
        char y;
    } nested;
};

/* Global instance with complex initializer */
struct Outer global_struct = {
    .a = FOO(1),
    .inner = { .c = 3.14 },
    .nested = {
        .x = { [0] = 1, [1] = 2, [2] = BAR(3, 4) },
        .y = 'Z'
    }
};

/* 7. Another struct with designated initializers */
struct Point {
    int x, y, z;
};

struct Line {
    struct Point start, end;
} line = {
    .start = { .x = 0, .y = 0, .z = 0 },
    .end = { .x = 10, .y = 20, .z = COMPLEX_MACRO(1, 2, 3) }
};

/* 8. C++ style alignas (C11/C++11) */
#ifdef __cplusplus
alignas(32) double aligned_double;
#else
_Alignas(32) double aligned_double;
#endif

/* 9. __typeof__ usage */
__typeof__(*complex_func_ptr) func_type;

/* Main function containing various balanced character constructs */
int main(void) {
    /* Compound literal with braces */
    int *arr = (int[]){1, 2, 3, 4, 5};
    
    /* Nested initializers */
    struct Outer local = {
        .a = 42,
        .inner = { .b = 100 },
        .nested = { .x = {9, 8, 7}, .y = 'A' }
    };
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        FOO(10),
        BAR(20, 30)
    );
    
    /* Array with computed size using ternary */
    int dynamic_like[__builtin_constant_p(1) ? 10 : 20];
    
    /* Complex expression with all bracket types */
    int result = FOO(
        BAR(
            arr[choice % 5],
            line.end.z
        )
    ) + global_struct.nested.x[1];
    
    /* Prevent dead code elimination */
    volatile int sink = result;
    
    /* Lambda-like expression in GNU C */
    int (*lambda)(int) = ({
        int __fn(int x) { return x * x; }
        __fn;
    });
    
    /* Statement expression with nested braces */
    int stmt_expr = ({
        int temp = 0;
        for (int i = 0; i < 5; i++) {
            temp += arr[i];
        }
        temp;
    });
    
    return sink + stmt_expr + lambda(3);
}

/* 10. Additional constructs at file scope */
#ifdef TEST_CONDITIONAL
    /* This block should be skipped by gengtype but still parsed */
    int conditional_var[FOO(10)] = { [0] = 1, [FOO(8)] = 2 };
#endif

/* 11. Function declaration with attributes */
void __attribute__((constructor(101))) 
__attribute__((noinline)) 
init_func(void) {
    /* Empty but has attribute parentheses */
}

/* 12. Array with nested designators */
int nested_designator[3][2] = {
    [0] = {1, 2},
    [1][0] = 3,
    [2] = {[1] = 4}
};

/* 13. Struct containing flexible array member */
struct Flex {
    int count;
    int data[];
};

/* 14. Union with anonymous struct */
union Anonymous {
    struct {
        int a, b;
    };
    double d;
};

/* 15. Final check: all three characters in sequence */
void* mixed_chars = (void*[FOO(3)]) {
    (void*){&global_struct},
    (void*[2]){&line, &local},
    &arr
};
