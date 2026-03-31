/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352 handling
 * parentheses, brackets, and braces.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) ({ \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    (_a + _b) * (c); \
})

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_arr[FOO(5)][BAR(2, 3)];
extern int incomplete_arr[];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

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
    int arr[3];
};

/* 6. Complex initializers with nested braces */
struct Outer global_outer = {
    .a = FOO(1),
    .inner = {
        .x = 2,
        .u = { .f = 3.14f }
    },
    .arr = { [0] = 1, [1] = 2, [2] = 3 }
};

/* 7. Compound literals */
int *compound_ptr = (int[]){1, 2, 3, 4};
struct Outer *outer_ptr = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .arr = { 100, 200, 300 }
};

/* 8. Preprocessor conditionals */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) ((x) << 2)
    int conditional_var[CONDITIONAL_MACRO(4)];
#else
    #define CONDITIONAL_MACRO(x) ((x) >> 1)
#endif

/* 9. __typeof__ usage */
__typeof__(*compound_ptr) typed_val;
__typeof__(global_outer.inner.u) typed_union;

/* 10. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(
    __builtin_constant_p(1), 
    100, 
    200
);

/* Main function containing multiple triggers */
int main(void) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))FOO;
    
    /* Array with computed size */
    int local_arr[sizeof(struct Outer) / sizeof(int)];
    
    /* Nested initializer in function scope */
    struct Outer local_outer = {
        .a = builtin_result,
        .inner = { 
            .x = BAR(3, 4),
            .u = { .i = COMPLEX_MACRO(1, 2, 3) }
        },
        .arr = { 0 }
    };
    
    /* Compound literal in expression */
    int sum = ((int[]){1, 2, 3})[0] + 
              ((int[]){4, 5, 6})[1] + 
              local_outer.arr[2];
    
    /* __typeof__ in function */
    __typeof__(sum) sum_copy = sum;
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = sum_copy;
    
    /* Lambda-like expression using statement expression */
    int result = ({
        int temp = local_attr;
        temp += FOO(temp);
        temp;
    });
    
    return result > 0 ? 0 : 1;
}

/* 11. Additional C++-like constructs (if parsed as C++) */
#if defined(__cplusplus) || defined(TEST_CPP)
    /* Template-like macro */
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
    
    /* Namespace-like prefix */
    struct Namespaced {
        struct Nested {
            int value;
        } nested;
    };
    
    /* More complex initializer */
    struct Namespaced ns = {
        .nested = { .value = MAX(10, 20) }
    };
#endif

/* 12. Enum with last comma (triggers different parsing) */
enum TestEnum {
    ENUM_A,
    ENUM_B,
    ENUM_C,  /* trailing comma */
};

/* 13. Zero-length array at end of struct */
struct FlexArray {
    int count;
    int data[];  /* flexible array member */
};

/* 14. Alignas specifier (C11/C++) */
_Alignas(32) char aligned_buffer[64];

/* 15. Nested parentheses in expressions */
int nested_parens = ((((1 + 2) * 3) - 4) / 5);

/* 16. Designated initializers with array indices */
int designated_array[10] = { [0] = 1, [5] = 2, [9] = 3 };

/* 17. Union with anonymous struct */
union AnonymousUnion {
    struct {
        int a, b;
    };
    long long ll;
};

/* 18. Bitfield declarations */
struct WithBitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int : 4;  /* unnamed bitfield */
    signed int value : 8;
};
