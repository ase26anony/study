/* test-gengtype-coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{'.
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
int dyn_array[const_size];
int attr_array[__builtin_constant_p(1) ? 10 : 20] __attribute__((aligned(16)));

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(32)));
int packed_struct __attribute__((packed));
int section_var __attribute__((section(".data")));

/* 5. Struct/union definitions with nested braces */
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

/* 6. Complex initializers with nested braces */
struct Outer global_struct = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = 4
};

/* 7. Compound literals */
int *compound_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer *struct_literal = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .b = 40
};

/* 8. Preprocessor conditionals with balanced characters */
#ifdef TEST_MACRO
    #define CONDITIONAL(x) ((x) * 2)
    int conditional_array[CONDITIONAL(5)];
#else
    #define CONDITIONAL(x) ((x) + 1)
#endif

/* 9. __typeof__ usage */
__typeof__(*compound_ptr) typed_val;
__typeof__(global_struct.inner.u) typed_union;

/* 10. GCC builtins with parentheses */
int chosen = __builtin_choose_expr(1, 100, 200);
long long builtin_ll = __builtin_expect(ADD(5, 3), 1);

/* Main function with mixed constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int local_array[MAX(3, 4)][SQUARE(2)];
    
    /* Nested initializer */
    struct Outer local_struct = {
        .a = ADD(1, 2),
        .inner = {
            .x = MAX(5, 10),
            .u = { .i = SQUARE(3) }
        },
        .b = CONDITIONAL(7)
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1, 2, 3})[0] + ((int[]){4, 5, 6})[1];
    
    /* __typeof__ with parentheses */
    __typeof__(local_struct.inner.u.i) local_typed = 42;
    
    /* Complex expression with all bracket types */
    int result = ADD(
        multi_dim[0][1][2],
        MAX(
            var_size[SIZE - 1],
            attr_array[__builtin_constant_p(2) ? 0 : 1]
        )
    ) + chosen;
    
    /* Prevent dead code elimination */
    if (result > 0) {
        return result;
    }
    
    return 0;
}

/* 11. Additional edge cases */
/* Function with array parameter */
void process_array(int arr[static 10]) {
    /* Empty function still contains braces */
}

/* Nested struct with bitfield */
struct BitfieldStruct {
    unsigned int a : 4;
    unsigned int b : 4;
    struct {
        unsigned int c : 8;
        unsigned int d : 16;
    } nested;
};

/* Initializer with designators and nested braces */
struct BitfieldStruct bfs = {
    .a = 1,
    .b = 2,
    .nested = { .c = 3, .d = 4 }
};

/* 12. Macro expansion with all bracket types */
#define COMPLEX_MACRO(a, b, c) \
    do { \
        int arr[(a)]; \
        struct Outer s = { .a = (b), .b = (c) }; \
        (void)arr; \
        (void)s; \
    } while(0)

/* Use the macro */
void use_macro(void) {
    COMPLEX_MACRO(5, 10, 15);
}
