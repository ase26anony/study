/* test-gengtype-coverage.c
 * This file is designed to trigger the balanced character parsing
 * logic in gengtype-parse.cc, specifically lines 341-352.
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
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*nested_func_ptr)(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_arr[FOO(5)][BAR(2, 3)];
extern int incomplete_array[];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int section_var __attribute__((section(".data")));

/* 5. Preprocessor conditionals containing balanced characters */
#ifdef TEST_CONDITIONAL
    #define CONDITIONAL_MACRO(x) [(x) + 1]
    int conditional_array CONDITIONAL_MACRO(10);
#else
    #define CONDITIONAL_MACRO(x) ((x) * 2)
#endif

/* 6. Struct/union definitions with nested initializers */
struct Outer {
    int a;
    double b;
    union Inner {
        int x;
        float y[3];
    } inner;
    struct Nested {
        char *name;
        int values[5];
    } nested;
};

/* 7. Complex initializer with braces */
struct Outer global_struct = {
    .a = FOO(1),
    .b = 3.14,
    .inner = {
        .y = {1.0f, 2.0f, 3.0f}
    },
    .nested = {
        .name = "test",
        .values = {1, 2, 3, 4, 5}
    }
};

/* 8. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 9. Enum with array size */
enum {
    ARRAY_SIZE = 10,
    ANOTHER_SIZE = ARRAY_SIZE * 2
};

/* 10. Function declaration with __attribute__ and complex return */
static __attribute__((always_inline)) 
int inline_func(int x) __attribute__((const));

/* Main function containing various balanced character constructs */
int main(void) {
    /* 11. Local array with computed size */
    int local_array[FOO(ARRAY_SIZE)][BAR(2, 3)];
    
    /* 12. Compound literal with nested braces */
    struct Outer *local_ptr = &(struct Outer){
        .a = 42,
        .b = 2.718,
        .inner = { .x = 100 },
        .nested = {
            .name = "local",
            .values = {0}
        }
    };
    
    /* 13. __typeof__ with parentheses */
    __typeof__(*global_ptr) val = 10;
    __typeof__(int[FOO(5)]) arr_type;
    
    /* 14. GCC builtins with balanced parentheses */
    int chosen = __builtin_choose_expr(
        __builtin_constant_p(1),
        FOO(10),
        BAR(20, 30)
    );
    
    /* 15. Nested switch-like macro expansion */
    int result = COMPLEX_MACRO(
        global_struct.a,
        val,
        chosen
    );
    
    /* 16. Array access with complex index */
    local_array[FOO(1)][BAR(1, 2)] = result;
    
    /* 17. Function pointer call */
    if (complex_func_ptr) {
        result = complex_func_ptr(3.14, result);
    }
    
    /* 18. Statement expression with braces */
    int stmt_expr = ({
        int temp = result;
        for (int i = 0; i < 5; i++) {
            temp += global_ptr[i];
        }
        temp;
    });
    
    /* 19. Alignas specifier (C11/C++11) */
    _Alignas(32) char aligned_buffer[64];
    
    /* 20. Designated initializer with array indices */
    int matrix[3][3] = {
        [0][0] = 1, [0][1] = 2, [0][2] = 3,
        [1] = {4, 5, 6},
        [2] = {[0] = 7, [1] = 8, [2] = 9}
    };
    
    return stmt_expr + matrix[0][0];
}

/* 21. Additional function with complex prototype */
static int (*(*complex_proto(int x, int y[]))[10])(void) {
    static int (*array_of_func_ptrs[10])(void);
    return &array_of_func_ptrs;
}

/* 22. Union with anonymous struct */
union Mixed {
    struct {
        int a;
        int b;
    };
    struct {
        float x;
        float y;
    } point;
    long long combined;
};

/* 23. Variable-length array in function (C99) */
void vla_function(int n) {
    int vla[n][n+1];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n+1; j++) {
            vla[i][j] = i * j;
        }
    }
}

/* 24. __attribute__ on function definition */
__attribute__((noinline, cold))
void attribute_func(void) {
    /* Empty but with attribute */
}

/* 25. Final check: ensure all constructs are used to avoid warnings */
void use_all(void) {
    (void)global_ptr;
    (void)attr_var;
    (void)vector_var;
    (void)section_var;
    (void)incomplete_array;
    attribute_func();
}
