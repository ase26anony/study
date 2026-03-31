/* test-gengtype-coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing logic in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(a, b, c) (a + (b * c))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[FOO(5)][20];
int arr3[][3] = {{1,2,3}, {4,5,6}};

/* 4. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(16)));
int attr3 __attribute__((deprecated("use something else")));

/* 5. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_var;

/* 6. Struct/union definitions with nested initializers */
struct Outer {
    int a;
    union {
        int b;
        double c;
    } u;
    struct {
        int nested_arr[3];
    } s;
};

/* 7. Complex initializer with nested braces */
struct Outer global_var = {
    .a = FOO(1),
    .u = { .c = 3.14 },
    .s = { .nested_arr = {1, 2, BAR(3, 4)} }
};

/* 8. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_constant_p(x)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 9. __typeof__ usage */
__typeof__(*complex_func_ptr) type_var;

/* 10. Variable length array (C99) */
void vla_example(int n) {
    int vla[n];
    (void)vla;
}

/* 11. Compound literals */
int *compound_lit = (int[]){1, 2, 3, 4};

/* 12. Designated initializers with nested braces */
struct Nested {
    struct {
        int x[2];
        int y;
    } inner;
} nested_var = { .inner = { .x = {5, 6}, .y = 7 } };

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_fp)(int) = (int (*)(int))FOO;
    
    /* Multi-dimensional array with initialization */
    int matrix[2][3] = {{1,2,3}, {4,5,6}};
    
    /* Nested struct initialization */
    struct Outer local_var = {
        .a = 10,
        .u = { .b = 20 },
        .s = { .nested_arr = {30, 40, 50} }
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1,2,3})[0] + ((int[]){4,5,6})[1];
    
    /* __builtin_choose_expr with ternary */
    int chosen = __builtin_choose_expr(
        __builtin_constant_p(sum),
        sum * 2,
        sum / 2
    );
    
    /* Array with computed size */
    int computed_arr[GCC_SPECIFIC(1) ? 10 : 20];
    
    /* Lambda-like function definition (GCC extension) */
    int result = ({
        int temp = local_var.a;
        temp += nested_var.inner.y;
        temp;
    });
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", result + chosen + sum);
    
    /* Return statement with expression in parentheses */
    return (result > 0 ? 0 : 1);
}

/* 13. Additional global scope triggers */
enum { SIZE = 100 };
int enum_array[SIZE];

/* 14. Pointer to array */
int (*ptr_to_array)[10] = &arr1;

/* 15. Function returning array pointer (invalid in C but parsed) */
// int (*returns_array(void))[10];  // This would trigger more parsing

/* 16. __attribute__ with nested parentheses */
void __attribute__((constructor(101))) init_func(void) {
    /* Empty but triggers attribute parsing */
}

/* 17. Asm statement with braces (GCC extension) */
void asm_example(void) {
    __asm__ volatile (
        "nop\n\t"
        : /* no outputs */
        : /* no inputs */
        : "memory"
    );
}

/* 18. Statement expression with nested braces (GCC extension) */
int stmt_expr(int x) {
    return ({
        int y = x * 2;
        int z;
        for (z = 0; z < y; z++) {
            /* loop body */
        }
        y;
    });
}
