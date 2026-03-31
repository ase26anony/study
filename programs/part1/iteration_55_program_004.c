/* test_tree_codes.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions for complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define IS_CONSTANT(expr) __builtin_constant_p(expr)
#define TYPES_COMPATIBLE(a, b) __builtin_types_compatible_p(a, b)

/* Statement expression creating complex tree structure */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        int z;
        switch (y) {
            case 1 ... 5:  /* Case range - specific internal representation */
                z = y + 1;
                break;
            case 6 ... 10:
                z = y - 1;
                break;
            default:
                z = y * y;
        }
        z;
    });
}

/* Function with static array parameter - creates specialized tree nodes */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * i;
    }
}

/* Type-generic selection using C11 _Generic */
#define TYPE_GENERIC(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function using __builtin_choose_expr in type-dependent context */
static int builtin_choose_test(int x) {
    /* This creates specialized comparison nodes */
    return CHOOSE_EXPR(
        TYPES_COMPATIBLE(typeof(x), int),
        x * 2,
        ({ 
            float f = x; 
            (int)(f * 3.0f); 
        })
    );
}

/* Function with unusual attributes */
int __attribute__((unused)) unused_var __attribute__((aligned(16))) = 42;

/* Function that might trigger internal error recovery paths */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* OpenMP test function - generates OMP nodes */
void omp_test(int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive - may create distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 100; i++) {
        sum += 1;
    }
}

/* Complex expression mixing multiple extensions */
int complex_expression(int x) {
    /* Mix statement expr, builtins, and conditional */
    return ({
        int result = 0;
        
        /* Use __builtin_constant_p in non-constant context */
        if (IS_CONSTANT(x)) {
            result = x;
        } else {
            /* Nested statement expression */
            result = ({
                int temp = x;
                for (int i = 0; i < 3; i++) {
                    temp = builtin_choose_test(temp);
                }
                temp;
            });
        }
        
        /* Add OpenMP pragma inside statement expression */
        #pragma omp parallel
        {
            result += 1;
        }
        
        result;
    });
}

/* Function with GNU extension: label as value */
void label_as_value(void) {
    void *ptr = &&label;
    goto *ptr;
label:
    printf("Jumped to label\n");
}

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(5);
    printf("stmt_expr_test: %d\n", result);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    
    /* Test type-generic selection */
    int x = 42;
    double y = 3.14;
    printf("Type of x: %s\n", TYPE_GENERIC(x));
    printf("Type of y: %s\n", TYPE_GENERIC(y));
    
    /* Test builtin choose expressions */
    result += builtin_choose_test(10);
    printf("builtin_choose_test: %d\n", result);
    
    /* Test complex expression */
    result += complex_expression(7);
    printf("complex_expression: %d\n", result);
    
    /* Test OpenMP */
    omp_test(100);
    
    /* Test label as value */
    label_as_value();
    
    /* Potentially problematic code for error recovery */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning/error but compile */
    void_func(123);  /* Calling void function with argument */
    #endif
    
    /* Array with designated initializer and range */
    int ranged_array[] = {
        [0 ... 9] = 1,  /* Another case range in initializer */
        [10] = 2
    };
    
    /* Use attribute on variable in middle of code */
    int __attribute__((unused)) temp_var = result;
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return 0;
}
