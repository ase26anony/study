/* test_gengtype_coverage.c
 * This file is designed to exercise the balanced character parsing
 * in gengtype-parse.cc, specifically lines 341-352.
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
int arr2[5][20];
int arr3[][3] = {{1,2,3}, {4,5,6}};
enum { SIZE = 15 };
int var_arr[SIZE];
const int const_size = 20;
int dyn_arr[const_size];
int attr_arr[10] __attribute__((aligned(16)));

/* 4. GCC attributes with parentheses and brackets */
int x __attribute__((aligned(32)));
int y __attribute__((vector_size(16)));

/* 5. C++-style alignas (C11/C23) */
_Alignas(16) int aligned_var;

/* 6. Structure with nested union and complex initializer */
struct Outer {
    int a;
    union {
        int b;
        double c;
        struct {
            char d[10];
            int e;
        } inner;
    } u;
    int *f[5];
};

/* 7. Designated and nested initializers */
struct Outer global_struct = { 
    .a = 1, 
    .u = { 
        .inner = { 
            .d = {'t', 'e', 's', 't'}, 
            .e = 42 
        } 
    },
    .f = { NULL, NULL, NULL, NULL, NULL }
};

/* 8. Compound literal */
int *global_ptr = (int[]){1, 2, 3, 4, 5};

/* 9. Preprocessor conditional with triggering constructs */
#ifdef TEST_CONDITIONAL
    #define COND_MACRO(a) (a * 2)
    int conditional_var[COND_MACRO(5)];
#else
    #define COND_MACRO(a) (a * 3)
    int conditional_var[COND_MACRO(10)];
#endif

/* 10. __typeof__ usage */
__typeof__(*global_ptr) typed_var;

/* 11. GCC builtins with parentheses */
int builtin_result = __builtin_choose_expr(1, 100, 200);
int offset = __builtin_offsetof(struct Outer, u.inner.e);

/* Main function containing various constructs */
int main(void) {
    /* Function pointer usage */
    double (*local_func)(int) = (double (*)(int))0;
    
    /* Multi-dimensional array in function */
    int local_arr[3][4] = {{0}};
    
    /* Compound literal in function */
    int *local_ptr = (int[]){FOO(1), BAR(2,3), NESTED(4)};
    
    /* Nested structure initializer */
    struct Outer local_struct = {
        .a = 10,
        .u = { .b = 20 },
        .f = { local_ptr, NULL, NULL, NULL, NULL }
    };
    
    /* Array with computed size */
    int computed_arr[builtin_result > 50 ? 10 : 5];
    
    /* __typeof__ in function */
    __typeof__(local_arr[0]) row_copy;
    
    /* GCC attribute on local variable */
    int local_attr __attribute__((unused)) = 0;
    
    /* Lambda-like function definition (GCC extension) */
    int (*lambda)(int) = ({ 
        int __fn(int x) { return x * x; } 
        __fn; 
    });
    
    /* Complex expression with all bracket types */
    int result = (local_ptr[0] + local_arr[1][2]) * 
                 (global_struct.u.inner.e - 
                  sizeof(local_arr) / sizeof(local_arr[0]));
    
    /* Prevent dead code elimination */
    if (result > 0) {
        return 0;
    }
    return 1;
}

/* 12. Additional top-level constructs */
union Extra {
    struct {
        int a;
        int b;
    } s;
    long long ll;
} extra_union = { .s = { .a = 1, .b = 2 } };

/* 13. Pointer to array */
int (*ptr_to_array)[10];

/* 14. Function returning array pointer (invalid in C but parsed) */
// int (*returns_array(void))[10];  // This would trigger more parsing

/* 15. Nested attribute */
int z __attribute__((__aligned__(sizeof(int))));

/* 16. Statement expression (GCC extension) */
int stmt_expr = ({ 
    int temp = 5; 
    temp * temp; 
});

/* 17. Empty structures/unions (edge cases) */
struct EmptyStruct {};
union EmptyUnion {};

/* 18. Zero-length array (GCC extension) */
struct WithFlex {
    int count;
    char data[];
};

/* 19. Nested switch-case like pattern in comments won't affect parsing */
/* The following looks like the target code but is just a comment:
   case '(':
       consume_balanced('(', ')');
       break;
*/

/* 20. Ensure file ends with balanced constructs */
static int final_array[] = {1, 2, {3, 4}, 5};
