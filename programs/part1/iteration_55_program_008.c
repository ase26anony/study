/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -c tree_coverage.c */
/* Additional flags for testing: -DTEST_ERROR_PATH -Wno-unused-variable -Wno-unused-function */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int builtin_choose_expr_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int statement_expr_test(void) {
    return ({
        int a = 5;
        int b = 10;
        /* Using __builtin_types_compatible_p creates comparison nodes */
        if (__builtin_types_compatible_p(typeof(a), int))
            a + b;
        else
            b - a;
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
static void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using _Generic selector (C11 feature) */
static const char* generic_selector_test(_Bool b) {
    return _Generic(b,
        _Bool: "boolean",
        default: "other"
    );
}

/* Function with switch case ranges */
static int switch_case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -1;
    }
}

/* Function with unusual attributes */
static void attribute_test(void) {
    /* Variable with multiple attributes */
    int x __attribute__((unused, aligned(16))) = 42;
    
    /* Function attribute on local declaration */
    static int __attribute__((always_inline)) 
    inline_helper(int a) __attribute__((unused));
    
    static int __attribute__((always_inline))
    inline_helper(int a) {
        return a * 3;
    }
    
    /* Using the helper */
    int result = inline_helper(x);
    (void)result;
}

/* OpenMP test function */
static void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive (may create distinct tree nodes) */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 50; i++) {
        /* Empty but generates OpenMP structure */
    }
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    /* Declare function with void parameter list */
    void foo(void);
    
    /* This should cause an error but may generate special tree nodes */
    /* foo(1); */  /* Uncomment to test error recovery */
    
    /* Array with static size in parameter but wrong usage */
    void bar(int n, int arr[n]) {
        (void)arr;
        (void)n;
    }
}
#endif

/* Main function combining all tests */
int main(void) {
    int result;
    
    /* Test 1: Builtin choose expression */
    result = builtin_choose_expr_test(10);
    printf("builtin_choose_expr_test: %d\n", result);
    
    /* Test 2: Statement expression */
    result = statement_expr_test();
    printf("statement_expr_test: %d\n", result);
    
    /* Test 3: Static array parameter */
    int arr[10];
    static_array_param(arr);
    printf("static_array_param: arr[5] = %d\n", arr[5]);
    
    /* Test 4: Generic selector */
    const char* type_str = generic_selector_test(1);
    printf("generic_selector_test: %s\n", type_str);
    
    /* Test 5: Switch with case ranges */
    result = switch_case_range_test(3);
    printf("switch_case_range_test(3): %d\n", result);
    
    /* Test 6: Attribute test */
    attribute_test();
    
    /* Test 7: OpenMP test */
    omp_test();
    
    #ifdef TEST_ERROR_PATH
    /* Test 8: Error path test (if enabled) */
    error_path_test();
    #endif
    
    /* Additional complex expression using multiple extensions */
    int complex_result = ({
        int x = 5;
        __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(x), int),
            ({ 
                #pragma omp parallel
                {
                    x = x * 2;
                }
                x + 1;
            }),
            x - 1
        );
    });
    printf("complex_result: %d\n", complex_result);
    
    /* User-defined literal style (GNU extension) */
    int binary_literal = 0b1010;
    printf("binary_literal: %d\n", binary_literal);
    
    /* Compound literal with attribute */
    struct point {
        int x;
        int y;
    } p = (struct point){ .x = 1, .y = 2 };
    printf("compound literal: (%d, %d)\n", p.x, p.y);
    
    printf("Done\n");
    return 0;
}
