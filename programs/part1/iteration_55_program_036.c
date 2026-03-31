/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with complex conditions */
static inline int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        __builtin_choose_expr(x > 0, x * 2, x / 2),
        ({ int y = x; y * y; })  /* Statement expression */
    );
}

/* Function with GNU C extensions in parameter */
static void attribute_test(int arr[static 10]) __attribute__((unused));
static void attribute_test(int arr[static 10]) {
    /* The 'static' in array parameter creates specialized tree nodes */
    for (int i = 0; i < 10; i++) arr[i] = i;
}

/* Function using __builtin_types_compatible_p */
static size_t type_compat_test(void *ptr) {
    /* Creates type comparison nodes */
    if (__builtin_types_compatible_p(typeof(*ptr), int))
        return sizeof(int);
    return 0;
}

/* C11 _Generic selector - creates distinct tree node */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* OpenMP test function */
static void omp_test(int n) {
    int sum = 0;
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    /* OpenMP target directive - may generate distinct nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 100; i++) {
        sum += 1;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function with case ranges - parsed into specific internal representation */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:   /* Case range creates specialized tree nodes */
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return -1;
    }
}

/* Function with trailing return type (C++ style in C via GNU extension) */
#ifdef __cplusplus
auto trailing_return_test(int x) -> decltype(x * 2) {
    return x * 2;
}
#endif

/* Test error recovery path */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expressions */
    result += builtin_choose_test(10);
    result += builtin_choose_test(-5);
    
    /* Test array with static index */
    int arr[10];
    attribute_test(arr);
    
    /* Test type compatibility */
    int x = 42;
    result += type_compat_test(&x);
    
    /* Test _Generic selector */
    double d = 3.14;
    const char *type_str = type_to_string(d);
    printf("Type: %s\n", type_str);
    
    /* Test OpenMP constructs */
    omp_test(1000);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(8);
    
    /* Test statement expressions in constant context */
    int complex_expr = ({ 
        int a = 5, b = 3; 
        __builtin_choose_expr(
            __builtin_constant_p(a),
            a * b,
            a + b
        );
    });
    result += complex_expr;
    
    /* Test attribute in unusual place */
    int unused_var __attribute__((unused)) = result;
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate error recovery nodes */
    void_func(123);  /* Wrong number of arguments */
    #endif
    
    /* Use result to prevent optimization */
    printf("Final result: %d\n", result);
    
    /* Additional complex expression with nested builtins */
    int final = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(result), int),
        ({ 
            /* Nested statement expression */
            int temp = result;
            while (temp > 10) temp /= 2;
            temp;
        }),
        0
    );
    
    printf("Processed: %d\n", final);
    printf("Done\n");
    
    return 0;
}

/* Additional OpenACC test if supported */
#ifdef _OPENACC
void acc_test(void) {
    int n = 1000;
    float *a = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel loop copyout(a[0:n])
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.5f;
    }
    
    free(a);
}
#endif
