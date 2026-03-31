/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define ADD(x, y) ((x) + (y))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal_handler)(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
const int const_arr[] = {1, 2, 3};
enum { SIZE = 5 };
int var_arr[SIZE];
int runtime_arr[sizeof(int) * 2];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(16)));

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
        struct {
            int x;
            int y;
        } point;
        struct {
            int width;
            int height;
        } rect;
    } data;
};

/* Global struct with nested brace initializer */
struct Outer global_struct = {
    .type = 1,
    .data = {
        .point = {
            .x = 10,
            .y = {20}  /* Nested braces */
        }
    }
};

/* 7. Another struct with array member */
struct WithArray {
    int ids[3];
    char name[20];
};

/* 8. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4};

/* 9. __typeof__ usage */
__typeof__(*global_ptr) typed_var;

/* 10. Function declaration with complex return type */
int (*(*make_func(void))[5])(void);

/* Main function containing various constructs */
int main(void) {
    /* 11. Local compound literal */
    int *local_ptr = (int[]){5, 6, 7};
    
    /* 12. Nested struct initialization */
    struct WithArray local_struct = {
        .ids = {8, 9, 10},
        .name = {'t', 'e', 's', 't', '\0'}
    };
    
    /* 13. GCC built-in with parentheses */
    int choice = __builtin_choose_expr(1, 100, 200);
    
    /* 14. Array with computed size */
    int computed[ADD(2, 3)];
    
    /* 15. Complex expression with multiple parentheses */
    int result = MAX(SQUARE(ADD(choice, 1)), 50);
    
    /* 16. Nested attribute in declaration */
    int __attribute__((aligned(8))) aligned_local;
    
    /* 17. Designated initializer with nested braces */
    int matrix[2][2] = {[0] = {1, 2}, [1] = {3, 4}};
    
    /* 18. __typeof__ in local scope */
    __typeof__(matrix[0]) row_copy;
    
    /* 19. Prevent dead code elimination */
    result += global_ptr[0] + local_ptr[1] + local_struct.ids[0];
    
    /* 20. Return statement with expression in parentheses */
    return (result > 0 ? 0 : 1);
}

/* 21. Additional global with all three characters */
struct {
    int (*func)(int[][3]);
    int arr[2][2];
} complex_global = {
    .func = 0,
    .arr = {{1,2},{3,4}}
};

/* 22. Function-like macro with nested parentheses */
#define NESTED_PARENS(a) (((a) + 1) * ((a) - 1))

/* 23. Use of NESTED_PARENS */
int use_nested = NESTED_PARENS(5);

/* 24. Empty struct for edge case */
struct Empty {};

/* 25. Union with array */
union DataUnion {
    int i;
    float f;
    char str[10];
};

/* 26. Pointer to array */
int (*ptr_to_array)[10];

/* 27. Array of pointers to functions */
int (*func_array[5])(void);

/* 28. __attribute__ with multiple parameters */
int multi_attr __attribute__((deprecated, unused));

/* 29. Alignas specifier (C11/C++) */
_Alignas(32) char aligned_char;

/* 30. Static assertion with parentheses */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 31. Inline assembly with braces (GCC extension) */
void dummy_asm(void) {
    __asm__ volatile (
        "nop\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : /* no clobbers */
    );
}
