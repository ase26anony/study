/* test-gengtype-coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
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
extern int incomplete[];
enum { SIZE = 100 };
int sized_by_enum[SIZE];
const int const_size = 50;
int var_arr[const_size];
int expr_arr[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. C++ style alignas (C11/C23) */
_Alignas(16) int aligned_var;
_Alignas(double) char aligned_char;

/* 6. Structures with nested initializers */
struct Inner {
    int x;
    double y;
};

struct Outer {
    int id;
    struct Inner inner;
    union {
        int a;
        double b;
    } data;
};

/* Global struct with complex initializer */
struct Outer global_struct = {
    .id = 42,
    .inner = { .x = 1, .y = 2.0 },
    .data = { .b = 3.14159 }
};

/* 7. Union with designated initializer */
union Mixed {
    int i;
    float f;
    char str[20];
};

union Mixed global_union = { .str = "Hello" };

/* 8. Preprocessor conditionals containing triggering constructs */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
    int likely_var = GCC_SPECIFIC(1);
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 9. __typeof__ usage */
__typeof__(*complex_func_ptr) func_type;
__typeof__(multi_dim[0]) row_type;

/* 10. Compound literals */
int *compound_lit = (int[]){1, 2, 3, 4, 5};
struct Inner *inner_lit = &(struct Inner){ .x = 10, .y = 20.5 };

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_arr[sizeof(local_func) * 2];
    
    /* Nested initializer in local scope */
    struct Outer local_struct = {
        .id = 100,
        .inner = { 
            .x = BAR(5, 2),
            .y = 3.14 
        },
        .data = { .a = 42 }
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){1, 2, 3, 4, 5})[i];
    }
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        (int){100},
        (int){200}
    );
    
    /* Lambda-like statement expression (GCC extension) */
    int result = COMPLEX_MACRO(1, 2, 3);
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = result;
    
    /* Array with attribute */
    char aligned_buffer[64] __attribute__((aligned(32)));
    
    /* Use __typeof__ in local scope */
    __typeof__(local_struct.id) id_copy = local_struct.id;
    
    /* Prevent dead code elimination */
    if (sum > 0 && chosen > 0 && result > 0) {
        return 0;
    }
    
    return 1;
}

/* 11. Additional top-level constructs */

/* Function with complex return type */
int (*(*make_array(void))[10])(void) {
    static int (*array[10])(void);
    return &array;
}

/* Structure with flexible array member */
struct FlexArray {
    int count;
    int data[];
};

/* Initializer with nested braces */
int matrix[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

/* Macro with nested parentheses */
#define NESTED_PARENS(a) ((((a) + 1) * 2) - 3)
int nested_result = NESTED_PARENS(42);

/* Attribute with multiple arguments */
int packed_var __attribute__((packed, aligned(4)));

/* Conditional expression in array size */
int cond_array[sizeof(int*) == 8 ? 64 : 32];

/* Use of offsetof which involves parentheses */
#include <stddef.h>
size_t offset = offsetof(struct Outer, data);

/* Array of function pointers */
int (*func_array[5])(int, int);

/* Typedef with parentheses */
typedef int (*callback_t)(void*, int);
callback_t my_callback;

/* Forward declaration with attributes */
int forward_decl(int x) __attribute__((warn_unused_result));

/* Inline function with attributes */
static inline int inline_func(int x) __attribute__((always_inline));
static inline int inline_func(int x) {
    return x * 2;
}

/* Final check - ensure all constructs are used */
void final_check(void) {
    /* Use all global variables to prevent warnings */
    (void)complex_func_ptr;
    (void)multi_dim[0][0];
    (void)attr_var;
    (void)vector_var;
    (void)deprecated_var;
    (void)aligned_var;
    (void)aligned_char;
    (void)global_struct;
    (void)global_union;
    (void)likely_var;
    (void)func_type;
    (void)row_type;
    (void)compound_lit;
    (void)inner_lit;
    (void)nested_result;
    (void)packed_var;
    (void)cond_array[0];
    (void)offset;
    (void)func_array[0];
    (void)my_callback;
}
