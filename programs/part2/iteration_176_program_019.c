/* test-gengtype-coverage.c - Test file for gengtype balanced character parsing */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(a, b) ((a) * (b))
#define COMPLEX_MACRO(x) ({ typeof(x) _x = (x); _x * _x; })

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))(float);

/* 3. Array declarations with brackets - multi-dimensional */
int multi_array[10][20];
int variable_array[FOO(5)][BAR(2, 3)];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(16)));
int section_var __attribute__((section(".data")));

/* 5. Struct with nested union and complex initializer */
struct Outer {
    int a;
    union {
        int x;
        double y;
        struct {
            char c;
            short s;
        } nested;
    } u;
    int *ptr_arr[5];
};

/* Global struct with nested brace initializer */
struct Outer global_struct = {
    .a = FOO(10),
    .u = { .nested = { .c = 'A', .s = 42 } },
    .ptr_arr = { NULL, NULL, NULL, NULL, NULL }
};

/* 6. Another struct with designated initializers and nested braces */
struct Inner {
    int values[3];
    struct {
        float f;
        double d;
    } inner;
};

/* 7. Preprocessor conditional with balanced characters */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) [x]
    int conditional_array CONDITIONAL_MACRO(10);
#else
    #define CONDITIONAL_MACRO(x) (x * 2)
    int conditional_value = CONDITIONAL_MACRO(20);
#endif

/* 8. __typeof__ usage with parentheses */
int typeof_var;
__typeof__(typeof_var) typeof_copy;
__typeof__(*complex_func_ptr) return_type;

/* 9. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 10. Function declaration with array parameter */
void process_array(int arr[][10], int (*callback)(int));

/* Main function containing various balanced character constructs */
int main(void) {
    /* 11. Local struct with nested brace initializer */
    struct Inner local = {
        .values = {1, 2, 3},
        .inner = { .f = 3.14f, .d = 2.71828 }
    };
    
    /* 12. Compound literal inside function */
    int *dynamic_array = (int[FOO(3)]){10, 20, 30};
    
    /* 13. GCC built-in with parentheses */
    int builtin_result = __builtin_choose_expr(
        sizeof(int) == 4,
        FOO(42),
        BAR(10, 20)
    );
    
    /* 14. Nested parentheses in expression */
    int nested_expr = ((((1 + 2) * 3) - 4) / 5);
    
    /* 15. Array access with brackets */
    int array_access = multi_array[0][0] + variable_array[1][2];
    
    /* 16. Function pointer call with parentheses */
    if (complex_func_ptr) {
        double result = (*complex_func_ptr)(3.14159);
    }
    
    /* 17. __alignof__ with parentheses */
    size_t alignment = __alignof__(struct Outer);
    
    /* 18. C++ style alignas (C11/C++11) */
    _Alignas(16) char aligned_buffer[64];
    
    /* 19. Statement expression with braces (GCC extension) */
    int stmt_expr = ({
        int temp = 0;
        for (int i = 0; i < 10; i++) {
            temp += i;
        }
        temp;
    });
    
    /* 20. Nested initializer with multiple braces */
    struct {
        int a[2][2];
        struct {
            int x;
            int y;
        } point;
    } nested_init = {
        .a = {{1, 2}, {3, 4}},
        .point = { .x = 10, .y = 20 }
    };
    
    /* 21. Attribute on variable inside function */
    int local_attr __attribute__((unused)) = 0;
    
    /* 22. Complex array declaration with enum size */
    enum { SIZE = 100 };
    int enum_sized_array[SIZE][SIZE/2];
    
    /* 23. Pointer to array */
    int (*ptr_to_array)[10] = &multi_array[0];
    
    /* 24. Use all variables to avoid dead code elimination */
    return (int)(
        builtin_result +
        nested_expr +
        array_access +
        alignment +
        stmt_expr +
        nested_init.a[0][0] +
        local.values[0] +
        dynamic_array[0] +
        global_ptr[0]
    );
}

/* 25. Additional function with complex prototype */
int (*(*extra_complex)(int (*)(double), int[][10]))(void) {
    return 0;
}

/* 26. Union with array and nested struct */
union ComplexUnion {
    struct {
        int tag;
        union {
            int i;
            float f;
            char str[50];
        } data;
    } s;
    long long raw[10];
};

/* 27. Zero-length array at end of struct (GCC extension) */
struct FlexArray {
    int count;
    int data[];
};

/* 28. __builtin_constant_p in array size */
int builtin_array[__builtin_constant_p(1) ? 10 : 20];

/* 29. Multiple levels of parentheses in declarator */
int (*(*(*insane_ptr)(void))[10])(float, double);

/* 30. Final check - ensure file ends with balanced constructs */
static struct {
    int final_check;
} final_struct = { .final_check = 42 };
