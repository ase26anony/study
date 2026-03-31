/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20][30];
enum { SIZE = 100 };
int var_size[SIZE];
const int const_size = 50;
int var_arr[const_size + 10];

/* 4. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(16)));
int attr3 __attribute__((deprecated("use attr2 instead")));

/* 5. Struct with nested union and complex initializer */
struct Outer {
    int a;
    union {
        int b;
        double c;
        struct {
            char d;
            short e;
        } inner;
    } u;
    int *f[5];
};

/* Global instance with nested brace initializer */
struct Outer global = {
    .a = 1,
    .u = { .inner = { .d = 'x', .e = 42 } },
    .f = { NULL, NULL, NULL, NULL, NULL }
};

/* 6. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = { .x = 1, .y = {2}, .z = 3 },
    [1] = { .x = 4, .y = 5, .z = 6 },
    { 7, 8, 9 }
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. __typeof__ usage */
__typeof__(*global.f[0]) type_var;

/* 9. Compound literals */
int *compound_lit = (int[]){1, 2, 3, 4, 5};
struct Point *point_ptr = &(struct Point){ .x = 10, .y = 20, .z = 30 };

/* 10. Function declaration with __attribute__ */
void my_func(int) __attribute__((noreturn));

/* Main function containing various constructs */
int main(void) {
    /* Use function-like macros */
    int x = FOO(42);
    int y = BAR(x, 2);
    int z = NESTED(y);
    
    /* Array access with brackets */
    multi_dim[0][1][2] = z;
    var_arr[10] = x;
    
    /* Use compound literal */
    int *p = (int[]){10, 20, 30, 40};
    
    /* GCC built-in with parentheses */
    int choice = __builtin_choose_expr(sizeof(int) == 4, 100, 200);
    
    /* __typeof__ in expression */
    __typeof__(choice) choice_copy = choice;
    
    /* Nested initializer */
    struct Outer local = {
        .a = choice_copy,
        .u = { .b = 999 },
        .f = { p, p+1, p+2, p+3, p+4 }
    };
    
    /* Access nested struct */
    local.u.inner.d = 'a';
    local.u.inner.e = 123;
    
    /* Array with computed size */
    int dyn_size[choice_copy > 150 ? 10 : 20];
    
    /* Use all variables to prevent dead code elimination */
    return (x + y + z + multi_dim[0][1][2] + var_arr[10] + 
            p[0] + choice + choice_copy + local.a + 
            local.u.inner.d + local.u.inner.e + dyn_size[0]);
}

/* 11. Additional constructs at file scope */
/* Function pointer array with initializer */
static int (*func_ptrs[])(void) = {
    NULL,
    NULL,
    NULL
};

/* Union with anonymous struct */
union Mixed {
    struct {
        int a;
        int b;
    };
    double c;
};

/* 12. More complex macro with nested parentheses */
#define VERY_COMPLEX(a, b, c) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        __typeof__(c) _c = (c); \
        (_a + _b) * _c; \
    })

/* 13. Statement expression (GCC extension) */
int stmt_expr = ({
    int temp = 0;
    for (int i = 0; i < 10; i++) {
        temp += i;
    }
    temp;
});

/* 14. Alignas specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[256];

/* 15. Static assertion with parentheses */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");
