/* test-gengtype-balanced.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

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
int (*(*nested_func_ptr)(void))[10];

/* 3. Array declarations with brackets - multi-dimensional */
int multi_array[10][20];
float matrix[3][3][3];

/* 4. Arrays with non-constant size using enum */
enum { SIZE = 100 };
int variable_array[SIZE];

/* 5. Arrays with conditional size using ternary */
const int const_size = 50;
int cond_array[__builtin_constant_p(1) ? 10 : 20];

/* 6. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(16)));

/* 7. C++-like alignas specifier (C11/C++11) */
_Alignas(16) int aligned_c11;

/* 8. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int x;
            int y;
        } point;
        struct {
            float radius;
            float angle;
        } polar;
    } data;
    int (*callback)(int);
};

/* Global instance with nested brace initializer */
struct Outer global_instance = {
    .type = 1,
    .data = {
        .point = {
            .x = FOO(10),
            .y = BAR(2, 3)
        }
    },
    .callback = NULL
};

/* 9. Another struct with designated initializers */
struct Nested {
    int a;
    struct {
        int b[3];
        char c;
    } inner;
    int d;
};

/* 10. Preprocessor conditionals containing balanced characters */
#ifdef TEST_DEFINE
    #define CONDITIONAL_MACRO(x) [(x) + 1]
    int conditional_array[CONDITIONAL_MACRO(5)];
#else
    #define CONDITIONAL_MACRO(x) ((x) * 2)
#endif

/* 11. __typeof__ usage with parentheses */
__typeof__(*global_instance.callback) callback_type;

/* 12. Function using GCC builtins with parentheses */
static int use_builtins(void) {
    return __builtin_choose_expr(1, 42, 0);
}

/* 13. Compound literals */
static int *get_array(void) {
    return (int[]){1, 2, 3, 4, 5};
}

/* 14. Nested struct initialization in function */
static struct Nested init_nested(void) {
    return (struct Nested){
        .a = 1,
        .inner = {
            .b = {[0] = 10, [2] = 20},
            .c = 'X'
        },
        .d = 3
    };
}

/* 15. Main function mixing all constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(double) = (int (*)(double))NULL;
    
    /* Array with computed size */
    int dynamic_like[sizeof(struct Outer) / sizeof(int)];
    
    /* Compound literal in expression */
    int *ptr = (int[][3]){{1, 2, 3}, {4, 5, 6}}[0];
    
    /* Nested initializer */
    struct Outer local = {
        .type = 2,
        .data = {
            .polar = {
                .radius = 3.14f,
                .angle = 1.57f
            }
        },
        .callback = (int (*)(int))use_builtins
    };
    
    /* __typeof__ with parentheses */
    __typeof__(local.data.point) point_copy = {.x = 1, .y = 2};
    
    /* Macro expansion with parentheses */
    int result = COMPLEX_MACRO(1, 2, 3);
    
    /* Array access with brackets */
    multi_array[0][0] = result;
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 42;
    
    /* Prevent dead code elimination */
    if (global_instance.type == 1 && local.type == 2) {
        return result + multi_array[0][0] + local_aligned;
    }
    
    return 0;
}

/* 16. Additional constructs at file scope */
#if 0
/* Commented out section with more balanced characters */
int (*commented_func)(int (*)(int[][3]), struct {int a;});
#endif

/* 17. Empty struct/union for edge cases */
struct Empty {};

/* 18. Anonymous struct in union */
union Mixed {
    struct {
        int a;
        int b;
    };
    float f;
};

/* 19. Pointer to array */
int (*ptr_to_array)[10];

/* 20. Function returning pointer to array */
int (*func_returning_array(int x))[10] {
    static int arr[10];
    return &arr;
}
