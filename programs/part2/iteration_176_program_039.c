/* test_gengtype_coverage.c
 * This file is designed to trigger the balanced character parsing
 * in gengtype-parse.cc lines 341-352.
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
int arr3[2][3][4];

/* 4. Arrays with non-constant size using enum */
enum { SIZE = 100 };
int var_arr[SIZE];
int var_arr2[FOO(10)][BAR(2, 3)];

/* 5. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(16)));
int attr3 __attribute__((aligned(32))) = 0;

/* 6. C11 _Alignas specifier */
_Alignas(16) int aligned1;
_Alignas(double) int aligned2;

/* 7. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        int i;
        double d;
        struct {
            char c;
            short s;
        } inner;
    } data;
    int arr[3];
};

/* Global instance with nested brace initializer */
struct Outer global = {
    .type = 1,
    .data = {
        .d = 3.14159,
        .inner = {
            .c = 'A',
            .s = 42
        }
    },
    .arr = {1, 2, {3}}
};

/* 8. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z[2];
};

/* 9. Union with initializer */
union Mixed {
    int i;
    float f;
    char str[10];
};

/* 10. Preprocessor conditional with constructs */
#ifdef TEST_DEFINE
    #define CONDITIONAL(x) ((x) + 100)
    int conditional_arr[CONDITIONAL(5)];
#else
    #define CONDITIONAL(x) ((x) + 200)
#endif

/* 11. __typeof__ usage */
int typeof_var = 0;
__typeof__(typeof_var) typeof_copy;
__typeof__(*complex_func_ptr) typeof_func_result;

/* 12. Compound literals */
struct Point* get_point(void) {
    return &(struct Point){.x = 10, .y = 20, .z = {30, 40}};
}

/* 13. GCC builtins with parentheses */
int builtin_test = __builtin_choose_expr(1, 100, 200);
int builtin_cond = __builtin_constant_p(42) ? 1 : 0;

/* 14. Function with all constructs */
int process_data(int (*callback)(int, int), int matrix[][5]) {
    /* Nested parentheses in expression */
    int result = FOO(BAR(matrix[0][0], callback(1, 2)));
    
    /* Array access with brackets */
    result += matrix[1][2] + matrix[2][3];
    
    /* Compound literal */
    int *dynamic = (int[]){1, 2, 3, 4, 5};
    
    /* Nested braces in initializer */
    struct Point local = {
        .x = result,
        .y = dynamic[2],
        .z = {builtin_test, builtin_cond}
    };
    
    /* __typeof__ in local scope */
    __typeof__(local.x) x_copy = local.x;
    
    return x_copy + NESTED(result);
}

/* 15. Main function with mixed constructs */
int main(void) {
    /* Initialize function pointer */
    extern int sample_func(double);
    complex_func_ptr = sample_func;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        arr1[i] = i * 2;
        for (int j = 0; j < 20; j++) {
            arr2[i/2][j] = i + j;
        }
    }
    
    /* Use compound literal */
    int *ptr = (int[FOO(3)]){1, 2, 3, 4};
    
    /* Nested attribute */
    int __attribute__((aligned(8))) aligned_var = 42;
    
    /* Complex expression with parentheses */
    int value = (FOO(5) + BAR(3, 4)) * (NESTED(2) - 1);
    
    /* Array with computed size */
    int computed_arr[value > 0 ? 10 : 5];
    
    /* Nested struct initializer in compound literal */
    struct Outer temp = {
        .type = 2,
        .data = {.i = 100},
        .arr = {[1] = 50, [0] = 25, [2] = 75}
    };
    
    /* Use __typeof__ with pointer */
    __typeof__(&temp) temp_ptr = &temp;
    
    /* Call function with complex arguments */
    int matrix[3][5] = {{1,2,3,4,5}, {6,7,8,9,10}, {11,12,13,14,15}};
    int final_result = process_data(0, matrix);
    
    /* Prevent dead code elimination */
    return final_result + global.data.i + aligned_var + ptr[0] + computed_arr[0];
}

/* 16. Additional function with deeply nested parentheses */
int deeply_nested(void) {
    return ((((1 + 2) * (3 - 4)) / ((5 % 6) + 7)) - ((8 << 1) >> 2));
}

/* 17. K&R style function definition (uses parentheses differently) */
int old_style(a, b)
    int a;
    int b[];
{
    return a + b[0];
}

/* 18. Variable Length Array (VLA) */
void vla_example(int n) {
    int vla[n];
    int vla2d[n][n+1];
    
    for (int i = 0; i < n; i++) {
        vla[i] = i * i;
        for (int j = 0; j < n+1; j++) {
            vla2d[i][j] = i + j;
        }
    }
}

/* 19. __attribute__ with constructor/destructor */
void init_func(void) __attribute__((constructor));
void cleanup_func(void) __attribute__((destructor));

void init_func(void) {
    /* Empty but triggers attribute parsing */
}

void cleanup_func(void) {
    /* Empty but triggers attribute parsing */
}

/* 20. Asm statement with braces */
void asm_example(void) {
    int src = 10, dst;
    
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (dst)
        : "r" (src)
        : "%eax"
    );
}
