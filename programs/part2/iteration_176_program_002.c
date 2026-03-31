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
int (*complex_func_ptr)(double);
int (*(*nested_func_ptr)(int))(void);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[5][20];
int arr3[2][3][4];
extern int var_arr[__builtin_constant_p(1) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(16)));
int attr3 __attribute__((format(printf, 1, 2)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
# define ALIGNED __attribute__((aligned(8)))
#else
# define ALIGNED
#endif

/* 6. Struct/union definitions with nested braces */
struct Outer {
    int a;
    union {
        int b;
        double c;
        struct {
            char d[4];
            short e;
        } inner;
    } u;
    int f[3];
};

/* 7. Complex initializer with nested braces */
struct Outer global_var = {
    .a = FOO(5),
    .u = {
        .inner = {
            .d = {'x', 'y', 'z', '\0'},
            .e = BAR(2, 3)
        }
    },
    .f = {[0] = 1, [2] = 3}
};

/* 8. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = {.x = 1, .y = 2, .z = 3},
    [1] = {.x = 4, .y = 5, .z = {6}},
    [2] = {7, 8, 9}
};

/* 9. Compound literal */
int *compound_lit = (int[]){1, 2, 3, 4, 5};

/* 10. __typeof__ usage */
__typeof__(*compound_lit) type_var;

/* Main function containing various constructs */
int main(void) {
    /* Function pointer usage */
    int (*local_func_ptr)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int size = 10;
    int dyn_arr[size];
    
    /* Nested initializer */
    struct Outer local_var = {
        .a = 100,
        .u = { .b = 200 },
        .f = {0}
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){10, 20, 30})[i];
    }
    
    /* __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        __builtin_constant_p(sum),
        sum * 2,
        sum / 2
    );
    
    /* Lambda-like expression using statement expression */
    int result = COMPLEX_MACRO(1, 2, 3);
    
    /* Multi-dimensional array access with brackets */
    int matrix[2][2] = {{1, 2}, {3, 4}};
    int elem = matrix[0][1] + matrix[1][0];
    
    /* Pointer to array */
    int (*ptr_to_arr)[2] = &matrix[0];
    
    /* Return statement with expression in parentheses */
    return (result + chosen + elem + global_var.a + local_var.a);
}

/* 11. Additional constructs at file scope */
enum {
    ARRAY_SIZE = 100
};

int sized_array[ARRAY_SIZE];

/* 12. Function declaration with complex return type */
int (*(*make_func_ptr(void))[5])(void);

/* 13. Nested struct with bitfield */
struct WithBitfield {
    unsigned int a : 4;
    unsigned int b : 4;
    struct {
        unsigned int c : 8;
        unsigned int d : 16;
    } nested;
};

/* 14. Initializer with nested braces and designators */
struct WithBitfield bf = {
    .a = 0xF,
    .b = 0xA,
    .nested = {
        .c = 0xFF,
        .d = 0xFFFF
    }
};

/* 15. Array of structs with nested initializers */
struct WithBitfield bf_array[2] = {
    [0] = { .a = 1, .b = 2, .nested = { .c = 3, .d = 4 } },
    [1] = { 5, 6, { 7, 8 } }
};
