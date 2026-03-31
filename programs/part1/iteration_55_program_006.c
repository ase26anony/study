/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test_tree test_tree.c */
/* Also try: gcc -O2 -fopenmp -std=gnu++17 -fdump-tree-optimized -xc++ test_tree.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with static array parameter (creates specialized tree nodes) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_types_compatible_p in type-dependent expression */
#define CHECK_TYPE(x) \
    __builtin_types_compatible_p(typeof(x), int) ? 1 : \
    __builtin_types_compatible_p(typeof(x), double) ? 2 : 0

int type_check_test(void) {
    int i = 5;
    double d = 3.14;
    return CHECK_TYPE(i) + CHECK_TYPE(d);
}

/* C11 _Generic selector (creates distinct tree node) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) 
    __attribute__((const));
    
int inline_func(int x) {
    return x * x;
}

/* OpenMP test function */
void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive (may create different tree nodes) */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 50; i++) {
        /* Empty loop body - just for tree generation */
    }
}

/* Function with switch case ranges */
int switch_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return 10;
        case 6 ... 10:
            return 20;
        default:
            return 0;
    }
}

/* Try to generate error recovery nodes */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test __builtin_choose_expr */
    result += choose_expr_test(10);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test type checking */
    result += type_check_test();
    
    /* Test _Generic */
    printf("Type of 5: %s\n", type_name(5));
    printf("Type of 3.14: %s\n", type_name(3.14));
    
    /* Test inline function with attributes */
    result += inline_func(5);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test switch with ranges */
    result += switch_range_test(3);
    
    /* Try to trigger error recovery paths */
    #ifdef TEST_ERROR_PATH
    /* This should cause an error but might generate interesting tree nodes */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use statement expression in constant context */
    int complex_expr = ({ 
        int x = 10; 
        int y = 20;
        __builtin_choose_expr(
            __builtin_constant_p(x + y),
            x + y,
            x - y
        );
    });
    result += complex_expr;
    
    /* Array with static index (GNU extension) */
    int static_idx_array[10];
    for (int i = 0; i < 10; i++) {
        static_idx_array[i] = i * i;
    }
    
    /* Try OpenACC-like pragma (might be interpreted specially) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        static_idx_array[i] += 1;
    }
    #endif
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific constructs if compiled as C++ */
#ifdef __cplusplus
// User-defined literal
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

// nullptr usage in template context
template<typename T>
T* get_ptr() {
    return nullptr;
}

// Trailing return type
auto trailing_return(int x) -> decltype(x * 2) {
    return x * 2;
}
#endif
