/* test_tree_codes.c - Complex program to trigger various GCC internal tree nodes */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int choose_expr_helper(int x) {
    /* This creates complex internal tree nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ 
            int y = x * 2; 
            y + 1; 
        }),
        ({ 
            volatile int z = x; 
            z - 1; 
        })
    );
}

/* Function with GNU C extensions in parameter */
static void func_with_attrs(int arr[static 10]) __attribute__((always_inline));
static void func_with_attrs(int arr[static 10]) {
    /* Use case ranges - creates special tree nodes */
    switch (arr[0]) {
        case 1 ... 5:
            arr[0] = choose_expr_helper(arr[0]);
            break;
        case 6 ... 10:
            /* Use __builtin_types_compatible_p in expression */
            arr[0] = sizeof(int[__builtin_types_compatible_p(typeof(arr[0]), int) ? 10 : 5]);
            break;
        default:
            /* _Generic creates distinct tree nodes */
            arr[0] = _Generic(arr[0],
                int: 42,
                short: 24,
                default: 0
            );
    }
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* OpenMP helper with complex clauses */
static void omp_helper(int n) {
    int i, sum = 0;
    
    /* OpenMP parallel region - creates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum) private(i) \
        if(n > 100) num_threads(4)
    for (i = 0; i < n; i++) {
        /* Nested statement expression */
        sum += ({ 
            int tmp = i * i; 
            tmp % 100; 
        });
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive - may create distinct tree nodes */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) if(target: n > 50)
    for (i = 0; i < n; i++) {
        sum += i;
    }
}

/* Function using __builtin_constant_p in type context */
static int constant_dependent(void) {
    /* Complex expression that creates specialized tree nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(__TIME__[0]),
        sizeof(int[10]),
        sizeof(int[5])
    ) + __builtin_types_compatible_p(int, const int);
}

/* Main function combining all patterns */
int main(void) {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int result;
    
    /* Test various GNU extensions */
    func_with_attrs(arr);
    
    /* Test OpenMP constructs */
    omp_helper(100);
    
    /* Test constant-dependent expressions */
    result = constant_dependent();
    
    /* Use _Generic with complex expressions */
    int generic_result = _Generic(result,
        int: choose_expr_helper(result),
        long: result * 2,
        default: ({ 
            int x = result; 
            x * x; 
        })
    );
    
    /* Statement expression as function argument */
    printf("Result: %d\n", 
        ({ 
            int tmp = generic_result; 
            tmp + arr[0]; 
        })
    );
    
    #ifdef TEST_ERROR_PATH
    /* This should generate error recovery tree nodes */
    void_func(42);  /* Wrong number of arguments */
    #endif
    
    /* Array with static index in parameter (GNU extension) */
    void process_array(int array[static 5]);
    
    /* Use attribute in unusual place */
    int i __attribute__((unused)) = 
        __builtin_choose_expr(1, 42, 0);
    
    /* OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int j = 0; j < 10; j++) {
        arr[j] += j;
    }
    #endif
    
    /* Complex switch with multiple case ranges */
    switch (result % 4) {
        case 0 ... 1:
            result += ({ int x = 10; x * 2; });
            break;
        case 2 ... 3:
            result = _Generic((char)result,
                char: 1,
                int: 2,
                default: 3
            );
            break;
    }
    
    printf("Final: %d\n", result);
    
    /* Ensure all code paths are reachable */
    if (__builtin_constant_p(1)) {
        printf("Constant path taken\n");
    } else {
        /* This creates different tree nodes for the else branch */
        printf("Non-constant path\n");
    }
    
    return 0;
}

/* Dummy function declaration with static array parameter */
void process_array(int array[static 5]) {
    array[0] = 0;
}

/* Additional function using C11 features */
static inline auto_type_demo(void) {
    /* Use of auto in C (GNU extension) */
    auto int x = 42;
    
    /* Compound literal with attribute */
    int *ptr = (int []) __attribute__((aligned(16))) {1, 2, 3};
    
    return x + ptr[0];
}
