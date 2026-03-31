/* test_tree_coverage.c - Program to exercise GCC's internal tree node generation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_expr_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x * 2, x / 2);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    __builtin_constant_p(_x) ? _x * 3 : _x + 1; \
})

/* Type-dependent expression using __builtin_types_compatible_p */
static int type_compatible_test(void) {
    int a = 5;
    double b = 3.14;
    
    /* Creates comparison nodes that might have uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return a + 1;
    }
    return 0;
}

/* Function with GNU attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) {
    return x * 2;
}

/* Function with static array parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* C11 _Generic selector expression */
#define TYPE_SELECTOR(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges (parsed into specific internal representation) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        default:
            return val;
    }
}

/* OpenMP test function */
static void omp_test(void) {
    int sum = 0;
    int array[100];
    
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 50; i++) {
        sum -= i;
    }
}

/* OpenACC pragma (analogous to OpenMP but distinct) */
static void openacc_test(void) {
    int data[100];
    
    #pragma acc parallel loop copy(data)
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
}

/* Function with void parameter - for potential error paths */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expression */
    result += builtin_choose_expr_test(10);
    
    /* Test statement expression macro */
    result += COMPLEX_EXPR(5);
    
    /* Test type compatibility */
    result += type_compatible_test();
    
    /* Test inline function with attributes */
    result += inline_func(7);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    const char *type_str = TYPE_SELECTOR(3.14);
    printf("Type: %s\n", type_str);
    
    /* Test case ranges */
    result += case_range_test(3);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test OpenACC */
    openacc_test();
    
    /* Potentially problematic code for error recovery paths */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning/error but will be parsed */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use __builtin_constant_p in complex expression */
    int x = 10;
    if (__builtin_constant_p(x) && __builtin_types_compatible_p(typeof(x), int)) {
        result += 100;
    }
    
    /* Compound literal with attributes */
    struct __attribute__((packed)) Point {
        int x;
        int y;
    } p = (struct Point){.x = 1, .y = 2};
    
    result += p.x + p.y;
    
    /* Aligned attribute */
    int __attribute__((aligned(64))) aligned_var = 42;
    result += aligned_var;
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
