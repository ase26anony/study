/* test_tree_codes.c */
/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test test_tree_codes.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_expr_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x + 1, x - 1);
}

/* Statement expression in constant context */
#define STMT_EXPR_TEST(x) ({ \
    int _result = (x); \
    typeof(x) _tmp = _result; \
    _tmp * 2; \
})

/* Type-dependent expression with __builtin_types_compatible_p */
static int types_compatible_test(void) {
    int x = 5;
    double y = 3.14;
    
    /* Creates comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        x = __builtin_constant_p(x) ? 10 : 20;
    }
    
    return x;
}

/* Function with static array parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with GNU attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) {
    return x * 2;
}

/* _Generic selector expression (C11) */
#define TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return x;
    }
}

/* OpenMP test function */
static void omp_test(void) {
    int i;
    int sum = 0;
    int arr[100];
    
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target teams - may generate additional internal nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (i = 0; i < 50; i++) {
        sum -= i;
    }
}

/* Function with void parameter */
static void void_func(void) {
    printf("Void function\n");
}

/* Main function combining all patterns */
int main(void) {
    int result;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test builtin choose expr */
    result = builtin_choose_expr_test(5);
    printf("Builtin choose: %d\n", result);
    
    /* Test statement expression */
    result = STMT_EXPR_TEST(10);
    printf("Statement expr: %d\n", result);
    
    /* Test types compatible */
    result = types_compatible_test();
    printf("Types compatible: %d\n", result);
    
    /* Test static array parameter */
    result = static_array_param(arr);
    printf("Static array: %d\n", result);
    
    /* Test inline function with attributes */
    result = inline_func(5);
    printf("Inline func: %d\n", result);
    
    /* Test _Generic selector */
    printf("Type name (int): %s\n", TYPE_NAME(5));
    printf("Type name (double): %s\n", TYPE_NAME(3.14));
    
    /* Test case ranges */
    result = case_range_test(3);
    printf("Case range: %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test void function call (with error path if enabled) */
    void_func();
    
    #ifdef TEST_ERROR_PATH
    /* This would be a type error but might trigger internal paths */
    /* void_func(1); */  /* Uncomment to test error recovery paths */
    #endif
    
    /* Complex expression combining multiple extensions */
    int complex_result = ({
        int x = 5;
        __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(x), int),
            __builtin_constant_p(x) ? STMT_EXPR_TEST(x) : x * 3,
            x
        );
    });
    
    printf("Complex result: %d\n", complex_result);
    
    /* Additional OpenACC-like pragma (treated specially by some GCC versions) */
    #ifdef __OPENACC__
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        arr[i] *= 2;
    }
    #endif
    
    printf("Done\n");
    return 0;
}
