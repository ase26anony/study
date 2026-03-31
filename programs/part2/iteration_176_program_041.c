/* test_gengtype_coverage.c - C test file for gengtype parse coverage */

/* 1. Function-like macros with parentheses */
#define FOO(x) ((x) + 1)
#define BAR(x, y) ((x) * (y))
#define COMPLEX_MACRO(a, b, c) \
    do { \
        (a) = (b) + (c); \
    } while(0)

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double, int);
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_array[5])(void))[10];

/* 3. Array declarations with brackets */
int multi_dim_array[10][20][30];
enum { SIZE = 100 };
int var_size_array[SIZE];
const int const_size = 50;
int another_array[const_size];

/* 4. GCC attributes with parentheses and brackets */
int aligned_var __attribute__((aligned(16)));
int packed_struct __attribute__((packed));
int vector_var __attribute__((vector_size(32)));

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
            int x;
            int y;
        } point;
        struct {
            int width;
            int height;
        } rect;
    } data;
    int (*callback)(int);
};

/* Global instance with nested brace initializer */
struct Outer global_instance = {
    .type = 1,
    .data = {
        .point = {
            .x = 10,
            .y = {20}  /* Nested braces */
        }
    },
    .callback = NULL
};

/* 7. Another struct with designated initializers */
struct Nested {
    int a;
    int b[3];
    struct {
        float f;
        double d;
    } inner;
};

/* 8. __typeof__ usage */
int some_int = 42;
__typeof__(some_int) typed_var;

/* 9. Compound literal in global scope */
int *global_ptr = (int[]){1, 2, 3, 4};

/* Main function containing various constructs */
int main(void) {
    /* 10. Compound literal */
    int *local_ptr = (int[]){5, 6, 7, 8};
    
    /* 11. Nested struct initialization */
    struct Nested nested = {
        .a = 100,
        .b = {200, 300, 400},
        .inner = {
            .f = 3.14f,
            .d = 2.71828
        }
    };
    
    /* 12. Array with computed size using ternary */
    int dynamic_size = 10;
    int computed_array[dynamic_size > 5 ? 20 : 10];
    
    /* 13. __builtin_choose_expr with parentheses */
    int chosen = __builtin_choose_expr(
        sizeof(int) == 4,
        42,
        24
    );
    
    /* 14. Lambda-like expression using statement expression (GCC extension) */
    int result = ({
        int temp = FOO(10);
        temp * BAR(temp, 2);
    });
    
    /* 15. Complex expression with multiple parentheses */
    int complex_result = (FOO(5) * BAR(3, 4)) + (complex_func_ptr ? 1 : 0);
    
    /* 16. Nested array access with brackets */
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int element = matrix[1][2];
    
    /* 17. Switch statement with braces */
    switch (result) {
        case 1: {
            int inner = 100;
            break;
        }
        case 2: {
            int inner = 200;
            break;
        }
        default: {
            int inner = 300;
            break;
        }
    }
    
    /* 18. Do-while with braces */
    do {
        int counter = 0;
        counter++;
    } while (0);
    
    /* 19. For loop with complex initializer */
    for (int i = 0, j = 10; i < j; i++, j--) {
        int product = i * j;
    }
    
    /* Prevent dead code elimination */
    return result + complex_result + element + chosen + typed_var;
}

/* 20. Function with complex parameter list and body */
static int complex_function(
    int param1,
    int param2[][10],
    void (*callback)(int, int)
) {
    /* Nested block with braces */
    {
        int local = param1;
        local += param2[0][0];
        
        if (callback) {
            (*callback)(local, 42);
        }
    }
    
    return 0;
}

/* 21. Union with anonymous struct (C11) */
union Anonymous {
    struct {
        int a;
        int b;
    };
    long long ll;
};

/* 22. Alignas specifier (C11/C++11) */
_Alignas(32) char aligned_buffer[256];

/* 23. Static assertions with parentheses */
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

/* 24. More macro complexity */
#define NESTED_PARENS(a) (((a) + 1) * ((a) - 1))
#define BRACKET_MACRO(arr) arr[0] + arr[1]
#define BRACE_MACRO { return 42; }

/* 25. Function returning array pointer */
int (*return_array_ptr(int size))[] {
    static int arr[100];
    return &arr;
}

/* 26. K&R style function definition (older style) */
int old_style_function(x, y)
    int x;
    int y[];
{
    return x + y[0];
}
