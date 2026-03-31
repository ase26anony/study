/* test-gengtype-coverage.c
 * This file is specifically designed to exercise the balanced character
 * parsing in gengtype-parse.cc lines 341-352.
 */

/* 1. Function-like macros with parentheses */
#define FOO(x) (x + 1)
#define BAR(x, y) ((x) * (y))
#define NESTED(x) (FOO(x) + BAR(x, x))

/* 2. Complex declarators with parentheses */
int (*complex_func_ptr)(double);
int (*(*more_complex)[5])(void);
void (*signal(int sig, void (*handler)(int)))(int);

/* 3. Array declarations with brackets */
int arr1[10];
int arr2[10][20];
int arr3[][5] = {{1,2,3,4,5}, {6,7,8,9,10}};

/* 4. Array with non-constant size using enum */
enum { SIZE = 15 };
int var_arr1[SIZE];
const int const_size = 20;
int var_arr2[const_size];

/* 5. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(16)));
int y __attribute__((vector_size(16)));
int z __attribute__((aligned(32), packed));

/* 6. C++ style alignas (C11/C++11) */
_Alignas(16) int aligned_var;

/* 7. Struct with nested union and complex initializer */
struct Outer {
    int a;
    union {
        int b;
        double c;
        struct {
            int d[3];
            char e;
        } inner;
    } u;
    int *f;
};

/* Global instance with nested brace initializer */
struct Outer global_struct = {
    .a = 1,
    .u = { .inner = { .d = {10, 20, 30}, .e = 'X' } },
    .f = (int[]){100, 200, 300}
};

/* 8. Another struct with designated initializers */
struct Point {
    int x;
    int y;
    int z[2];
};

/* 9. Preprocessor conditionals containing triggering constructs */
#ifdef TEST_MACRO
    #define SPECIAL(x) ({ typeof(x) _x = (x); _x * 2; })
    int special_array[SPECIAL(5)];
#else
    #define SPECIAL(x) (x * 3)
#endif

/* 10. __typeof__ usage */
int typeof_example = 0;
__typeof__(typeof_example) another_var;

/* 11. Compound literal in declaration */
int *compound_lit_ptr = (int[]){1, 2, 3, 4, 5};

/* 12. Function declaration with array parameter */
void process_array(int matrix[][10], int rows);

/* Main function containing multiple triggering constructs */
int main(void) {
    /* Function-like macro invocation */
    int a = FOO(5);
    int b = BAR(a, 3);
    int c = NESTED(b);
    
    /* Array access with brackets */
    arr1[0] = a;
    arr2[1][2] = b;
    arr3[0][0] = c;
    
    /* Complex declarator usage */
    int (*local_func_ptr)(double) = 0;
    
    /* Compound literal */
    struct Point p = { .x = 1, .y = 2, .z = {3, 4} };
    
    /* Nested struct initialization */
    struct Outer local_struct = {
        .a = 42,
        .u = { .b = 99 },
        .f = (int[]){5, 10, 15}
    };
    
    /* GCC built-in with parentheses */
    int choice = __builtin_choose_expr(1, 100, 200);
    
    /* __typeof__ in expression */
    __typeof__(p.x) x_val = p.x;
    
    /* Array with computed size */
    int dyn_arr[__builtin_constant_p(1) ? 10 : 20];
    
    /* Attribute on local variable */
    int local_attr __attribute__((unused)) = 0;
    
    /* Use all variables to avoid dead code elimination */
    return a + b + c + arr1[0] + arr2[1][2] + choice + x_val + 
           global_struct.a + local_struct.a + p.x + compound_lit_ptr[0];
}

/* 13. Additional constructs at file scope */
/* Function definition with complex body */
void dummy_function(void) {
    /* Lambda-like expression using statement expression (GCC extension) */
    int result = ({ 
        int sum = 0; 
        for (int i = 0; i < 5; i++) sum += i; 
        sum; 
    });
    
    /* Nested blocks with braces */
    {
        int inner = 5;
        {
            int more_inner = 10;
            (void)more_inner;
        }
        (void)inner;
    }
    
    (void)result;
}

/* 14. Union with array */
union Data {
    int i;
    float f;
    char str[20];
    struct {
        int id;
        char name[30];
    } s;
};

/* 15. Initializer with deeply nested braces */
union Data data_instance = {
    .s = { .id = 1, .name = {'T', 'e', 's', 't', '\0'} }
};

/* 16. Macro that expands to something with all three characters */
#define TRIPLE_TRIGGER(x) { int y[(x)]; y[0] = (x); }
#ifdef USE_TRIPLE
    TRIPLE_TRIGGER(5)
#endif

/* 17. K&R style function definition (uses parentheses differently) */
int old_style(a, b)
    int a;
    int b;
{
    return a + b;
}

/* 18. Asm statement with braces (GCC extension) */
void asm_example(void) {
    int src = 1, dst;
    asm volatile ("mov %1, %0"
                  : "=r"(dst)
                  : "r"(src));
}

/* 19. Multiple dimensions with attributes */
int multi_attr[10] __attribute__((aligned(64), used, section(".data")));

/* 20. Final check - ensure file ends with balanced constructs */
static struct {
    int final_array[((sizeof(int) == 4) ? 10 : 20)];
    void (*final_func)(int[((sizeof(int) == 4) ? 10 : 20)]);
} final_struct = {
    .final_array = { [0 ... 9] = 255 },
    .final_func = 0
};
