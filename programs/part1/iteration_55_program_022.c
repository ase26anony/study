/* tree_coverage_test.c - Test program to exercise GCC's internal tree node classification */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wall tree_coverage_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int builtin_choose_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and __builtin_types_compatible_p */
static inline int type_dependent_expr(void) {
    return ({
        int result = 0;
        if (__builtin_types_compatible_p(typeof(result), int))
            result = 42;
        else if (__builtin_types_compatible_p(typeof(result), float))
            result = 99;
        result;
    });
}

/* Function with static array index parameter (creates specialized tree nodes) */
static void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function with switch case ranges (parsed into specific internal representation) */
static int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        case 11 ... 20:  /* Extended range */
            return val * 30;
        default:
            return -1;
    }
}

/* Function using _Generic selector (creates distinct tree node type) */
static const char* generic_selector(int x) {
    return _Generic(x,
        int: "integer",
        float: "float",
        double: "double",
        default: "unknown"
    );
}

/* Function with unusual attribute placement */
static int __attribute__((always_inline)) attributed_inline(int x) {
    int __attribute__((unused)) unused_var = x * 2;
    return x + 1;
}

/* Function with void parameter - to be called incorrectly */
static void void_func(void) {
    printf("Void function\n");
}

/* OpenMP test function - generates OMP nodes */
static void omp_test_parallel(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
}

/* OpenMP target test - may generate different OMP nodes */
static void omp_test_target(void) {
    int arr[100];
    
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    int total = 0;
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < 100; i++) {
        total += arr[i];
    }
    printf("OpenMP target total: %d\n", total);
}

/* Function with complex nested expressions using GNU extensions */
static int complex_gnu_expr(int x, int y) {
    /* Multiple nested __builtin_choose_expr */
    return __builtin_choose_expr(
        x > y,
        __builtin_choose_expr(
            __builtin_constant_p(x),
            ({ int z = x * y; z + 100; }),
            ({ int z = x + y; z * 2; })
        ),
        __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(x), int),
            x - y,
            ({ int z = y - x; z * 3; })
        )
    );
}

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test builtin choose expressions */
    result += builtin_choose_test(10);
    printf("builtin_choose_test: %d\n", result);
    
    /* Test type-dependent expressions */
    result += type_dependent_expr();
    printf("type_dependent_expr added: %d\n", result);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    printf("Array[5] = %d\n", arr[5]);
    
    /* Test switch with ranges */
    result += switch_with_ranges(7);
    printf("switch_with_ranges(7): added %d\n", result);
    
    /* Test _Generic selector */
    const char* type_str = generic_selector(result);
    printf("Type string: %s\n", type_str);
    
    /* Test attributed function */
    result = attributed_inline(result);
    printf("After attributed_inline: %d\n", result);
    
    /* Test complex GNU expressions */
    result = complex_gnu_expr(result, 5);
    printf("After complex_gnu_expr: %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test_parallel();
    omp_test_target();
    
    /* Test error path - void function call with argument */
    /* This is intentionally incorrect but may trigger internal paths */
    #ifdef TEST_ERROR_PATH
    /* The following line should generate a compilation error in normal use,
       but might create interesting tree nodes during error recovery */
    /* void_func(42); */  /* Commented out by default */
    #endif
    
    /* Additional edge cases */
    
    /* Array with static storage and attribute */
    static int __attribute__((aligned(16))) aligned_array[4] = {1, 2, 3, 4};
    
    /* Using __builtin_constant_p in control flow */
    if (__builtin_constant_p(result)) {
        printf("Result is constant: %d\n", result);
    } else {
        printf("Result is not constant\n");
    }
    
    /* Nested statement expression with OpenMP */
    int omp_result = ({
        int local_sum = 0;
        #pragma omp parallel for reduction(+:local_sum)
        for (int i = 0; i < 50; i++) {
            local_sum += i;
        }
        local_sum;
    });
    printf("Nested OMP result: %d\n", omp_result);
    
    /* Test with OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 3;
    }
    #endif
    
    printf("Final result: %d\n", result);
    printf("Test completed successfully.\n");
    
    return 0;
}
