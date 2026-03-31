/* test_gengtype_coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
int (*(*more_complex)[5])(void);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[5][20];
int arr3[][3] = {{1,2,3}, {4,5,6}};

/* 4. Variable-length array style (using enum) */
enum { SIZE = 20 };
int var_arr[SIZE];

/* 5. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((format(printf, 1, 2)));

/* 6. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_var;

/* 7. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int a;
            int b[3];
        } s;
        double d;
        void *p;
    } u;
};

/* Global instance with nested brace initializers */
struct Outer global = {
    .type = 1,
    .u = {
        .s = {
            .a = 42,
            .b = {1, 2, {3}}
        }
    }
};

/* 8. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = {.x = 1, .y = {2}, .z = 3},
    [1] = {.x = 4, .y = 5, .z = 6},
    {7, 8, 9}
};

/* 9. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
    #define GCC_VERSION 0
#endif

/* 10. __typeof__ usage */
__typeof__(*points) point_copy;

/* 11. Compound literal */
int *compound_lit = (int[]){10, 20, 30, {40}};

/* 12. Nested compound literal */
struct Point *ptr = &(struct Point){{.x = 100}, .y = 200, .z = 300};

/* Main function containing various constructs */
int main(void) {
    /* Use function-like macro */
    int a = FOO(5);
    int b = BAR(a, 2);
    int c = NESTED(b);
    
    /* Use complex function pointer type */
    double (*func_array[3])(int) = {NULL, NULL, NULL};
    
    /* Multi-dimensional array access with brackets */
    arr2[2][3] = arr3[1][2];
    
    /* GCC built-in with parentheses */
    int choice = __builtin_choose_expr(sizeof(int) == 4, 42, 24);
    
    /* Ternary operator in array size (more brackets) */
    int dyn_arr[__builtin_constant_p(1) ? 10 : 20];
    
    /* Nested initializers in local scope */
    struct Outer local = {
        .type = 2,
        .u = {
            .d = 3.14159
        }
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1, 2, 3})[0] + ((int[]){4, 5, 6})[1];
    
    /* __typeof__ in local scope */
    __typeof__(arr1) arr_copy;
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = 0;
    
    /* Alignas on local */
    _Alignas(8) char buffer[64];
    
    /* Nested braces in initializer */
    struct {
        int a;
        struct {
            int b;
            int c[2];
        } inner;
    } nested_struct = {1, {2, {3, 4}}};
    
    /* Return computed value to prevent dead code elimination */
    return a + b + c + choice + sum + global.u.s.a + points[0].x 
           + nested_struct.inner.c[0] + arr2[0][0];
}

/* 13. Additional constructs at file scope to ensure parsing continues */

/* Function with complex parameter list */
void complex_params(int a, int b[][5], void (*callback)(int, int)) {
    /* Nested block with braces */
    {
        int local = 0;
        callback(a, local);
    }
}

/* Union with array */
union Data {
    int i;
    float f;
    char str[20];
};

/* Initialized union */
union Data data = {.str = "Hello"};

/* 14. More bracket combinations */
int (*matrix_ptr)[10][20];
int (*(*func_ptr_array[5]))(void);

/* 15. Final struct with all types of braces/parentheses */
struct Final {
    int (*func)(int);
    int array[5];
    struct {
        int x;
    } nested;
} final_instance = {
    .func = NULL,
    .array = {1,2,3,4,5},
    .nested = {.x = 42}
};
