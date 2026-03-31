/* test_tree_codes.c - Comprehensive test for GCC internal tree node generation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with complex conditions */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        __builtin_choose_expr(x > 0, x * 2, x / 2),
        ({ int y = x; y * y + 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int statement_expr_test(int a) {
    return ({
        int tmp = a;
        __builtin_types_compatible_p(typeof(tmp), int) ? 
            tmp * 3 : tmp;
    });
}

/* Function using __builtin_constant_p in type-dependent expression */
static int constant_p_test(void) {
    int x = 5;
    /* This may generate comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(
            typeof(__builtin_constant_p(x) ? (void)0 : (int*)0),
            void*)) {
        return 1;
    }
    return 0;
}

/* Function with array parameter using static qualifier */
int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
static int generic_selector_test(void) {
    return _Generic(1.0f,
        float: 1,
        double: 2,
        int: 3,
        default: 4
    );
}

/* Function with case ranges (GNU extension) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val + 5;
        default:
            return -1;
    }
}

/* Function with attributes in unusual places */
static int __attribute__((always_inline)) 
attributed_test(int x __attribute__((unused))) {
    int y __attribute__((unused)) = x;
    return 0;
}

/* OpenMP test function - generates various OpenMP tree nodes */
static void omp_test(void) {
    int i, sum = 0;
    int array[100];
    
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel for - may generate OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    /* OpenMP target teams distribute */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (i = 0; i < 50; i++) {
        sum -= i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function with void parameter - for potential error recovery paths */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    return;
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test various GNU extensions */
    result += builtin_choose_test(10);
    result += statement_expr_test(5);
    result += constant_p_test();
    result += generic_selector_test();
    result += case_range_test(3);
    result += attributed_test(7);
    
    /* Test static array parameter */
    result += static_array_test(test_array);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Potentially problematic code for error recovery */
    #ifdef TEST_ERROR_PATH
    /* This should cause an error but may generate special tree nodes */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Test completed - check tree dumps for uncovered paths\n");
    
    return 0;
}

/* Additional complex patterns in global scope */
/* Using __builtin_choose_expr with __builtin_types_compatible_p */
static int global_test_var = 
    __builtin_choose_expr(
        __builtin_types_compatible_p(int*, void*),
        0,
        __builtin_choose_expr(
            __builtin_constant_p(1),
            100,
            200
        )
    );

/* Constructor-like initialization with statement expression */
struct complex_struct {
    int a;
    int b;
};

static struct complex_struct cs = {
    .a = ({ int x = 5; x * 2; }),
    .b = __builtin_choose_expr(1, 10, 20)
};
