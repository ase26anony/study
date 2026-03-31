/* test-gengtype-coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{'.
 */

/* 1. Function-like macros with parentheses */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SQUARE(x) ((x) * (x))
#define COMPLEX_MACRO(a, b, c) (((a) + (b)) * (c))

/* 2. Complex declarators with parentheses */
int (*func_ptr_with_params)(int, double, char *);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_fp)(void))[10];

/* 3. Array declarations with brackets */
int multi_dim_array[5][10][15];
extern int incomplete_array[];
enum { ARRAY_SIZE = 20 };
int sized_array[ARRAY_SIZE];
int variable_array[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));
int vector_type __attribute__((vector_size(16)));

/* 5. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_c11;
#ifdef __cplusplus
alignas(32) double aligned_cpp;
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

/* 7. Complex initializer with nested braces */
struct Outer global_var = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14f }
    },
    .b = {4}
};

/* 8. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
} data = { .str = "Hello" };

/* 9. Array with nested initializer */
int matrix[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

/* 10. Preprocessor conditionals containing balanced characters */
#ifdef TEST_FEATURE
    #define FEATURE_MACRO(x) do { \
        int temp = (x) + 1; \
        printf("%d\n", temp); \
    } while(0)
#else
    #define FEATURE_MACRO(x) ((void)0)
#endif

/* 11. __typeof__ usage */
__typeof__(*func_ptr_with_params) func_return_type;
__typeof__(multi_dim_array[0]) first_dim;

/* 12. Compound literals */
struct Point {
    int x, y;
};

/* Main function containing various balanced character constructs */
int main(void) {
    /* Function pointer usage with parentheses */
    int (*local_fp)(int) = NULL;
    
    /* Array usage with brackets */
    int local_arr[10] = {0};
    local_arr[5] = 100;
    
    /* Compound literal with braces */
    struct Point p = (struct Point){ .x = 10, .y = 20 };
    
    /* Nested compound literal in expression */
    int *dynamic_array = (int[]){1, 2, 3, 4, 5};
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        42,
        24
    );
    
    /* Complex expression with all balanced characters */
    int result = MAX(
        SQUARE(
            (matrix[1][1] + p.x)
        ),
        COMPLEX_MACRO(
            local_arr[2],
            choice,
            3
        )
    );
    
    /* Nested struct initialization in function */
    struct Outer local_outer = {
        .a = result,
        .inner = {
            .x = 100,
            .u = { .i = 200 }
        },
        .b = 300
    };
    
    /* Array with designators */
    int sparse[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Nested switch-like macro expansion */
    FEATURE_MACRO(result);
    
    /* Return statement with complex expression */
    return (
        (local_outer.a + local_outer.b) *
        (p.x + p.y) /
        (sizeof(multi_dim_array) / sizeof(multi_dim_array[0]))
    );
}

/* 13. Additional constructs at file scope */
/* Function declaration with __attribute__ and parameters */
void __attribute__((noreturn)) fatal_error(const char *msg, ...);

/* Array of function pointers */
int (*func_array[5])(void);

/* Struct containing arrays and nested structs */
struct Container {
    int (*callbacks[10])(int, void*);
    struct {
        int data[100];
        char *names[50];
    } nested;
};

/* Initializer for the above struct */
struct Container container_instance = {
    .callbacks = { NULL, NULL, NULL },
    .nested = {
        .data = { [0] = 1, [99] = 100 },
        .names = { [0] = "first", [49] = "last" }
    }
};

/* 14. More GCC extensions */
/* Statement expression with braces */
#define MIN(a,b) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a < _b ? _a : _b; \
})

/* Use the macro */
int min_val = MIN(10, 20);

/* 15. __auto_type usage (C23/GCC extension) */
#ifdef __GNUC__
    __auto_type auto_var = &global_var;
#endif

/* 16. Nested parentheses in sizeof */
size_t complex_size = sizeof(int[10][sizeof(struct Outer)]);
