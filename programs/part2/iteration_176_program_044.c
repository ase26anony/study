/* test-gengtype-coverage.c */
/* This file is designed to exercise the balanced character parsing
   in gengtype-parse.cc, specifically the switch cases for '(', '[', and '{' */

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

/* 3. Array declarations with brackets - multi-dimensional */
int multi_array[10][20];
int variable_array[FOO(5)][BAR(2, 3)];

/* 4. GCC attributes with parentheses and brackets */
int attr_var __attribute__((aligned(16)));
int vector_var __attribute__((vector_size(32)));
int deprecated_var __attribute__((deprecated("use new_var instead")));

/* 5. Struct with nested union and complex initializer */
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
        int data[4];
    } value;
};

/* Global instance with nested brace initializer */
struct Outer global_obj = { 
    .type = 1, 
    .value = { 
        .point = { 
            .x = FOO(10), 
            .y = BAR(5, 2) 
        } 
    } 
};

/* 6. Another struct with array member */
struct WithArray {
    int items[COMPLEX_MACRO(1, 2, 3)];
    char *names[][20];
};

/* 7. Preprocessor conditionals */
#ifdef TEST_CONDITIONAL
    #define SPECIAL_SIZE 100
    int conditional_array[SPECIAL_SIZE];
#else
    #define SPECIAL_SIZE 50
    int conditional_array[SPECIAL_SIZE];
#endif

/* 8. Using __typeof__ with parentheses */
__typeof__(*complex_func_ptr) func_return_type;
__typeof__(multi_array[0]) row_type;

/* 9. Compound literals */
int *compound_literal_ptr = (int[]){1, 2, 3, 4, 5};
struct Outer another_obj = (struct Outer){ 
    .type = 2, 
    .value = { 
        .rect = { 
            .width = 100, 
            .height = 200 
        } 
    } 
};

/* 10. Function declarations with complex parameters */
void process_array(int (*callback)(int[], int), int data[][10]);
int (*get_processor(void))(int, int);

/* 11. C++ style alignas (C11/C++11) */
_Alignas(32) int aligned_var;

/* 12. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(1, FOO(10), BAR(20, 30));
int constant_p = __builtin_constant_p(FOO(5));

/* Main function containing various constructs */
int main(void) {
    /* Local struct with initializer */
    struct Outer local_obj = { 
        .type = 3, 
        .value = { 
            .data = { 
                FOO(1), 
                BAR(2, 3), 
                COMPLEX_MACRO(4, 5, 6), 
                7 
            } 
        } 
    };
    
    /* Array with computed size */
    int dynamic_size_array[FOO(local_obj.value.data[0])];
    
    /* Nested compound literal */
    int *nested_literal = (int[]){ 
        (int){FOO(10)}, 
        (int){BAR(20, 30)}, 
        (int){COMPLEX_MACRO(40, 50, 60)} 
    };
    
    /* Using __typeof__ in expression */
    __typeof__(dynamic_size_array) *array_ptr = &dynamic_size_array;
    
    /* Complex expression with multiple parentheses */
    int result = FOO(BAR(COMPLEX_MACRO(1, 2, 3), FOO(4)));
    
    /* Prevent dead code elimination */
    if (result > 0) {
        return result + global_obj.type + local_obj.type;
    }
    
    return 0;
}

/* 13. Additional global with deeply nested braces */
struct DeeplyNested {
    struct {
        union {
            struct {
                int a;
                int b;
            } s;
            int arr[2];
        } u;
    } inner[3];
} deeply_nested = { 
    .inner = { 
        [0] = { .u = { .s = { .a = 1, .b = 2 } } },
        [1] = { .u = { .arr = { 3, 4 } } },
        [2] = { .u = { .s = { .a = 5, .b = 6 } } }
    } 
};

/* 14. Function pointer array */
int (*func_array[])(int, int) = { NULL, NULL, NULL };

/* 15. Variable Length Array (VLA) in function */
void vla_function(int n) {
    int vla[n][n+1];
    int (*vla_ptr)[n+1] = vla;
    
    /* Use VLA to avoid unused variable warning */
    if (n > 0) {
        vla[0][0] = FOO(n);
    }
}

/* 16. __auto_type usage (GCC extension) */
__auto_type auto_var = &global_obj;

/* 17. Statement expression (GCC extension) */
int statement_expr_result = ({
    int temp = FOO(10);
    temp * BAR(temp, 2);
});

/* 18. Array in struct with designated initializer */
struct ArrayStruct {
    int values[FOO(3)];
} array_struct = { .values = { [0] = 1, [FOO(1)] = 2, [2] = 3 } };
