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

/* 3. Array declarations with brackets - multi-dimensional and variable */
int multi_array[10][20][30];
enum { ARRAY_SIZE = 100 };
const int const_size = 50;
int variable_array[const_size];
int expr_array[FOO(10) * BAR(2, 3)];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_var;
#ifdef __cplusplus
alignas(32) double aligned_double;
#endif

/* 6. Nested structures with brace initializers */
struct Inner {
    int x;
    double y;
    char z[20];
};

union Data {
    int i;
    float f;
    char str[30];
};

struct Outer {
    int id;
    struct Inner inner;
    union Data data;
    int *ptr_array[5];
};

/* Global instance with complex initializer */
struct Outer global_instance = {
    .id = 42,
    .inner = {
        .x = 10,
        .y = 3.14,
        .z = "Hello"
    },
    .data = { .f = 2.718 },
    .ptr_array = { NULL, NULL, NULL, NULL, NULL }
};

/* 7. Preprocessor conditionals with balanced characters */
#ifdef TEST_FEATURE
int conditional_array[100] = { [0 ... 99] = 1 };
#else
int conditional_array[50] = {0};
#endif

/* 8. __typeof__ with parentheses */
__typeof__(*complex_func_ptr) func_return_type;
__typeof__(multi_array[0][0]) element_type;

/* 9. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Inner inline_instance = (struct Inner){ .x = 5, .y = 2.5, .z = "Test" };

/* 10. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(1, FOO(10), BAR(20, 30));
int constant_p = __builtin_constant_p(42);
long long builtin_ll = __builtin_expect(FOO(100), 1);

/* Main function containing all triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_array[builtin_result] = {0};
    
    /* Nested initializer */
    struct Outer local_instance = {
        .id = 99,
        .inner = {
            .x = 20,
            .y = 1.618,
            .z = "World"
        },
        .data = { .i = 314159 },
        .ptr_array = { 
            compound_literal_ptr,
            &local_array[0],
            &global_instance.inner.x,
            NULL,
            NULL
        }
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){1, 2, 3, 4, 5})[i];
    }
    
    /* __typeof__ in declaration */
    __typeof__(sum) sum_copy = sum;
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = sum_copy;
    
    /* Array with designated initializer */
    int designated[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Nested switch-like macro expansion */
    int result = COMPLEX_MACRO(1, 2, 3);
    
    /* Prevent dead code elimination */
    if (result > 0) {
        return result + sum + local_instance.id + global_instance.id;
    }
    
    return 0;
}

/* 11. Additional constructs at file scope */
typedef int (*callback_t)(int, void*);
callback_t callbacks[] = {
    NULL,
    (int (*)(int, void*))main,
    NULL
};

/* 12. Struct with bitfields and arrays */
struct BitfieldStruct {
    unsigned int flags : 4;
    unsigned int value : 12;
    char name[FOO(20)];
    int *ptr;
};

/* 13. Union with anonymous struct */
union AnonymousUnion {
    struct {
        int a;
        int b;
    };
    long long ll;
};

/* 14. Array of function pointers */
int (*func_array[3])(int) = {
    NULL,
    NULL,
    NULL
};

/* 15. Final check with all balanced characters in sequence */
void final_check(void) {
    /* This line contains all three balanced characters in sequence */
    int (*p)(int) = (int (*)(int)){0}; /* parens, parens, braces */
    int arr[][2] = {{1, 2}, {3, 4}}; /* brackets, braces, braces */
    struct { int a; } s = { .a = 1 }; /* braces, parens, braces */
}
