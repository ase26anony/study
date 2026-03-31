/* test_tree_codes.c - Comprehensive test for GCC internal tree codes */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(a, b)
#define IS_CONSTANT(e) __builtin_constant_p(e)

/* Statement expression with complex type */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        typeof(y) z = y + 1;
        z;
    });
}

/* Function with static array parameter (C99 feature) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using __builtin_choose_expr in constant context */
static int builtin_choose_test(void) {
    return CHOOSE_EXPR(1, 42, 0);
}

/* Type-dependent expression with __builtin_types_compatible_p */
static int type_compat_test(int x) {
    return TYPE_COMPAT(typeof(x), int) ? x : 0;
}

/* Test case ranges in switch (GNU extension) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        default:
            return 0;
    }
}

/* Function with void parameter - will generate error if called with arg */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* _Generic selector (C11) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Attribute in unusual place */
int __attribute__((unused)) unused_var = 0;

/* Complex declaration with attributes */
static int __attribute__((always_inline)) inline_attr(int x) 
    __attribute__((const));

static int inline_attr(int x) {
    return x * x;
}

/* OpenMP test function */
static void omp_test(void) {
    int sum = 0;
    int array[100];
    
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target teams distribute - may generate different nodes */
    #pragma omp target teams distribute parallel for map(tofrom:array)
    for (int i = 0; i < 100; i++) {
        array[i] *= 2;
    }
}

/* OpenACC pragma (if supported) */
#ifdef _OPENACC
static void acc_test(void) {
    int data[50];
    
    #pragma acc parallel loop copy(data)
    for (int i = 0; i < 50; i++) {
        data[i] = i * 3;
    }
}
#endif

/* Function with trailing return type (C++ style in GNU C) */
#ifdef __cplusplus
auto trailing_return(int x) -> decltype(x * 2) {
    return x * 2;
}
#endif

/* User-defined literal simulation in C */
#ifdef __cplusplus
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180.0L;
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(5);
    printf("Stmt expr result: %d\n", result);
    
    /* Test builtin choose */
    result += builtin_choose_test();
    
    /* Test type compatibility */
    result += type_compat_test(10);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(8);
    
    /* Test _Generic selector */
    int x = 42;
    double y = 3.14;
    printf("Type of x: %s\n", type_name(x));
    printf("Type of y: %s\n", type_name(y));
    
    /* Test inline function with attribute */
    result += inline_attr(7);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    
    /* Test constant expression with __builtin_constant_p */
    if (IS_CONSTANT(42)) {
        result += 100;
    }
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test OpenACC if available */
    #ifdef _OPENACC
    acc_test();
    #endif
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate a compilation error but may create
       interesting tree nodes during error recovery */
    void_func(1);  /* Wrong number of arguments */
    #endif
    
    /* C++ specific tests if compiling as C++ */
    #ifdef __cplusplus
    result += trailing_return(21);
    double radians = 90.0_deg;
    printf("90 degrees = %f radians\n", radians);
    #endif
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
