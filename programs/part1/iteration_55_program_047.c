/* test_tree_codes.c - Comprehensive test for GCC internal tree node generation */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with complex constant expressions */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x) && (x > 0),
        x * 2,
        __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(x), int),
            x + 1,
            x - 1
        )
    );
}

/* Statement expression in constant context */
#define COMPLEX_MACRO(val) ({ \
    typeof(val) __v = (val); \
    __builtin_constant_p(__v) ? __v * 3 : __v + 7; \
})

/* Function with GNU attributes in unusual places */
inline int __attribute__((always_inline)) 
attributed_func(int i __attribute__((unused)), 
                float f __attribute__((may_alias))) {
    return (int)f + i;
}

/* Function with static array parameter (creates specialized tree nodes) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* C11 _Generic selector - creates distinct tree node */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    float: "float", \
    double: "double", \
    default: "unknown" \
)

/* C++ style nullptr simulation for C */
#define nullptr ((void*)0)

/* Switch with case ranges - parsed into specific internal representation */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 10;
        case 6 ... 10:
            return x * 20;
        case 100 ... 200:
            return x / 2;
        default:
            return -1;
    }
}

/* OpenMP helper with reduction */
int omp_parallel_sum(int *arr, int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* OpenMP target offloading - may create distinct tree nodes */
void omp_target_test(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * i;
    }
    
    free(data);
}

/* Function with void parameter - for error path testing */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expressions */
    result += builtin_choose_test(5);
    result += builtin_choose_test(-3);
    
    /* Test statement expressions */
    result += COMPLEX_MACRO(10);
    int dynamic_val = 20;
    result += COMPLEX_MACRO(dynamic_val);
    
    /* Test attributed function */
    result += attributed_func(5, 3.14f);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    
    /* Test _Generic selector */
    int int_val = 42;
    float float_val = 3.14f;
    const char *type1 = type_to_string(int_val);
    const char *type2 = type_to_string(float_val);
    printf("Types: %s, %s\n", type1, type2);
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    result += switch_with_ranges(150);
    
    /* Test OpenMP parallel sum */
    int omp_arr[100];
    for (int i = 0; i < 100; i++) {
        omp_arr[i] = i + 1;
    }
    result += omp_parallel_sum(omp_arr, 100);
    
    /* Test OpenMP target (if supported) */
    #ifdef _OPENMP
    if (omp_get_num_devices() > 0) {
        omp_target_test(50);
    }
    #endif
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning but compile */
    void_func();
    /* This would be an error but we're testing internal paths */
    /* void_func(42); */  /* Uncomment to test error recovery paths */
    #endif
    
    /* Use nullptr-like construct */
    void *ptr = nullptr;
    if (ptr == NULL) {
        result += 1000;
    }
    
    /* Additional complex expression with typeof */
    typeof(result) result_copy = result;
    result = __builtin_types_compatible_p(typeof(result), int) ? 
             result_copy * 2 : result_copy;
    
    /* Array with designators (C99) - creates CONSTRUCTOR nodes */
    int designated[5] = {[0] = 1, [2] = 3, [4] = 5};
    for (int i = 0; i < 5; i++) {
        result += designated[i];
    }
    
    /* Compound literal */
    struct point { int x, y; };
    struct point p = (struct point){ .x = result, .y = result * 2 };
    result = p.x + p.y;
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
