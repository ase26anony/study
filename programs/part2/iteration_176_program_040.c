/* test-gengtype-coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{'.
 */

/* 1. Function-like macros with parentheses */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SQUARE(x) ((x) * (x))
#define COMPLEX_MACRO(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a + _b; })

/* 2. Complex declarators with parentheses */
int (*func_ptr_with_params)(int, double, char *);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_fp)(void))[10];

/* 3. Array declarations with brackets - multi-dimensional and variable */
int multi_dim_array[5][10][15];
enum { ARRAY_SIZE = 20 };
int sized_array[ARRAY_SIZE];
const int const_size = 30;
int var_array[const_size];
int attr_array[10] __attribute__((aligned(64)));

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(32)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));
int vector_type __attribute__((vector_size(16)));

/* 5. Nested structures with brace initializers */
struct Outer {
    int a;
    struct Inner {
        int x;
        double y;
        union {
            int i;
            float f;
        } u;
    } inner;
    int b;
};

/* Global struct with complex initializer using braces */
struct Outer global_struct = {
    .a = 1,
    .inner = {
        .x = 2,
        .y = 3.14,
        .u = { .f = 2.718 }
    },
    .b = 4
};

/* 6. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
};

union Data global_union = { .str = "Hello" };

/* 7. Preprocessor conditionals containing triggering constructs */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
    int likely_var = GCC_SPECIFIC(1);
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 8. __typeof__ usage with parentheses */
int typeof_var;
__typeof__(typeof_var) typeof_copy;

/* 9. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Point { int x; int y; };
struct Point *point_ptr = &(struct Point){ .x = 10, .y = 20 };

/* 10. Nested initializers with multiple braces */
int nested_array[2][3] = { {1, 2, 3}, {4, 5, 6} };
struct Nested {
    int a[3];
    struct { int b; int c; } inner;
} nested_struct = { 
    .a = {7, 8, 9}, 
    .inner = { .b = 10, .c = 11 } 
};

/* 11. Function declarations with complex return types */
int (*(*make_array(void))[10])() {
    static int (*arr[10])();
    return &arr;
}

/* 12. __builtin_choose_expr with ternary */
int builtin_choice = __builtin_choose_expr(
    sizeof(int) == 4,
    (int[]){1, 2, 3}[0],
    (int[]){4, 5, 6}[0]
);

/* 13. Lambda-like statement expression (GCC extension) */
int statement_expr_result = ({
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){1, 2, 3, 4, 5})[i];
    }
    sum;
});

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_fp)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int computed_size = MAX(5, 10);
    int dynamic_array[computed_size];
    
    /* Nested struct initialization */
    struct Outer local_struct = {
        .a = 100,
        .inner = {
            .x = 200,
            .y = 300.0,
            .u = { .i = 400 }
        },
        .b = 500
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){10, 20, 30})[i];
    }
    
    /* __typeof__ with parentheses */
    __typeof__(*compound_literal_ptr) first_element = compound_literal_ptr[0];
    
    /* GCC builtin with parentheses */
    int is_constant = __builtin_constant_p(sum);
    
    /* Array subscript with nested parentheses */
    int value = nested_array[((int){1})][((int){0})];
    
    /* Prevent dead code elimination */
    if (global_struct.a + global_union.i + sum + first_element + 
        is_constant + value + statement_expr_result + builtin_choice > 0) {
        return 0;
    }
    
    return 1;
}

/* 14. Additional top-level constructs */
/* Function with attribute and array parameter */
void attribute_func(int param[static 10]) __attribute__((nonnull(1)));

/* Array of function pointers */
int (*func_array[5])(void);

/* Complex declarator at file scope */
void (*(*file_scope_fp)(int (*)(double)))(char *);

/* Final struct with deeply nested initializer */
struct DeepNest {
    struct Level1 {
        struct Level2 {
            int data[3];
        } l2;
    } l1;
} deep_nest = {
    .l1 = {
        .l2 = {
            .data = { ((int){1}), ((int){2}), ((int){3}) }
        }
    }
};
