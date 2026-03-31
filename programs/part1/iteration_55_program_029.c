/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */

/* Enable GNU extensions and OpenMP */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int builtin_choose_expr_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static int statement_expr_func(int a) {
    return ({
        int temp = a;
        for (int i = 0; i < 3; i++) temp += i;
        temp;
    });
}

/* Function using __builtin_types_compatible_p in complex expression */
static size_t type_compatible_test(void *ptr) {
    return __builtin_types_compatible_p(typeof(ptr), void*) 
           ? sizeof(* (void**)ptr) 
           : __builtin_types_compatible_p(typeof(ptr), int*) 
             ? sizeof(int) 
             : 0;
}

/* Function with array parameter using 'static' index (C99 feature) */
static int static_array_index(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic (C11 feature) */
static const char* generic_selector_test(_Bool b) {
    return _Generic(b,
        _Bool: "boolean",
        default: "other"
    );
}

/* Function with switch case ranges (GNU extension) */
static int switch_case_ranges(int val) {
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
attributed_var_test(void) {
    int i __attribute__((unused)) = 42;
    int j __attribute__((aligned(16))) = 24;
    return i + j;
}

/* OpenMP helper function */
static void openmp_test(int n, int *result) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < n; i++) {
        sum -= i;
    }
    
    *result = sum;
}

/* Function that might trigger error recovery paths if enabled */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    /* This is syntactically valid but semantically wrong */
    void foo(void);
    
    /* The compiler will complain but may generate special nodes */
    foo(1);  /* Calling void function with argument */
    
    /* Another dubious construct */
    int arr[5] = { [6] = 10 };  /* Designator out of range */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expression */
    result += builtin_choose_expr_test(10);
    
    /* Test statement expression function */
    result += statement_expr_func(5);
    
    /* Test type compatible expressions */
    int x = 42;
    result += type_compatible_test(&x);
    
    /* Test static array index function */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_index(arr);
    
    /* Test _Generic selector */
    const char *str = generic_selector_test(1);
    result += (int)str[0];  /* Use result to prevent optimization */
    
    /* Test switch with case ranges */
    result += switch_case_ranges(3);
    
    /* Test attributed variables */
    result += attributed_var_test();
    
    /* Test OpenMP constructs */
    int omp_result = 0;
    openmp_test(100, &omp_result);
    result += omp_result;
    
    /* Potentially test error paths */
    #ifdef TEST_ERROR_PATH
    error_path_test();
    #endif
    
    /* Use C11 nullptr if available */
    #if __STDC_VERSION__ >= 201112L
    void *ptr = NULL;
    result += (ptr == NULL);
    #endif
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
