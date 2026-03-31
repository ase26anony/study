/* test-gengtype-coverage.c
 * This file is designed to trigger balanced character parsing in gengtype.
 * It should be parsed by gengtype, not compiled normally.
 */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[3][4][5];
enum { SIZE = 10 };
int var_size[SIZE];
const int const_size = 20;
int dynamic_like[const_size + SIZE];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(16)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
#  define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
#  define GCC_SPECIFIC(x) (x)
#endif

/* 6. Structures with nested unions and initializers */
struct Outer {
    int type;
    union {
        int ival;
        double dval;
        char *sval;
    } data;
    struct {
        int x;
        int y;
    } coord;
};

/* Complex initializer with nested braces */
struct Outer global_var = {
    .type = 1,
    .data = { .ival = 42 },
    .coord = { .x = 10, .y = {20} }
};

/* 7. Another struct with designated initializers */
struct Nested {
    int a;
    struct {
        int b[3];
        int c;
    } inner;
};

struct Nested nested_var = {
    .a = 1,
    .inner = {
        .b = {1, 2, 3},
        .c = 4
    }
};

/* 8. __typeof__ usage */
__typeof__(*complex_func_ptr) func_type;
__typeof__(multi_dim[0]) first_dim;

/* 9. Compound literals */
int *array_ptr = (int[]){1, 2, 3, 4};
struct Outer *obj_ptr = &(struct Outer){
    .type = 2,
    .data = { .dval = 3.14 },
    .coord = { .x = 5, .y = 6 }
};

/* 10. Main function with mixed constructs */
int main(void) {
    /* Use function-like macro */
    int sum = ADD(5, 3);
    
    /* Use complex function pointer type */
    int (*local_func)(double, int) = 0;
    
    /* Multi-dimensional array access */
    int val = multi_dim[1][2][3];
    
    /* Array with computed index */
    int idx = var_size[SIZE - 1];
    
    /* GCC built-in with parentheses */
    int chosen = __builtin_choose_expr(1, 10, 20);
    
    /* Compound literal in expression */
    int *local_ptr = (int[]){sum, val, chosen};
    
    /* Nested structure initialization */
    struct Nested local_nested = {
        .a = 100,
        .inner = {
            .b = {local_ptr[0], local_ptr[1], local_ptr[2]},
            .c = 200
        }
    };
    
    /* __typeof__ in declaration */
    __typeof__(local_nested.inner.b) local_array = {0};
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 99;
    
    /* Use all variables to avoid dead code elimination */
    return (sum + val + chosen + local_nested.a + local_aligned) % 256;
}

/* 11. Additional constructs at file scope */
/* Function with array parameter */
void process_array(int arr[static 10]) {
    /* Empty function, just for declaration */
}

/* Pointer to array */
int (*array_pointer)[SIZE];

/* Anonymous struct in union */
union Mixed {
    struct {
        int a;
        int b;
    };
    double d;
};

/* 12. More complex initializers */
struct {
    int a;
    struct {
        int b[2][2];
    } inner;
} complex_init = {
    .a = 1,
    .inner = {
        .b = {{1, 2}, {3, 4}}
    }
};

/* 13. Macro with nested parentheses */
#define NESTED(x) ((((x) + 1) * 2) - 3)
int macro_result = NESTED(5);

/* 14. Array with attribute */
int special_array[16] __attribute__((aligned(64)));

/* 15. Final mixed declaration using everything */
#ifdef TEST_MODE
__typeof__(complex_init.inner.b) final_array __attribute__((aligned(32))) = {{0}};
#else
__typeof__(complex_init.inner.b) final_array = {{1, 2}, {3, 4}};
#endif
