/* test_gengtype_coverage.c - C test file for gengtype parse coverage */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20][30];
enum { SIZE = 100 };
int var_size[SIZE];
const int const_size = 50;
int dynamic_like[const_size];
int attr_array[10] __attribute__((aligned(64)));

/* 4. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16), packed));
int y __attribute__((vector_size(16)));

/* 5. Preprocessor conditionals */
#ifdef __GNUC__
    #define GCC_SPECIFIC(x) __builtin_expect(!!(x), 1)
#else
    #define GCC_SPECIFIC(x) (x)
#endif

/* 6. Struct with nested union and complex initializer */
struct Outer {
    int type;
    union {
        struct {
            int a;
            int b[5];
        } s;
        double d;
        void *p;
    } u;
    int (*callback)(struct Outer *);
};

/* Global instance with nested brace initializer */
struct Outer global_var = {
    .type = 1,
    .u = {
        .s = {
            .a = FOO(10),
            .b = {1, 2, [4] = 5}
        }
    },
    .callback = 0
};

/* 7. Another struct with designated initializers */
struct Point {
    int x, y, z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
} rect = {
    .top_left = { .x = 0, .y = 0, .z = {0} },
    .bottom_right = { 100, 200, 300 },
    .id = 42
};

/* 8. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 9. Function using all constructs */
int process_data(int input) {
    /* Function pointer usage */
    int (*local_func)(int) = (int (*)(int))0;
    
    /* Array with computed size */
    int local_arr[input > 0 ? 10 : 20];
    
    /* Compound literal */
    struct Point *p = &(struct Point){ .x = 1, .y = 2, .z = 3 };
    
    /* Nested braces in initializer */
    int matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
    
    /* __typeof__ with parentheses */
    __typeof__(*p) point_copy = {0};
    
    /* GCC built-in with parentheses */
    int result = __builtin_choose_expr(input > 0, 
                                      FOO(input), 
                                      BAR(input, 2));
    
    /* Attribute in local variable */
    int aligned_var __attribute__((aligned(8))) = result;
    
    /* Nested switch-like macro expansion */
    return NESTED(result) + aligned_var;
}

/* 10. Main function with mixed constructs */
int main(void) {
    /* Complex expression with parentheses */
    int value = (complex_func_ptr ? 1 : 0) + (sizeof(multi_dim) / sizeof(int));
    
    /* Array access with brackets */
    value += multi_dim[0][0][0];
    value += var_size[10];
    
    /* Struct member access through pointer */
    struct Outer *op = &global_var;
    value += op->u.s.b[2];
    
    /* Compound literal in expression */
    value += ((int[]){10, 20, 30})[1];
    
    /* Nested initializer in local variable */
    struct Rectangle local_rect = {
        .top_left = { .x = 0, .y = 0, .z = 0 },
        .bottom_right = { .x = 50, .y = 50, .z = 50 },
        .id = value
    };
    
    /* Function call with parentheses */
    value = process_data(value);
    
    /* __builtin_constant_p in array size */
    int smart_arr[__builtin_constant_p(value) ? 10 : 20];
    smart_arr[0] = value;
    
    /* Lambda-like function pointer (GCC extension) */
    int (*lambda)(int) = ({ 
        int __fn(int x) { return x * 2; } 
        __fn; 
    });
    
    value = lambda(value);
    
    /* Return statement with complex expression */
    return (value > 0) ? value : 
           (global_ptr ? global_ptr[0] : 
            (rect.id + sizeof(struct Outer)));
}

/* 11. Additional edge cases at file scope */
/* Empty braces */
struct Empty { } empty_instance = {};

/* Zero-length array (GCC extension) */
struct Header {
    int type;
    int data[0];
};

/* Alignment specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[64];

/* Nested parentheses in macro arguments */
#define ULTRA_NESTED(a, b, c) (((a) + (b)) * (c))
int ultra_result = ULTRA_NESTED(1, (2 + 3), (4 * (5 - 1)));

/* Array in struct with nested initializer */
struct WithArray {
    int counts[3][2];
} with_array = {
    .counts = { {1, 2}, {3, 4}, {5, 6} }
};

/* 12. Multiple levels of nesting */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int data;
        } l3;
    } l2[2];
} deeply_nested = {
    .l2 = {
        [0] = { .l3 = { .data = 100 } },
        [1] = { .l3 = { .data = 200 } }
    }
};
