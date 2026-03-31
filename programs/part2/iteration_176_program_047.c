/* test-gengtype-coverage.c */
/* This file is designed to trigger consume_balanced() calls in gengtype-parse.cc */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a + _b; })

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_size[FOO(5)][BAR(2, 3)];
enum { SIZE = 8 };
int enum_array[SIZE][SIZE];
const int const_size = 16;
int const_array[const_size];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Nested structure with union and complex initializer */
struct Outer {
    int a;
    union {
        int i;
        double d;
        char str[20];
    } u;
    struct {
        int x;
        int y;
        int arr[3][2];
    } nested;
};

/* Global instance with nested brace initializer */
struct Outer global_struct = { 
    .a = FOO(1),
    .u = { .d = 3.14 },
    .nested = { 
        .x = 1, 
        .y = 2,
        .arr = { {1, 2}, {3, 4}, {5, 6} }
    }
};

/* 6. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[2] = {
    [0] = { .x = 1, .y = {2}, .z = 3 },
    [1] = { .x = 4, .y = 5, .z = 6 }
};

/* 7. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. __typeof__ usage */
__typeof__(*complex_func_ptr) func_type;
__typeof__(multi_dim[0]) row_type;

/* 9. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Point *point_ptr = &(struct Point){ .x = 10, .y = 20, .z = 30 };

/* 10. Function declarations with complex parameters */
void process_array(int (*array)[10][20], int size);
int (*get_callback(void))(int, int);

/* Main function containing various constructs */
int main(void) {
    /* Use function-like macro */
    int x = FOO(5);
    int y = BAR(x, 3);
    
    /* Use complex macro with statement expression */
    int z = COMPLEX_MACRO(x, y);
    
    /* Array access with brackets */
    int val = multi_dim[2][3];
    val += var_size[1][2];
    val += enum_array[3][4];
    
    /* Use compound literal */
    int *arr = (int[][3]){{1, 2, 3}, {4, 5, 6}};
    
    /* Use __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(sizeof(int) == 4, 42, 24);
    
    /* Nested structure access with braces in initializer */
    struct Outer local_struct = {
        .a = chosen,
        .u = { .i = 100 },
        .nested = {
            .x = 1,
            .y = 2,
            .arr = { {7, 8}, {9, 10} }
        }
    };
    
    /* Use typeof in declaration */
    __typeof__(local_struct.nested.arr[0]) row = {11, 12};
    
    /* GCC built-in with parentheses */
    int expected = GCC_SPECIFIC(z > 0);
    
    /* Complex expression with multiple parentheses */
    int result = (FOO(x) + BAR(y, z)) * (chosen / (expected + 1));
    
    /* Prevent dead code elimination */
    volatile int dummy = result;
    
    return dummy > 0 ? 0 : 1;
}

/* 11. Additional complex type at file scope */
union ComplexUnion {
    struct {
        int (*func)(int);
        int array[5][5];
    } s;
    long long data;
} complex_union = {
    .s = {
        .func = 0,
        .array = { {0} }
    }
};

/* 12. Function pointer array with initializer */
int (*callbacks[])(int) = {
    [0] = (int (*)(int))main,
    [1] = 0,
    [2] = 0
};

/* 13. Nested anonymous struct/union */
struct Anonymous {
    union {
        struct {
            int a;
            int b;
        };
        struct {
            long c;
            long d;
        };
    };
    int e;
} anonymous_var = { .a = 1, .b = 2, .e = 3 };
