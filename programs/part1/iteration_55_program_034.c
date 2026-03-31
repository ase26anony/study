/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wall tree_coverage_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x * 2, x / 2);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    int _x = (x); \
    typeof(_x) _result; \
    __builtin_constant_p(_x) ? (_result = _x * 3) : (_result = _x + 5); \
    _result; \
})

/* Type-dependent expression using __builtin_types_compatible_p */
static int type_compatible_test(void) {
    int x = 0;
    /* Creates comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        x = 42;
    }
    return x;
}

/* Function with static array index parameter (C99 feature) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selection (C11) */
static const char* generic_selector(int x) {
    return _Generic(x,
        int: "integer",
        float: "float",
        double: "double",
        default: "unknown"
    );
}

/* Function with GNU attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) {
    return x * 2;
}

/* Function with case ranges (GNU extension) */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return 1;
        case 6 ... 10:
            return 2;
        default:
            return 0;
    }
}

/* OpenMP helper function */
static void omp_helper(int n, int *result) {
    int sum = 0;
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    /* OpenMP target directive - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 100; i++) {
        sum += 1;
    }
    
    *result = sum;
}

/* OpenACC pragma (if supported) */
#ifdef _OPENACC
static void acc_test(void) {
    int x = 0;
    #pragma acc parallel loop copy(x)
    for (int i = 0; i < 100; i++) {
        x += i;
    }
}
#endif

/* Potentially problematic function declaration */
static void void_func(void);

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test various constructs */
    result += builtin_choose_test(10);
    result += COMPLEX_EXPR(5);
    result += type_compatible_test();
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    const char *type_str = generic_selector(42);
    printf("Type: %s\n", type_str);
    
    result += inline_func(5);
    result += case_range_test(3);
    
    /* OpenMP test */
    int omp_result = 0;
    omp_helper(100, &omp_result);
    result += omp_result;
    
    #ifdef _OPENACC
    acc_test();
    #endif
    
    /* Test error recovery path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning/error and potentially special tree nodes */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Definition of void function */
static void void_func(void) {
    /* Empty */
}

/* Additional complex expression using nested statement expressions */
static int nested_stmt_expr(void) {
    return ({
        int x = ({
            int y = 10;
            y * 2;
        });
        x + 5;
    });
}

/* Function using __builtin_constant_p in complex way */
static int constant_p_test(int x) {
    if (__builtin_constant_p(x) && x > 0) {
        return x * x;
    } else {
        return ({
            int tmp = x;
            while (tmp > 0) tmp--;
            tmp;
        });
    }
}
