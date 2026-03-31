/* test-gengtype-balanced.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a + _b; })

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
int (*(*nested_func_ptr)(int))(void);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[10][20];
int arr3[FOO(5)][BAR(2, 3)];

/* 4. Variable length array (C99) */
int var_arr[__builtin_constant_p(1) ? 10 : 20];

/* 5. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((format(printf, 1, 2)));

/* 6. C11 _Alignas specifier */
_Alignas(16) int aligned_var;

/* 7. Struct/union definitions with nested structures */
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

/* 8. Complex initializer with nested braces */
struct Outer global_struct = { 
    .a = FOO(1), 
    .inner = { 
        .x = 2, 
        .u = { .f = 3.14 } 
    }, 
    .b = 4 
};

/* 9. Preprocessor conditionals containing balanced characters */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) [(x) + 1]
    int conditional_array[CONDITIONAL_MACRO(5)];
#else
    #define CONDITIONAL_MACRO(x) ((x) * 2)
#endif

/* 10. __typeof__ usage with parentheses */
__typeof__(*complex_func_ptr) func_return_type;
__typeof__(arr1[0]) array_element_type;

/* 11. Compound literals */
int *p = (int[]){1, 2, 3, 4};
struct Outer *op = &(struct Outer){ 
    .a = 5, 
    .inner = { .x = 6, .u = { .i = 7 } }, 
    .b = 8 
};

/* 12. GCC builtins with complex expressions */
int builtin_result = __builtin_choose_expr(
    __builtin_constant_p(1), 
    sizeof(int[FOO(3)]), 
    sizeof(int[BAR(2, 2)])
);

/* 13. Enum with array size */
enum { ARRAY_SIZE = 10 };
int enum_sized_array[ARRAY_SIZE];

/* 14. Function declaration with array parameter */
void process_array(int matrix[][10], int rows);

/* 15. Main function containing multiple balanced constructs */
int main(void) {
    /* Function pointer usage */
    int result = FOO(10);
    result = BAR(result, 2);
    
    /* Array access with brackets */
    arr1[0] = result;
    arr2[1][2] = arr1[0];
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){10, 20, 30})[i];
    }
    
    /* Nested struct access */
    global_struct.inner.u.i = sum;
    
    /* __typeof__ in variable declaration */
    __typeof__(global_struct) local_struct = {
        .a = 1,
        .inner = { .x = 2, .u = { .i = 3 } },
        .b = 4
    };
    
    /* GCC statement expression */
    int stmt_expr = COMPLEX_MACRO(local_struct.a, local_struct.b);
    
    /* Array with computed size */
    int computed_size_arr[stmt_expr > 0 ? 5 : 10];
    
    /* Prevent dead code elimination */
    if (complex_func_ptr) {}
    if (p) {}
    
    return result + sum + stmt_expr;
}

/* 16. Additional function with complex signature */
int (*(*register_callback(int id, void (*cb)(int, int[]))))(void) {
    static int (*(*registered)(void)) = 0;
    /* Nested block with braces */
    {
        int temp[5] = {1, 2, 3, 4, 5};
        cb(id, temp);
    }
    return registered;
}

/* 17. Union with array member */
union Data {
    int i;
    float f;
    char str[20];
    struct {
        int x;
        int y;
    } point;
};

/* 18. Initializer with designated initializers and nested braces */
union Data data_instance = { 
    .point = { .x = 100, .y = 200 } 
};

/* 19. Macro with nested parentheses */
#define NESTED_PARENS(a) ((((a) + 1) * 2) - 3)
int nested_parens_result = NESTED_PARENS(5);

/* 20. Array with attribute */
int attributed_array[16] __attribute__((aligned(64)));

/* End of file - should have triggered all balanced character cases */
