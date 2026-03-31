/* test_gengtype_balanced.c - Test file for gengtype balanced character parsing */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(a, b) ((a) * (b))
#define NESTED_MACRO(x) (FOO(x) + BAR(x, 2))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim[10][20];
int var_arr[__builtin_constant_p(1) ? 10 : 20];
extern int incomplete_array[];

/* 4. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((format(printf, 1, 2)));

/* 5. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_var;

/* 6. Struct/union definitions with nested braces */
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

/* 7. Complex initializer with nested braces */
struct Outer global_struct = { 
    .a = 1, 
    .inner = { 
        .x = 2, 
        .u = { .f = 3.14 } 
    },
    .arr = { [0] = 10, [1] = 20, [2] = 30 }
};

/* 8. Union with designated initializer */
union Data {
    int i;
    float f;
    char str[20];
} data = { .f = 2.718 };

/* 9. Preprocessor conditionals containing balanced characters */
#ifdef TEST_MACRO
    #define CONDITIONAL_MACRO(x) ({ \
        typeof(x) _x = (x); \
        _x * _x; \
    })
#else
    #define CONDITIONAL_MACRO(x) ((x) + 1)
#endif

/* 10. __typeof__ usage with parentheses */
__typeof__(*complex_func_ptr) func_return_type;

/* 11. Compound literal in declaration */
int *p = (int[]){1, 2, 3, 4, 5};

/* 12. Nested struct with array of structs */
struct Node {
    int value;
    struct Node *children[4];
};

/* Main function containing various balanced constructs */
int main(void) {
    /* Function pointer usage with parentheses */
    int result = FOO(5);
    result += BAR(result, 2);
    
    /* Array access with brackets */
    multi_dim[0][0] = 42;
    var_arr[5] = 10;
    
    /* Compound literal with braces */
    struct Outer local = { 
        .a = 100, 
        .inner = { .x = 200, .u = { .i = 300 } },
        .arr = { 1, 2, 3 }
    };
    
    /* Nested array initializer */
    int matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
    
    /* __builtin_choose_expr with parentheses */
    int choice = __builtin_choose_expr(
        __builtin_constant_p(result),
        result * 2,
        result / 2
    );
    
    /* Lambda-like expression using statement expression (GCC extension) */
    int lambda_result = ({
        int temp = choice;
        for (int i = 0; i < 3; i++) {
            temp += p[i];
        }
        temp;
    });
    
    /* Complex expression with all balanced characters */
    int final = (lambda_result > 0) ? 
                (matrix[0][0] + global_struct.arr[0]) : 
                (local.inner.u.i * choice);
    
    /* Prevent dead code elimination */
    volatile int sink = final;
    
    return sink > 0 ? 0 : 1;
}

/* 13. Additional constructs at file scope */
enum Color { RED, GREEN, BLUE };
typedef enum Color Color_t;

/* 14. Function with complex parameter list */
void complex_params(int a, int b[static 10], int (*callback)(int)) {
    /* Nested block with local struct */
    {
        struct Local {
            int data[5];
        } loc = { .data = {0} };
        
        /* Use all parameters */
        for (int i = 0; i < 10; i++) {
            b[i] = a + loc.data[i % 5];
        }
        
        if (callback) {
            callback(a);
        }
    }
}

/* 15. More GCC builtins with parentheses */
int builtin_test = __builtin_popcount(0xFF);
long builtin_align = __builtin_align_up(100, 16);

/* 16. Array with computed size using sizeof */
char buffer[sizeof(struct Outer) * 2];

/* 17. Pointer to array */
int (*ptr_to_array)[10] = &multi_dim[0];

/* 18. Flexible array member in struct */
struct FlexArray {
    int count;
    int data[];
};

/* 19. Initializer with nested compound literals */
struct Node node_tree = {
    .value = 1,
    .children = {
        &(struct Node){ .value = 2, .children = {NULL} },
        &(struct Node){ .value = 3, .children = {NULL} },
        NULL,
        NULL
    }
};

/* 20. Macro expanding to balanced characters */
#define WRAP(x) (((x) + 1) * 2)
int wrapped = WRAP(WRAP(WRAP(10)));
