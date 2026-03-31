/* test_gengtype_coverage.c
 * This file is designed to trigger the balanced character parsing
 * in gengtype-parse.cc lines 341-352.
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
int arr2[5][10];
int arr3[2][3][4];
extern int var_arr[FOO(5) ? 10 : 20];

/* 4. GCC attributes with parentheses and brackets */
int attr1 __attribute__((aligned(16)));
int attr2 __attribute__((vector_size(16)));
int attr3 __attribute__((format(printf, 1, 2)));

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
    int b;
};

/* 6. Complex initializers with nested braces */
struct Outer global_var = {
    .a = 1,
    .inner = {
        .x = 2,
        .u = { .f = 3.14 }
    },
    .b = 4
};

/* 7. Compound literals */
int *compound_lit = (int[]){1, 2, 3, 4, 5};
struct Outer *another = &(struct Outer){
    .a = 10,
    .inner = { .x = 20, .u = { .i = 30 } },
    .b = 40
};

/* 8. Preprocessor conditionals */
#ifdef TEST_DEFINE
    #define CONDITIONAL(x) ((x) + 100)
#else
    #define CONDITIONAL(x) ((x) + 200)
#endif

/* 9. __typeof__ usage */
__typeof__(*compound_lit) type_var;
__typeof__(complex_func_ptr) type_func;

/* 10. GCC builtins with parentheses */
int builtin_test = __builtin_choose_expr(1, 10, 20);
int align_test = __alignof__(struct Outer);

/* 11. Enum with array size */
enum { ARRAY_SIZE = 100 };
int enum_array[ARRAY_SIZE];

/* 12. Function with all constructs */
int test_function(int param)
{
    /* Local struct with initializer */
    struct Outer local = {
        .a = param,
        .inner = { .x = FOO(param), .u = { .i = BAR(param, 2) } },
        .b = NESTED(param)
    };
    
    /* Array with computed size */
    int local_arr[local.a > 0 ? 10 : 5];
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < sizeof((int[]){1,2,3,4})/sizeof(int); i++) {
        sum += ((int[]){1,2,3,4})[i];
    }
    
    /* __typeof__ in declaration */
    __typeof__(local.a) local_copy = local.a;
    
    /* Nested parentheses in expression */
    int result = ((((param + 1) * 2) - 3) / 4);
    
    /* Array indexing with brackets */
    result += arr2[1][2] + arr3[0][1][2];
    
    /* Function pointer call */
    if (complex_func_ptr) {
        result += complex_func_ptr(3.14);
    }
    
    return result + sum + local_copy;
}

/* 13. Union with anonymous struct */
union Mixed {
    struct {
        int a, b;
    };
    struct {
        float x, y;
    } f;
    long long ll;
};

/* 14. Designated initializers with nested designators */
union Mixed mixed = { .a = 1, .b = 2 };
union Mixed mixed2 = { .f = { .x = 1.0, .y = 2.0 } };

/* 15. Main function using everything */
int main(void)
{
    /* Trigger all parsing cases */
    int (*func_array[3])(int) = { test_function, test_function, test_function };
    
    struct {
        int a[FOO(3)];
        struct Outer o;
    } anonymous = { .a = {1,2,3}, .o = global_var };
    
    /* Complex expression with all bracket types */
    int value = test_function(
        FOO(
            BAR(
                arr2[1][2],
                ((int[]){1,2,3})[0]
            )
        )
    );
    
    /* __builtin_constant_p in array dimension */
    int dyn_arr[__builtin_constant_p(value) ? 10 : 20];
    
    /* Nested initializer */
    struct Outer nested_init = {
        .a = 1,
        .inner = {
            .x = 2,
            .u = {
                .i = ({
                    int temp = 3;
                    temp * 2;
                })
            }
        },
        .b = 4
    };
    
    /* Statement expression with braces */
    int stmt_expr = ({
        int x = 5;
        int y = 10;
        x + y;
    });
    
    return value + stmt_expr + func_array[0](5) + dyn_arr[0];
}
