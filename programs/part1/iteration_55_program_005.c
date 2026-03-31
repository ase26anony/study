/* tree_coverage.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define IS_CONSTANT(expr) __builtin_constant_p(expr)
#define TYPES_COMPATIBLE(t1, t2) __builtin_types_compatible_p(t1, t2)

/* Statement expression with complex tree structure */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        int z;
        if (IS_CONSTANT(x))
            z = y + 1;
        else
            z = y - 1;
        CHOOSE_EXPR(z > 0, z, -z);
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
const char* type_name(void* ptr) {
    return _Generic(ptr,
        int*: "int pointer",
        float*: "float pointer",
        char*: "char pointer",
        default: "unknown pointer"
    );
}

/* Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_func(int x) 
    __attribute__((const));

int inline_func(int x) {
    return x * __builtin_abs(x);
}

/* C++ style nullptr simulation for C */
#ifndef __cplusplus
#define nullptr ((void*)0)
#endif

/* OpenMP test function */
void omp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        sum += i * i;
    }
    
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < n; ++i) {
        sum += i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function with switch case ranges */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        case 11 ... 20:
            return x + 100;
        default:
            return -x;
    }
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* User-defined literal simulation using compound literals */
#define MAKE_ARRAY(...) (int[]){__VA_ARGS__}

/* Main function combining all constructs */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(5);
    result += stmt_expr_test(-3);
    
    /* Test static array parameter */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    int x = 42;
    float y = 3.14f;
    const char* name1 = type_name(&x);
    const char* name2 = type_name(&y);
    printf("Type names: %s, %s\n", name1, name2);
    
    /* Test inline function with attributes */
    result += inline_func(7);
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    result += switch_with_ranges(8);
    result += switch_with_ranges(15);
    
    /* Test compound literals (user-defined literal simulation) */
    int* dynamic_arr = MAKE_ARRAY(1, 2, 3, 4, 5);
    for (int i = 0; i < 5; ++i) {
        result += dynamic_arr[i];
    }
    
    /* Test type compatibility checks */
    if (TYPES_COMPATIBLE(int, signed int)) {
        result += 100;
    }
    
    if (!TYPES_COMPATIBLE(int*, void*)) {
        result += 200;
    }
    
    /* Test builtin choose with constant condition */
    int chosen = CHOOSE_EXPR(1, 42, 99);  /* Always chooses 42 */
    result += chosen;
    
    /* Test OpenMP constructs */
    omp_test(100);
    
    /* Potentially problematic code for error recovery */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger special tree nodes */
    void_func(123);  /* Calling void function with argument */
    #endif
    
    /* Array with static qualifier in parameter (different from static_array_param) */
    void array_static_test(int a[const static 5]) {
        for (int i = 0; i < 5; ++i) {
            result += a[i];
        }
    }
    
    int local_arr[5] = {10, 20, 30, 40, 50};
    array_static_test(local_arr);
    
    /* Use __builtin_constant_p in complex expression */
    int complex_expr = (IS_CONSTANT(argc) ? 1000 : 2000) + 
                       (TYPES_COMPATIBLE(char*, const char*) ? 3000 : 4000);
    result += complex_expr;
    
    /* Final result */
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
