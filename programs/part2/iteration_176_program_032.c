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
void (*signal_handler)(int sig, void (*handler)(int));
int (*(*nested_func_ptr)(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
extern int extern_array[];
int var_size[FOO(5) + BAR(2, 3)];
enum { ARRAY_SIZE = 100 };
int enum_sized[ARRAY_SIZE];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));
/* Vector types use brackets in some GCC extensions */
typedef int v4si __attribute__((vector_size(16)));

/* 5. Aggregate types with nested structures and unions */
struct Outer {
    int a;
    union {
        int i;
        double d;
        struct {
            char c;
            short s;
        } nested;
    } u;
    int arr[5];
};

/* 6. Complex initializers with braces */
struct Outer global_outer = {
    .a = FOO(10),
    .u = {
        .nested = {
            .c = 'x',
            .s = 42
        }
    },
    .arr = {1, 2, 3, 4, 5}
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. __typeof__ with parentheses */
#define MAX(a, b) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b; \
})

/* 9. Compound literals */
typedef struct Point {
    int x, y;
} Point;

/* 10. Main function containing all triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int dynamic_like[sizeof(struct Outer) / sizeof(int)];
    
    /* Compound literal */
    Point *p = &(Point){ .x = 10, .y = 20 };
    
    /* Nested initializer */
    struct Outer local_outer = {
        .a = 100,
        .u = { .i = 42 },
        .arr = { [0] = 1, [4] = 5 }
    };
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        (int[]){1, 2, 3, 4},
        (int[]){0}
    )[0];
    
    /* Lambda-like statement expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += local_outer.arr[i];
        }
        sum;
    });
    
    /* Multi-dimensional array access with brackets */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int elem = matrix[1][2];
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 42;
    
    /* Use of all variables to prevent optimization */
    return (global_outer.a + p->x + local_outer.u.i + result + elem + choice + local_aligned) % 256;
}

/* 11. Additional complex type at file scope */
union FinalUnion {
    struct {
        int (*funcs[5])(void);
        void *data;
    } s;
    long long raw[4];
} final_union = {
    .s = {
        .funcs = {0, 0, 0, 0, main},
        .data = &global_outer
    }
};

/* 12. C++ style comment to test parser handling */
// This should be ignored by the parser
