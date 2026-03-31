/* test_tree_codes.c - Comprehensive test for GCC internal tree node generation */

/* Enable GNU extensions and OpenMP */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int builtin_choose_expr_test(int x) {
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
#define TYPE_SAFE_ADD(a, b) \
    __builtin_choose_expr( \
        __builtin_types_compatible_p(typeof(a), typeof(b)), \
        (a) + (b), \
        (__builtin_types_compatible_p(typeof(a), int) ? (int)(a) + (int)(b) : 0) \
    )

/* C11 _Generic selector (creates distinct tree node) */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Function with unusual attributes */
int __attribute__((optimize("O0"))) 
__attribute__((noinline))
attribute_test(int x) {
    /* Case range creates specific internal representation */
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return x;
    }
}

/* OpenMP test function */
void omp_test(int n) {
    int sum = 0;
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    /* Nested OpenMP with teams - may generate additional tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function using statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    __builtin_constant_p(_x) ? \
        __builtin_choose_expr(_x > 0, _x * 2, _x / 2) : \
        ({ volatile typeof(x) _tmp = _x; _tmp + 1; }); \
})

/* Test function with void parameter (for potential error paths) */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Test builtin choose expr */
    result += builtin_choose_expr_test(10);
    result += builtin_choose_expr_test(argc);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test type-safe add macro */
    result += TYPE_SAFE_ADD(5, 10);
    result += TYPE_SAFE_ADD(5.0, 10);  // Mixed types
    
    /* Test _Generic selector */
    printf("Type: %s\n", type_to_string(result));
    printf("Type: %s\n", type_to_string(3.14));
    
    /* Test attribute function */
    result += attribute_test(3);
    result += attribute_test(7);
    
    /* Test OpenMP */
    omp_test(100);
    
    /* Test complex expression macro */
    result += COMPLEX_EXPR(20);
    result += COMPLEX_EXPR(-5);
    
    /* Test statement expression as function argument */
    printf("Result: %d\n", ({ int x = result; x % 100; }));
    
    /* Potential error path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger
       internal compiler paths during parsing */
    void_func(42);  // Wrong number of arguments
    #endif
    
    /* Use qualifiers in array parameters (C99 feature) */
    void vla_test(int n, int arr[restrict static n]) {
        for (int i = 0; i < n; i++) {
            result += arr[i];
        }
    }
    
    /* Test VLA with restrict */
    int n = 5;
    int vla_arr[n];
    for (int i = 0; i < n; i++) vla_arr[i] = i;
    vla_test(n, vla_arr);
    
    /* Additional OpenMP constructs */
    #pragma omp parallel sections
    {
        #pragma omp section
        { result += 1; }
        
        #pragma omp section
        { result += 2; }
    }
    
    /* OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop copy(result)
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    #endif
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
