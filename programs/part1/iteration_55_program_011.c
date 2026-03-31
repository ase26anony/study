/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wno-unused-variable tree_coverage_test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and type compatibility check */
inline int type_complexity_test(void) {
    return ({
        int result = 0;
        if (__builtin_types_compatible_p(typeof(result), int))
            result = 42;
        else
            result = 24;
        result;
    });
}

/* Function using __builtin_constant_p in type-dependent expression */
static int constant_p_test(int arg) {
    int r = __builtin_constant_p(arg) ? 100 : 200;
    
    /* Nested __builtin_choose_expr */
    int s = __builtin_choose_expr(
        r > 150,
        __builtin_choose_expr(__builtin_constant_p(r), 1, 2),
        3
    );
    
    return r + s;
}

/* Function with static array parameter (creates specialized tree nodes) */
void static_array_param(int arr[static 10]) {
    /* Access ensures the parameter is used */
    arr[0] = 0;
}

/* Function using C11 _Generic selector */
#define TYPE_SELECTOR(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Function with switch case ranges (parsed into specific internal representation) */
int switch_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -1;
    }
}

/* Function with unusual attribute placement */
int __attribute__((unused)) unused_func(int __attribute__((unused)) param1, 
                                        int param2 __attribute__((unused))) {
    return 0;
}

/* OpenMP test function - generates OMP_* tree nodes */
void omp_test(void) {
    int i, sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    #pragma omp target teams distribute parallel for
    for (i = 0; i < 50; i++) {
        /* Empty loop body - just for structure generation */
    }
}

/* Function that might trigger internal error recovery paths */
#ifdef TEST_ERROR_PATH
void error_path_test(void) {
    /* Declare function with void parameter list */
    void foo(void);
    
    /* This should cause an error but may generate special tree nodes */
    /* Note: This line will cause compilation error if uncommented */
    /* foo(1); */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += choose_expr_test(10);
    result += type_complexity_test();
    result += constant_p_test(20);
    
    /* Test switch with ranges */
    result += switch_range_test(3);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    
    /* Test _Generic selector */
    const char *type_str = TYPE_SELECTOR(result);
    (void)type_str; /* Suppress unused warning */
    
    /* Test unused attribute function */
    result += unused_func(1, 2);
    
    /* Execute OpenMP region */
    omp_test();
    
    /* Additional complex expression using all builtins */
    int complex_expr = ({
        int x = 5;
        int y = __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(x), int),
            __builtin_constant_p(x) ? x * 2 : x * 3,
            0
        );
        y;
    });
    
    result += complex_expr;
    
    /* Use OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    #endif
    
    /* Final print to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Additional C++ specific code (compile with g++) */
#ifdef __cplusplus
// C++ specific constructs that may generate unique tree nodes
template<typename T>
auto trailing_return_test(T x) -> decltype(x * 2) {
    return x * 2;
}

void cpp_specific_tests() {
    // nullptr may create specific tree nodes
    int* ptr = nullptr;
    
    // User-defined literal (C++11)
    // Note: Would need actual UDL definition to compile
    // auto val = 42_km;
    
    (void)ptr;
}
#endif
