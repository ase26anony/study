/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   logic in gengtype-parse.cc, specifically the switch cases for
   '(', '[', and '{' characters. */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) \
    do { \
        (a) = (b) + (c); \
    } while(0)

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[10][20];
int arr3[FOO(5)][BAR(2, 3)];
extern int var_arr[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((format(printf, 1, 2)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
# define ALIGN_VAL 16
#else
# define ALIGN_VAL 8
#endif

/* 6. Struct/union definitions with braces */
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

/* 7. Complex initializers with nested braces */
struct Outer global_struct = {
    .a = FOO(1),
    .inner = {
        .x = 42,
        .u = {
            .f = 3.14f
        }
    },
    .b = {2}
};

/* 8. Compound literals */
int *global_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer *struct_ptr = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .b = 40
};

/* 9. __typeof__ usage */
__typeof__(*global_ptr) typed_val;
__typeof__(complex_func_ptr) *typed_func_ptr;

/* 10. C++ style alignas (C11/C++11) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Alignas(16) int aligned_var;
#endif

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_arr[sizeof(struct Outer) / sizeof(int)];
    
    /* Nested initializers */
    struct Outer local_struct = {
        .a = BAR(2, 3),
        .inner = {
            .x = __builtin_choose_expr(1, 100, 200),
            .u = { .i = 42 }
        },
        .b = {0}
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1, 2, 3})[0] + ((int[]){4, 5, 6})[1];
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        (int){100},
        (int){200}
    );
    
    /* Complex expression with all bracket types */
    int result = FOO(
        BAR(
            local_struct.a,
            ((int[][2]){{1,2},{3,4}})[1][0]
        )
    ) + choice;
    
    /* Prevent dead code elimination */
    if (global_ptr[0] + result > 0) {
        return 0;
    }
    
    return 1;
}

/* 11. Additional edge cases */
/* Function with array parameter */
void process_array(int matrix[][10], int rows);
/* K&R style function definition (if supported) */
int old_style_func(x, y)
    int x;
    int y[];
{
    return x + y[0];
}

/* 12. Nested macro expansion with all bracket types */
#define NESTED_MACRO(a) \
    ({ \
        int _r = (a) + arr1[0]; \
        struct Outer _s = { .a = _r }; \
        _r; \
    })

/* 13. Designated initializers with array indices */
struct WithArray {
    int data[5];
} array_struct = {
    .data = {[0] = 1, [2] = 3, [4] = 5}
};

/* 14. Anonymous struct/union (C11) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
struct Anonymous {
    union {
        struct {
            int x, y;
        };
        int coords[2];
    };
} anon = { .x = 1, .y = 2 };
#endif
