/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
enum { SIZE = 15 };
int var_size[SIZE];
const int const_size = 25;
int var_arr[const_size > 20 ? 30 : 10];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. C++-like alignas (C11/C++11) */
#ifdef __cplusplus
alignas(32) double aligned_double;
#else
_Alignas(32) double aligned_double;
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
    int arr[3][2];
};

/* Global instance with nested brace initializer */
struct Outer global_var = {
    .type = 1,
    .data = {
        .point = { .x = 10, .y = {20} }
    },
    .arr = { {1, 2}, {3, 4}, {5, 6} }
};

/* 7. Another struct with designated initializers */
struct Inner {
    int a;
    int b;
    int c[2];
};

/* 8. Union with array */
union DataUnion {
    int i;
    float f;
    char str[20];
};

/* 9. Preprocessor conditional with balanced characters */
#ifdef TEST_CASE
    #define SPECIAL(x) ({ typeof(x) _x = (x); _x * 2; })
    int special_var = SPECIAL(5);
#else
    #define SPECIAL(x) ((x) * 3)
#endif

/* 10. Function using all constructs */
int process_data(struct Outer *ptr, int (*callback)(int)) {
    /* Compound literal with braces */
    int *dynamic = (int[]){1, 2, 3, 4, 5};
    
    /* __typeof__ with parentheses */
    __typeof__(*dynamic) val = 42;
    
    /* GCC built-in with parentheses */
    int chosen = __builtin_choose_expr(
        val > 0,
        FOO(val),
        BAR(val, 2)
    );
    
    /* Nested array access with brackets */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int elem = matrix[1][2];
    
    /* Function pointer call with parentheses */
    if (callback) {
        val = callback(val);
    }
    
    /* Return expression with multiple parentheses */
    return (chosen + (elem * NESTED(val)));
}

/* 11. Main function as container */
int main(void) {
    /* Initialize with compound literal */
    struct Inner local = {
        .a = 1,
        .b = 2,
        .c = {3, 4}
    };
    
    /* Array with computed size */
    int computed_size[FOO(local.a) + BAR(local.b, 2)];
    
    /* Use __builtin_constant_p in array size */
    int builtin_arr[__builtin_constant_p(1) ? 10 : 20];
    
    /* Nested struct initialization */
    struct Outer instance = {
        2,
        { .rect = { 100, 200 } },
        { {7, 8}, {9, 10}, {11, 12} }
    };
    
    /* Lambda-like function pointer (C only) */
    int (*lambda)(int) = (int (*)(int)){ 
        [](int x) { return x * x; }  /* C++ style, will be ignored in C */
    };
    
    /* Process and return result */
    int result = process_data(&instance, 0);
    
    /* Prevent dead code elimination */
    return result + global_var.type + local.a + computed_size[0] + builtin_arr[0];
}

/* 12. Additional edge cases at file scope */
#if 0
    /* This block won't be compiled but will be parsed */
    int disabled_array[({ int y = 5; y; })];
#endif

/* 13. Macro with nested parentheses and brackets */
#define COMPLEX_MACRO(a, b) \
    do { \
        int _arr[(a) + (b)]; \
        for (int i = 0; i < (a) + (b); i++) { \
            _arr[i] = (i * (a)) + (b); \
        } \
    } while (0)

/* 14. Struct with flexible array member */
struct FlexArray {
    int count;
    int data[];  /* GCC extension */
};

/* 15. Using the complex macro */
void use_macro(void) {
    COMPLEX_MACRO(3, 4);
}

/* 16. Attribute on struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* 17. Inline assembly with braces (GCC extension) */
void asm_example(void) {
    __asm__ volatile (
        "mov %0, %%eax\n"
        "add $1, %%eax\n"
        : /* outputs */
        : /* inputs */
        : /* clobbers */
    );
}

/* 18. Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
v4si vec = {1, 2, 3, 4};

/* 19. Transparent union attribute */
union __attribute__((transparent_union)) TransUnion {
    int i;
    long l;
};

/* 20. Final struct with everything combined */
struct Ultimate {
    int (*funcs[5])(struct Outer *);
    union DataUnion data[3];
    int matrix[2][2];
    struct FlexArray *flex;
} __attribute__((aligned(64)));
