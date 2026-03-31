/* tree_coverage_test.c - Test program to exercise GCC's tree_code_kind default case */

/* Enable GNU extensions and OpenMP */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x * 2, x / 2);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    __builtin_constant_p(_x) ? _x * 3 : _x + 1; \
})

/* Type-dependent expression using __builtin_types_compatible_p */
static int type_compatible_test(void) {
    int a = 5;
    double b = 3.14;
    
    /* Creates comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return __builtin_types_compatible_p(typeof(b), double) ? 1 : 0;
    }
    return 0;
}

/* Function with static array parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
static int generic_selector_test(void) {
    int i = 5;
    double d = 3.14;
    const char *s = "test";
    
    /* _Generic creates distinct tree node type */
    printf("Integer: %d\n", _Generic(i, 
        int: i * 2,
        double: (int)(d * 2),
        default: 0));
    
    return _Generic(s, char*: 1, const char*: 2, default: 3);
}

/* Function with switch case ranges */
static int case_range_test(int x) {
    /* Case ranges create specific internal representation */
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        case 11 ... 20:
            return x + 1;
        default:
            return -1;
    }
}

/* Function with various attributes in unusual places */
static void attribute_test(void) {
    /* Variable with unused attribute */
    int i __attribute__((unused)) = 0;
    
    /* Function attribute on variable declaration */
    int (*fp)(void) __attribute__((noreturn)) = NULL;
    
    /* Type attribute */
    typedef int my_int __attribute__((aligned(16)));
    my_int aligned_var = 42;
    
    (void)aligned_var;
}

/* OpenMP test function - generates OMP_* tree nodes */
static void omp_test(void) {
    int i;
    int sum = 0;
    int arr[100];
    
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target teams distribute - may generate additional OMP nodes */
    #pragma omp target teams distribute parallel for map(tofrom:arr)
    for (i = 0; i < 100; i++) {
        arr[i] *= 2;
    }
}

/* Function with void parameter - for error path testing */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result;
    int test_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test builtin choose expression */
    result = builtin_choose_test(5);
    printf("Builtin choose: %d\n", result);
    
    /* Test statement expression macro */
    result = COMPLEX_EXPR(7);
    printf("Complex expr: %d\n", result);
    
    /* Test type compatibility */
    result = type_compatible_test();
    printf("Type compatible: %d\n", result);
    
    /* Test static array parameter */
    result = static_array_param(test_array);
    printf("Static array sum: %d\n", result);
    
    /* Test _Generic selector */
    result = generic_selector_test();
    printf("Generic selector: %d\n", result);
    
    /* Test case ranges */
    result = case_range_test(3);
    printf("Case range (3): %d\n", result);
    result = case_range_test(15);
    printf("Case range (15): %d\n", result);
    
    /* Test attributes */
    attribute_test();
    
    /* Test OpenMP */
    omp_test();
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning but compile */
    void_func(1);  /* Calling void function with argument */
    #endif
    
    /* Use __builtin_constant_p in a complex expression */
    int x = 10;
    if (__builtin_constant_p(x) ? 0 : 1) {
        x = __builtin_abs(x);
    }
    
    printf("Final x: %d\n", x);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific code if compiled as C++ */
#ifdef __cplusplus
#include <type_traits>

/* C++ nullptr and user-defined literal */
constexpr int operator"" _custom(unsigned long long n) {
    return static_cast<int>(n * 2);
}

/* Template with trailing return type */
template<typename T, typename U>
auto template_test(T t, U u) -> decltype(t + u) {
    return t + u;
}

/* Use in a separate function to ensure instantiation */
static void cpp_specific_test(void) {
    int n = 42_custom;
    auto result = template_test(3.14, 2);
    (void)result;
    (void)n;
}
#endif
