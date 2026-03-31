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
int var_arr[const_size > 10 ? 30 : 40];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(32)));

/* 5. C++-style alignas (C11/C++11) */
_Alignas(16) int aligned_c11;
#ifdef __cplusplus
alignas(32) double aligned_cpp;
#endif

/* 6. Struct/union definitions with nested initializers */
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

/* 7. Complex initializer with nested braces */
struct Outer global_var = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = {4}
};

/* 8. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
} data = { .str = "Hello" };

/* 9. Preprocessor conditionals containing balanced characters */
#ifdef TEST_CASE
    int conditional_var[(SIZE > 5) ? 10 : 20] = {0};
    #define CONDITIONAL_MACRO(x) (((x) + 1) * 2)
#endif

/* 10. __typeof__ usage */
__typeof__(*complex_func_ptr) type_var;
__typeof__(multi_dim[0][0]) elem_type;

/* 11. Compound literals */
int *array_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer *obj_ptr = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .b = 40
};

/* 12. GCC builtins with parentheses */
int chosen = __builtin_choose_expr(1, 100, 200);
int constant_p = __builtin_constant_p(42);
long long swapped = __builtin_bswap64(0x0123456789ABCDEFULL);

/* 13. Lambda-like function pointer initialization (C compatible) */
int (*lambda)(int) = (int (*)(int))NULL;

/* Main function containing various balanced character constructs */
int main(void) {
    /* Function pointer call */
    if (complex_func_ptr) {
        int result = complex_func_ptr(3.14, 2);
    }
    
    /* Array access with multiple brackets */
    int val = multi_dim[1][2][3];
    val += var_arr[5];
    
    /* Nested initializer in local scope */
    struct Outer local = {
        .a = ADD(1, 2),
        .inner = {
            .x = MAX(val, 10),
            .u = { .i = SQUARE(5) }
        },
        .b = sizeof(multi_dim)
    };
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += ((int[]){10, 20, 30, 40, 50})[i];
    }
    
    /* __typeof__ in declaration */
    __typeof__(local.a) copy = local.a;
    
    /* Attribute on local variable */
    int local_aligned __attribute__((aligned(8))) = 42;
    
    /* Complex expression with nested parentheses */
    int complex_expr = (ADD(MAX(local.a, local.b), 
                           SQUARE((int){local.inner.x})) 
                        * (chosen > 50 ? 2 : 3));
    
    /* Return statement with parentheses */
    return (complex_expr > 100 ? 0 : 1);
}

/* 14. Additional struct with bitfield and array */
struct BitfieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    int arr[3];
} bitfield_instance = { .a = 1, .b = 2, .c = 3, .arr = {4, 5, 6} };

/* 15. Function declaration with array parameter */
void process_array(int matrix[][10], int rows);
void process_array(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
        }
    }
}

/* 16. Pointer to array */
int (*ptr_to_array)[10] = &multi_dim[0];

/* 17. Anonymous struct/union (C11) */
struct Anonymous {
    union {
        int as_int;
        float as_float;
    };
    struct {
        int x;
        int y;
    };
} anonymous_instance = { .as_int = 42, .x = 1, .y = 2 };
