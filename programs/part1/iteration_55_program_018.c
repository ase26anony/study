/* tree_coverage_test.c - Comprehensive test to trigger default case in tree_code_kind */

/* Enable GNU extensions and C11 features */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with complex constant expressions */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        __builtin_choose_expr(x > 0, x * 2, x / 2),
        ({ int y = x; y * y; })  /* Statement expression */
    );
}

/* Function using __builtin_types_compatible_p in type-dependent context */
static size_t type_compat_test(void *ptr) {
    /* Generates comparison nodes with uncommon tree codes */
    return __builtin_types_compatible_p(typeof(*ptr), int) ? 
           sizeof(int) : sizeof(long);
}

/* Function with GNU statement expressions and attributes */
static inline int __attribute__((always_inline)) 
statement_expr_test(int a) __attribute__((unused));
static inline int statement_expr_test(int a) {
    /* Complex statement expression in return */
    return ({
        int tmp = a;
        __attribute__((unused)) int unused __attribute__((unused)) = tmp * 2;
        tmp + ({ int inner = 3; inner * inner; });
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
static int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) - creates distinct tree node */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Function with switch case ranges (parsed into specific internal representation) */
static int switch_range_test(int val) {
    switch (val) {
        case 1 ... 5:   /* Case range creates specialized tree node */
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -val;
    }
}

/* OpenMP test function - generates OMP_* tree nodes */
static void omp_test_parallel(void) {
    int i;
    int sum = 0;
    int array[100];
    
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target teams distribute - may generate additional OMP nodes */
    #pragma omp target teams distribute parallel for map(tofrom: array[0:100])
    for (i = 0; i < 100; i++) {
        array[i] *= 2;
    }
}

/* Function with trailing return type (C++ style in GNU C) */
static auto trailing_return_test(int x, double y) -> typeof(x + y) {
    return x + y;
}

/* Test error recovery path - void function called with argument */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test various GNU extensions */
    result += builtin_choose_test(10);
    printf("builtin_choose_test: %d\n", result);
    
    result += statement_expr_test(5);
    printf("statement_expr_test: %d\n", result);
    
    /* Test static array parameter */
    result += static_array_test(arr);
    printf("static_array_test: %d\n", result);
    
    /* Test type compatibility */
    result += type_compat_test(&result);
    printf("type_compat_test: %d\n", result);
    
    /* Test _Generic selector */
    const char *type_str = type_to_string(result);
    printf("Type string: %s\n", type_str);
    
    /* Test switch with ranges */
    result += switch_range_test(3);
    printf("switch_range_test: %d\n", result);
    
    /* Test trailing return type */
    double dbl_result = trailing_return_test(5, 3.14);
    printf("trailing_return_test: %f\n", dbl_result);
    
    /* Test OpenMP constructs */
    omp_test_parallel();
    
    /* Test error recovery path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate an error but will be parsed */
    void_func(42);  /* Wrong number of arguments */
    #endif
    
    /* Use __builtin_constant_p in complex context */
    int dynamic_val = result;
    int computed = __builtin_constant_p(dynamic_val) ? 100 : 
                   ({ int x = dynamic_val; while (x > 0) x--; x; });
    
    printf("Computed: %d\n", computed);
    
    /* Test with attributes in unusual places */
    int __attribute__((unused)) attr_test = 
        ({ __attribute__((unused)) int inner = 42; inner; });
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Test completed successfully\n");
    
    return 0;
}

/* Additional C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

/* User-defined literal */
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

/* Template with nullptr and trailing return */
template<typename T>
auto template_test(T* ptr) -> decltype(ptr, std::declval<T>()) {
    return ptr ? *ptr : T{};
}

/* Test these in a separate function */
static void cpp_specific_tests(void) {
    auto angle = 90.0_deg;
    int* ptr = nullptr;
    auto val = template_test(ptr);
}
#endif
