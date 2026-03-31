/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr((cond), (a), (b))
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define IS_CONSTANT(expr) __builtin_constant_p(expr)

/* Statement expression with complex type */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        typeof(y) z = y + 1;
        z;
    });
}

/* Function with static array parameter (C99 feature) */
void process_array(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] *= 2;
    }
}

/* Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_with_attrs(int x) 
    __attribute__((const));

int inline_with_attrs(int x) {
    return x * 3;
}

/* _Generic selector (C11) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* C++-like nullptr simulation for C */
#define NULL_PTR ((void *)0)

/* Switch with case ranges (GNU extension) */
int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        default:
            return -1;
    }
}

/* Function with void parameter - will cause error if called with args */
void void_func(void) {
    printf("Void function\n");
}

/* OpenMP helper function */
void omp_parallel_sum(int *array, int size) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Complex expression using multiple GNU extensions */
int complex_expression(int a, int b) {
    /* Use __builtin_choose_expr with __builtin_constant_p */
    int result = CHOOSE_EXPR(
        IS_CONSTANT(a),
        a * b,
        ({ 
            /* Statement expression with typeof */
            typeof(a) temp = a;
            for (int i = 0; i < 3; i++) temp += b;
            temp;
        })
    );
    
    /* Use __builtin_types_compatible_p in expression */
    if (TYPE_COMPAT(result, int)) {
        result += 10;
    }
    
    return result;
}

/* Main function combining all constructs */
int main(void) {
    int arr[10];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        arr[i] = i + 1;
    }
    
    /* Test statement expressions */
    result = stmt_expr_test(5);
    printf("Statement expr result: %d\n", result);
    
    /* Test array with static parameter */
    process_array(arr);
    
    /* Test inline function with attributes */
    result = inline_with_attrs(result);
    printf("After inline with attrs: %d\n", result);
    
    /* Test _Generic selector */
    printf("Type of result: %s\n", type_name(result));
    printf("Type of arr: %s\n", type_name(arr));
    
    /* Test switch with ranges */
    result = switch_with_ranges(result % 10);
    printf("Switch with ranges result: %d\n", result);
    
    /* Test complex expression */
    result = complex_expression(result, 2);
    printf("Complex expression result: %d\n", result);
    
    /* Test OpenMP parallel region */
    omp_parallel_sum(arr, 10);
    
    /* Test error path (commented out by default) */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause a compilation error */
    void_func(1);  /* Calling void function with argument */
    #endif
    
    /* Additional OpenMP constructs */
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < 10; i++) {
        arr[i] = arr[i] * arr[i];
    }
    
    /* Use __builtin_constant_p in control flow */
    if (IS_CONSTANT(42)) {
        printf("Constant expression detected\n");
    }
    
    /* Qualifiers in array parameters */
    {
        int matrix[3][4];
        /* Access with multi-dimensional static hint */
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                matrix[i][j] = i * j;
            }
        }
    }
    
    /* Final print to prevent dead code elimination */
    printf("Test completed successfully\n");
    
    return 0;
}

/* Additional test functions for more coverage */

/* Function with GNU attribute in parameter */
void attribute_in_param(int x __attribute__((unused))) {
    /* Do nothing */
}

/* Using __builtin_choose_expr with complex conditions */
static int choose_complex(void) {
    return CHOOSE_EXPR(
        __builtin_constant_p(__LINE__),
        ({ 
            /* Nested statement expression */
            int x = __LINE__;
            x * 2;
        }),
        __LINE__
    );
}

/* OpenACC pragma (if supported) */
#ifdef _OPENACC
void acc_test(void) {
    int n = 1000;
    float *a = malloc(n * sizeof(float));
    float *b = malloc(n * sizeof(float));
    
    #pragma acc parallel loop copyout(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.5f;
    }
    
    #pragma acc parallel loop copyin(a[0:n]) copyout(b[0:n])
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2.0f;
    }
    
    free(a);
    free(b);
}
#endif
