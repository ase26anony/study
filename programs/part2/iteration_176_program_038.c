/* test-gengtype-coverage.c
 * This file is designed to trigger balanced character parsing in gengtype-parse.cc
 * Specifically targeting lines 341-352 handling '(', '[', and '{' characters.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED_MACRO(x) FOO(BAR((x), 2))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
extern int incomplete[];
enum { SIZE = 100 };
int sized_by_enum[SIZE];
const int const_size = 50;
int var_arr[const_size];
int gcc_attribute_arr[10] __attribute__((aligned(16)));

/* 4. typeof with parentheses */
__typeof__(*complex_func_ptr) func_return_type;

/* 5. Preprocessor conditionals containing balanced characters */
#ifdef __GNUC__
# define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
# define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct/union definitions with nested initializers */
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
struct Outer global_var = { 
    .a = FOO(1),
    .inner = { .c = 3.14 },
    .nested = { 
        .d = { [0] = 1, [2] = 3 },
        .e = 'X'
    }
};

/* 7. Another struct with designated initializers and nested braces */
struct Point {
    int x, y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
} rect = {
    .top_left = { .x = 0, .y = 10 },
    .bottom_right = { .x = 20, .y = 0 }
};

/* 8. Compound literal */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};

/* 9. GCC builtins with parentheses and ternary */
static inline int choose_expr_test(void) {
    return __builtin_choose_expr(
        __builtin_constant_p(1),
        42,
        sizeof(int[10])
    );
}

/* 10. C++ style alignas (C11/C++11) */
#ifdef __cplusplus
alignas(16) int aligned_var;
#else
_Alignas(16) int aligned_var;
#endif

/* 11. Vector attribute with brackets (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* Main function containing various balanced character constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_arr[sizeof(struct Outer) / sizeof(int)];
    
    /* Nested initializer in local scope */
    struct Outer local_var = {
        .a = BAR(2, 3),
        .inner = { .b = NESTED_MACRO(5) },
        .nested = { 
            .d = { 9, 8, 7 },
            .e = 'Z'
        }
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){10, 20, 30})[1];
    
    /* __typeof__ in local scope */
    __typeof__(local_var.nested.d) d_copy;
    
    /* GCC builtin with parentheses */
    int likely_true = GCC_SPECIFIC(1);
    
    /* Complex expression with all bracket types */
    int result = FOO(
        BAR(
            multi_dim[0][0],
            local_var.nested.d[likely_true ? 0 : 1]
        )
    ) + sizeof(int[choose_expr_test()]);
    
    /* Prevent dead code elimination */
    if (result > 1000) {
        return 1;
    }
    
    return 0;
}

/* 12. Additional constructs at file scope to ensure parsing */
#if 0
/* This disabled block contains more examples that the parser will skip over */
int (*disabled_func)(int[10][20]) = (int (*)(int[10][20])){0};
struct { int a; } disabled_var = { .a = ({ int x = 5; x; }) };
#endif

/* 13. K&R style function definition (uses parentheses differently) */
int old_style_func(x, y)
    int x;
    char *y;
{
    return x + (*y);
}

/* 14. Nested parentheses in macro arguments */
#define ULTRA_NESTED(a, b, c) (a + (b * (c - 1)))
int ultra_result = ULTRA_NESTED(1, (2+3), ((4)));

/* 15. Attribute with parentheses containing brackets */
void __attribute__((constructor(101))) init_func(void) {
    /* Empty constructor */
}

/* 16. Asm statement with braces (GCC extension) */
void asm_example(void) {
    int src = 42, dst;
    __asm__ volatile (
        "mov %1, %0\n\t"
        : "=r"(dst)
        : "r"(src)
    );
}
