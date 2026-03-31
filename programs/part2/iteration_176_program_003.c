/* test-gengtype-coverage.c
 * This file is designed to trigger the balanced character parsing
 * in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define COMPLEX_MACRO(x) (sizeof((x)[0]) + (int)(x))

/* 2. Complex declarators with parentheses */
int (*global_func_ptr)(double, int);
void (*signal_handler)(int (*)(void));
int (*(*nested_func_ptr)[5])(float);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
const int const_array[] = {1, 2, 3};
enum { SIZE = 15 };
int var_array[SIZE + 5];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
char packed_struct __attribute__((packed));
int vector_type __attribute__((vector_size(16)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
# define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
# define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        int ival;
        double dval;
        int *pval;
    } data;
    struct {
        int x;
        int y;
    } point;
};

/* Global struct with nested brace initializer */
struct Outer global_outer = {
    .type = 1,
    .data = { .dval = 3.14159 },
    .point = { .x = 10, .y = {20} }
};

/* 7. Another struct with designated initializers */
struct Nested {
    int a;
    int b[3];
    struct Outer inner;
};

struct Nested nested_var = {
    .a = 5,
    .b = { [1] = 10, [0] = 5, [2] = {15} },
    .inner = { 
        .type = 2, 
        .data = { .ival = 42 },
        .point = { 7, 8 }
    }
};

/* 8. __typeof__ usage */
__typeof__(*global_func_ptr) func_type_var;

/* 9. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4};

/* Main function containing various constructs */
int main(void) {
    /* Function pointer declaration with parentheses */
    int (*local_func)(int) = 0;
    
    /* Array with computed size */
    int dynamic_size[ADD(5, 3)];
    
    /* Compound literal */
    int *local_ptr = (int[]){10, 20, 30};
    
    /* Nested initializer with braces */
    struct Outer local_outer = {
        .type = 3,
        .data = { .pval = local_ptr },
        .point = { .x = 100, .y = 200 }
    };
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        sizeof(int) == 4,
        42,
        24
    );
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int result = ({
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            sum += local_ptr[i];
        }
        sum;
    });
    
    /* Multi-dimensional array access with brackets */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int elem = matrix[1][2];
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 123;
    
    /* Use all variables to avoid dead code elimination */
    return ADD(choice, result) + elem + local_aligned + global_outer.type + nested_var.a;
}

/* 10. Additional complex type at file scope */
typedef int (*callback_t)(int (*)(void), int[][5]);

/* 11. Union with anonymous struct */
union Mixed {
    struct {
        int a;
        int b;
    };
    double d;
};

/* 12. Zero-length array at end of struct */
struct WithFlex {
    int count;
    int data[];
};

/* 13. Alignas specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[64];

/* 14. Nested switch-case in macro expansion */
#define HANDLE_CASE(x) \
    switch (x) {       \
        case 1: break; \
        case 2: break; \
        default: break; \
    }

/* 15. __attribute__ with nested parentheses */
void __attribute__((constructor(101))) init_func(void) {
    /* Empty constructor */
}

/* 16. Array with attribute */
int special_array[10] __attribute__((deprecated));
