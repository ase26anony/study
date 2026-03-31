/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wall tree_coverage_test.c -o test */

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
    int a = 5;
    double b = 3.14;
    
    /* Creates comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return a + 1;
    }
    if (__builtin_types_compatible_p(typeof(b), double)) {
        return (int)b;
    }
    return 0;
}

/* Function with static array index parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with GNU attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused)), 
            int y __attribute__((deprecated))) {
    return y * 2;
}

/* _Generic selection expression (C11) - creates distinct tree node */
#define TYPE_GENERIC(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges - parsed into specific internal representation */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 10 ... 20:
            return val + 100;
        default:
            return -1;
    }
}

/* OpenMP helper function */
static void omp_helper(int *data, int n) {
    int i;
    #pragma omp parallel for private(i)
    for (i = 0; i < n; i++) {
        data[i] = data[i] * 2 + 1;
    }
}

/* Another OpenMP construct with teams/distribute */
static void omp_teams_test(int *data, int n) {
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = data[i] % 100;
    }
}

/* Function with void parameter - for error path testing */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test builtin choose expression */
    result += builtin_choose_test(5);
    
    /* Test statement expression */
    result += COMPLEX_EXPR(10);
    
    /* Test type compatibility */
    result += type_compatible_test();
    
    /* Test static array parameter */
    result += static_array_param(arr);
    
    /* Test inline function with attributes */
    result += inline_func(1, 2);
    
    /* Test _Generic selection */
    const char *type_str = TYPE_GENERIC(3.14);
    printf("Type: %s\n", type_str);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(15);
    
    /* Test OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP thread count: %d\n", omp_get_num_threads());
        }
        
        #pragma omp for reduction(+:result)
        for (int i = 0; i < 100; i++) {
            result += i % 10;
        }
    }
    
    /* Test OpenMP helper */
    omp_helper(arr, 10);
    
    /* Test OpenMP teams (may create distinct tree nodes) */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 10; i++) {
        arr[i] = arr[i] * 3;
    }
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate an error but may create interesting tree nodes */
    void_func(42);  /* Wrong number of arguments */
    #endif
    
    /* Use qualifiers in array parameters */
    int (*func_ptr)(int arr[static 10]) = static_array_param;
    result += func_ptr(arr);
    
    /* Final print to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional C++-like constructs if compiled as C++ */
#ifdef __cplusplus
/* User-defined literal (C++11) */
constexpr unsigned long long operator"" _custom(unsigned long long n) {
    return n * 2;
}

/* Template with trailing return type */
template<typename T>
auto template_func(T x) -> decltype(x * 2) {
    return x * 2;
}
#endif
