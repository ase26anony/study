/* tree_coverage_test.c - Comprehensive test to trigger various GCC tree nodes */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions for complex tree node generation */
#define CHOOSE_EXPR(cond, x, y) __builtin_choose_expr(cond, x, y)
#define IS_CONSTANT(x) __builtin_constant_p(x)
#define TYPES_COMPATIBLE(t1, t2) __builtin_types_compatible_p(t1, t2)

/* Helper using statement expressions in constant context */
static inline int stmt_expr_helper(int x) {
    return ({ 
        int y = x * 2; 
        y + 1; 
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
void process_array(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using __builtin_choose_expr with type-dependent expressions */
int type_dependent_expr(int x) {
    /* This creates comparison nodes with uncommon tree codes */
    return CHOOSE_EXPR(
        TYPES_COMPATIBLE(typeof(x), int),
        x * 2,
        ({ double d = x; (int)(d * 3.14); })
    );
}

/* Function with switch case ranges (parsed into specific internal representation) */
int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        case 11 ... 20:  /* Multiple ranges */
            return val * 30;
        default:
            return -1;
    }
}

/* Function using _Generic selector (C11, creates distinct tree node) */
int generic_selector(int x) {
    return _Generic(x,
        int: x * 2,
        float: (int)(x * 3.0f),
        double: (int)(x * 4.0),
        default: -1
    );
}

/* Function with various attributes (may create annotated nodes) */
int __attribute__((always_inline)) 
__attribute__((warn_unused_result))
inline_attributed(int x) {
    return x + 42;
}

/* OpenMP helper function */
void omp_computation(int n, int *result) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    *result = sum;
}

/* Function that might trigger internal error recovery paths */
#ifdef TEST_ERROR_PATH
void problematic_call(void) {
    void foo(void);
    /* This call with argument to void function might trigger 
       special error recovery tree nodes */
    foo(1);  /* Intentional type error */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    int arr[10] = {0};
    
    /* 1. Use statement expressions */
    result = stmt_expr_helper(5);
    printf("Statement expr result: %d\n", result);
    
    /* 2. Process array with static parameter */
    process_array(arr);
    printf("Array processed\n");
    
    /* 3. Type-dependent expression */
    result = type_dependent_expr(7);
    printf("Type-dependent result: %d\n", result);
    
    /* 4. Switch with ranges */
    result = switch_with_ranges(3);
    printf("Switch range result: %d\n", result);
    
    /* 5. _Generic selector */
    result = generic_selector(10);
    printf("Generic selector result: %d\n", result);
    
    /* 6. Attributed function */
    result = inline_attributed(20);
    printf("Attributed function result: %d\n", result);
    
    /* 7. OpenMP computation */
    int omp_result = 0;
    omp_computation(100, &omp_result);
    printf("OpenMP result: %d\n", omp_result);
    
    /* 8. Use __builtin_constant_p in complex expression */
    int constant_test = CHOOSE_EXPR(
        IS_CONSTANT(42),
        ({ int z = 100; z * 2; }),
        999
    );
    printf("Constant test result: %d\n", constant_test);
    
    /* 9. OpenMP target directive (may create distinct tree nodes) */
    #pragma omp target teams distribute parallel for map(tofrom: arr)
    for (int i = 0; i < 10; i++) {
        arr[i] += i;
    }
    printf("OpenMP target completed\n");
    
    /* 10. Complex expression with multiple builtins */
    int complex_expr = ({
        int x = 5;
        int y = CHOOSE_EXPR(
            TYPES_COMPATIBLE(typeof(x), int) && IS_CONSTANT(x),
            x * 3,
            x * 4
        );
        y + generic_selector(x);
    });
    printf("Complex expression result: %d\n", complex_expr);
    
    printf("All tests completed successfully\n");
    return 0;
}
