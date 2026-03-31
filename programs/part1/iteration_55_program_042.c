/* tree_code_kind_coverage.c - Test program to exercise GCC's internal tree node classification */

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
static int statement_expr_test(void) {
    return ({
        int a = 5;
        int b = 10;
        __builtin_types_compatible_p(int, typeof(a)) ? a : b;
    });
}

/* Function using __builtin_constant_p in complex way */
static int constant_p_test(int x) {
    if (__builtin_constant_p(x)) {
        return __builtin_popcount(x);
    } else {
        return ({ 
            typeof(x) y = x; 
            while (y & (y - 1)) y &= y - 1; 
            y; 
        });
    }
}

/* Test _Generic selection (C11) - creates SELECTOR tree nodes */
#define type_selector(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with attribute in unusual place */
static int attribute_test(int x __attribute__((unused))) {
    return 42;
}

/* Function with static array index qualifier */
static int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with case ranges */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return -1;
    }
}

/* OpenMP test function */
static void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenACC-like pragma (might create different nodes) */
    #pragma acc parallel loop
    for (int i = 0; i < 50; i++) {
        /* dummy computation */
        sum += i % 2;
    }
}

/* Function with void parameter - for error path testing */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    return;
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += builtin_choose_expr_test(10);
    result += statement_expr_test();
    result += constant_p_test(7);
    
    /* Test _Generic selector */
    const char *type_str = type_selector(result);
    printf("Type string: %s\n", type_str);
    
    /* Test attributes and qualifiers */
    result += attribute_test(5);
    
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_test(arr);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(8);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test error recovery path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning but compile */
    void_func();
    /* Try to call with argument - will generate error recovery nodes */
    void_func(42);
    #endif
    
    /* Use __builtin_choose_expr in constant context */
    int complex_expr = __builtin_choose_expr(
        1,
        ({
            int x = 5;
            int y = 10;
            __builtin_types_compatible_p(typeof(x), int) ? x + y : x - y;
        }),
        0
    );
    result += complex_expr;
    
    /* Mix with __builtin_constant_p */
    if (__builtin_constant_p(complex_expr)) {
        result += __builtin_ffs(complex_expr);
    }
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific test (compile with g++) */
#ifdef __cplusplus
// User-defined literal
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

// Template with trailing return type
template<typename T, typename U>
auto template_test(T t, U u) -> decltype(t + u) {
    return t + u;
}

// nullptr usage in template context
template<typename T>
T* null_test() {
    return nullptr;
}
#endif
