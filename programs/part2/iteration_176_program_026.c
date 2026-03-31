/* test_gengtype_coverage.c
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
int multi_dim[10][20][30];
extern int incomplete_array[];
enum { ARRAY_SIZE = 100 };
int sized_array[ARRAY_SIZE];
const int const_size = 50;
int var_array[const_size];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Nested structures with braces */
struct Inner {
    int x;
    double y;
    char z[20];
};

union Data {
    int i;
    float f;
    char str[100];
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int flag3 : 4;
    } bits;
};

struct Outer {
    struct Inner inner;
    union Data data;
    int *ptr_array[5];
    void (*callback)(struct Inner);
};

/* 6. Complex initializers with nested braces */
struct Outer global_outer = {
    .inner = {
        .x = FOO(10),
        .y = 3.14159,
        .z = {'t', 'e', 's', 't', '\0'}
    },
    .data = {
        .bits = {
            .flag1 = 1,
            .flag2 = 3,
            .flag3 = 15
        }
    },
    .ptr_array = {NULL, NULL, NULL, NULL, NULL},
    .callback = NULL
};

/* 7. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Inner another_inner = (struct Inner){.x = 42, .y = 2.71828};

/* 8. Preprocessor conditionals with balanced characters */
#ifdef TEST_MODE
    #define SPECIAL_ARRAY int special[100]
    struct Special {
        int (*func)(int[10]);
    };
#else
    #define SPECIAL_ARRAY int normal[50]
#endif

/* 9. __typeof__ usage */
__typeof__(*compound_literal_ptr) typed_val;
__typeof__(global_outer.inner.x) another_x;

/* 10. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(1, FOO(5), BAR(3, 4));
int constant_p = __builtin_constant_p(ARRAY_SIZE);

/* Main function containing various balanced character constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))NULL;
    
    /* Array with computed size */
    int dynamic_like[FOO(5) * 2];
    
    /* Nested initializer in function scope */
    struct Outer local_outer = {
        .inner = {.x = 100, .y = 1.234},
        .data = {.i = 0xDEADBEEF},
        .callback = (void (*)(struct Inner))0
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){10, 20, 30, 40, 50})[i];
    }
    
    /* __typeof__ in statement */
    __typeof__(sum) sum_copy = sum;
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = 0;
    
    /* Designated initializer with nested braces */
    int matrix[2][3] = {
        [0] = {1, 2, 3},
        [1] = {[2] = 6, [0] = 4, [1] = 5}
    };
    
    /* Use variables to avoid dead code elimination */
    if (global_outer.inner.x > 0) {
        return sum_copy + builtin_result + constant_p + matrix[0][0];
    }
    
    return 0;
}

/* 11. Additional edge cases */
/* Function returning array pointer */
int (*returns_array_ptr(void))[10] {
    static int arr[10][10];
    return arr[5];
}

/* K&R style function definition (uses parentheses differently) */
int old_style(a, b)
    int a;
    int b[][10];
{
    return a + b[0][0];
}

/* 12. C++ style comments to test parser robustness */
// This is a C++ style comment with brackets [like this]
// And parentheses (like this)
// And braces {like this}
