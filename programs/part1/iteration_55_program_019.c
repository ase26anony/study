/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 tree_coverage_test.c -o test */

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

/* Function with GNU statement expression in return */
static inline int stmt_expr_func(int a) {
    return ({
        int tmp = a;
        while (tmp > 0) tmp--;
        tmp + 42;
    });
}

/* Function using __builtin_types_compatible_p in type-dependent expression */
static size_t type_compatible_test(void *ptr) {
    /* Creates comparison nodes with potentially uncommon tree codes */
    if (__builtin_types_compatible_p(__typeof__(ptr), void*))
        return sizeof(void*);
    return __builtin_types_compatible_p(__typeof__(ptr), int*) ? sizeof(int) : 0;
}

/* Function with attribute in unusual place */
static int attributed_var(void) __attribute__((warn_unused_result));
static int attributed_var(void) {
    int result __attribute__((unused)) = 42;
    return result;
}

/* Function with static array index in parameter (C99 feature) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
static const char* generic_selector(int x) {
    return _Generic(x,
        int: "int",
        float: "float",
        double: "double",
        default: "unknown"
    );
}

/* Function with case ranges (GNU extension) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 10 ... 20:
            return val / 2;
        default:
            return -1;
    }
}

/* OpenMP helper function */
static void omp_helper(int *data, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        data[i] = i * i;
    }
}

/* Function with OpenMP target directive */
static void omp_target_test(void) {
    int n = 100;
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    free(data);
}

/* Function with OpenACC-like pragma (might create distinct nodes) */
static void acc_style_pragma(void) {
    int i;
    /* Some compilers might treat this specially even without -fopenacc */
    #pragma acc parallel loop
    for (i = 0; i < 10; i++) {
        /* dummy loop */
    }
}

/* Template-like macro that creates complex expressions */
#define COMPLEX_EXPR(x) \
    ({ \
        __typeof__(x) _x = (x); \
        __builtin_choose_expr( \
            __builtin_constant_p(_x), \
            (_x * _x) + 1, \
            (_x / 2) | 0xFF \
        ); \
    })

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += choose_expr_test(10);
    result += stmt_expr_func(5);
    
    /* Test type compatibility expressions */
    int x = 42;
    result += type_compatible_test(&x);
    
    /* Test attributed function */
    result += attributed_var();
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    const char *type_str = generic_selector(result);
    printf("Type string: %s\n", type_str);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(15);
    
    /* Test complex macro expression */
    result += COMPLEX_EXPR(result);
    
    /* Test OpenMP constructs */
    int n = 50;
    int *omp_data = (int*)malloc(n * sizeof(int));
    omp_helper(omp_data, n);
    
    for (int i = 0; i < n; i++) {
        result += omp_data[i];
    }
    free(omp_data);
    
    /* Test OpenMP target (if supported) */
    #ifdef _OPENMP
    omp_target_test();
    #endif
    
    /* Test ACC-style pragma */
    acc_style_pragma();
    
    /* Potentially problematic code for error recovery paths */
    #ifdef TEST_ERROR_PATH
    /* This function takes void but we try to call it with argument */
    void void_func(void);
    /* The next line would cause an error, but error recovery might generate special nodes */
    /* void_func(1); */  /* Commented out to keep code compilable */
    
    /* Switch with duplicate cases (semantically invalid but syntactically valid) */
    switch (result) {
        case 1:
            break;
        case 1:  /* Duplicate case */
            break;
    }
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional function with trailing return type style (C++-like in C) */
#ifdef __cplusplus
/* For C++ compilation, add C++ specific constructs */
#include <type_traits>

template<typename T>
auto cpp_style_func(T t) -> decltype(t + 1) {
    return t + 1;
}

void test_cpp_specific(void) {
    // nullptr and user-defined literal (C++11)
    int* ptr = nullptr;
    // User-defined literal syntax might create special nodes
    // auto x = 123_km;  // Would require literal operator
}
#endif
