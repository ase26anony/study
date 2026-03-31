/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wall tree_coverage_test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int builtin_choose_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int statement_expr_test(void) {
    return ({
        int a = 5;
        int b = 10;
        __builtin_types_compatible_p(int, typeof(a)) ? a : b;
    });
}

/* Function using __builtin_types_compatible_p in type-dependent expression */
static int types_compatible_test(void) {
    int x = 0;
    /* This may generate specialized comparison nodes */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        x = __builtin_types_compatible_p(typeof(&x), int*) ? 1 : 2;
    }
    return x;
}

/* Function with attribute in unusual place */
static void attribute_test(void) {
    int i __attribute__((unused)) = 0;
    volatile int j __attribute__((unused)) = 1;
    
    /* Array parameter with static qualifier - creates specialized tree nodes */
    auto void local_func(int arr[static 10]);
    /* Forward declaration only - won't be called */
}

/* Function using _Generic selection (C11) */
static int generic_selector_test(int x) {
    return _Generic(x,
        int: 1,
        long: 2,
        default: 3
    );
}

/* Function with switch case ranges */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return 10;
        case 6 ... 10:
            return 20;
        default:
            return 30;
    }
}

/* Function that would cause error recovery if called incorrectly */
static void void_param_func(void) {
    /* Empty - void parameter */
}

/* OpenMP test function - generates OMP nodes */
static void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* Additional OpenMP construct */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 50; i++) {
        /* Simple computation */
        sum += i * 2;
    }
}

/* Function using complex GNU extensions together */
static int mixed_extensions_test(int x) {
    /* Combine multiple extensions */
    int result = __builtin_choose_expr(
        x > 0,
        _Generic(x, int: x * 2, default: x),
        ({
            int y = x;
            while (__builtin_types_compatible_p(typeof(y), int) && y < 10) {
                y++;
            }
            y;
        })
    );
    
    return result;
}

/* Main function combining all tests */
int main(void) {
    int result = 0;
    
    printf("Starting tree coverage test...\n");
    
    /* Test 1: Builtin choose with statement expressions */
    result += builtin_choose_test(10);
    printf("Test 1 result: %d\n", result);
    
    /* Test 2: Statement expression test */
    result += statement_expr_test();
    printf("Test 2 result: %d\n", result);
    
    /* Test 3: Types compatible test */
    result += types_compatible_test();
    printf("Test 3 result: %d\n", result);
    
    /* Test 4: Attribute test */
    attribute_test();
    
    /* Test 5: Generic selector */
    result += generic_selector_test(result);
    printf("Test 5 result: %d\n", result);
    
    /* Test 6: Case range test */
    result += case_range_test(result % 10);
    printf("Test 6 result: %d\n", result);
    
    /* Test 7: Mixed extensions */
    result += mixed_extensions_test(result);
    printf("Test 7 result: %d\n", result);
    
    /* Test 8: OpenMP constructs */
    omp_test();
    
    /* Optional error path - guarded by define */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger special tree nodes */
    void_param_func(1);  /* Calling void function with argument */
    #endif
    
    /* Use all results to prevent dead code elimination */
    volatile int final_result = result;
    
    printf("Final accumulated result: %d\n", final_result);
    printf("Test complete.\n");
    
    return 0;
}

/* Additional C++ specific test (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

/* C++ specific constructs that might generate unique tree nodes */
template<typename T>
auto trailing_return_test(T x) -> decltype(x * 2) {
    return x * 2;
}

void cpp_specific_tests(void) {
    /* nullptr - C++ specific */
    int* ptr = nullptr;
    
    /* User-defined literal (C++11) */
    auto val = 123_ull;
    
    /* Template with trailing return type */
    auto result = trailing_return_test(5.0);
    
    /* Lambda expression */
    auto lambda = [](int x) { return x * x; };
    lambda(10);
}
#endif
