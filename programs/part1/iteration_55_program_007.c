/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -c tree_test.c */
/* Additional flags for testing: -DTEST_ERROR_PATH -Wno-unused-variable -Wno-unused-function */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates complex internal tree nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and type compatibility check */
inline int type_check_test(void) {
    return __builtin_types_compatible_p(int, long) 
           ? 0 
           : ({ 
               typeof(int) z = 42; 
               __builtin_choose_expr(1, z, 0);
             });
}

/* Function with static array parameter (GNU extension) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic (C11) - creates SELECTOR tree nodes */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Function with switch case ranges (GNU extension) */
static int switch_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -1;
    }
}

/* Function with attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func_with_attrs(int x __attribute__((unused))) {
    int y __attribute__((unused)) = x;
    return 0;
}

/* OpenMP test function - generates various OpenMP tree nodes */
static void omp_test(void) {
    int sum = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP target teams - may generate additional OpenMP nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 100; i++) {
        sum += arr[i] * 2;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    /* Declare function with void parameter */
    void foo(void);
    
    /* This should cause an error but may generate special tree nodes */
    /* foo(1); */  /* Uncomment to test error recovery */
    
    /* Array with static size in parameter (another GNU extension) */
    void bar(int n, int array[n]);
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += choose_expr_test(10);
    result += type_check_test();
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    const char* type_name = get_type_name(result);
    printf("Type name: %s\n", type_name);
    
    /* Test switch with ranges */
    result += switch_range_test(3);
    
    /* Test function with attributes */
    result += inline_func_with_attrs(result);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test various constant expressions with builtins */
    int complex_expr = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(result), int),
        ({ 
            int temp = result;
            __builtin_constant_p(temp) ? temp * 2 : temp / 2;
        }),
        result
    );
    
    /* Use statement expression in a loop */
    for (int i = 0; i < 5; i++) {
        int val = ({ 
            int x = i * 2; 
            __builtin_choose_expr(i > 2, x + 1, x - 1); 
        });
        result += val;
    }
    
    /* Test with OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    #endif
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific constructs if compiled as C++ */
#ifdef __cplusplus
namespace {
    /* User-defined literal */
    long double operator"" _deg(long double deg) {
        return deg * 3.14159265358979323846L / 180;
    }
    
    /* Template with trailing return type */
    template<typename T, typename U>
    auto template_test(T t, U u) -> decltype(t + u) {
        return t + u;
    }
}
#endif
