/* Compile with: gcc -O2 -fopenmp -std=gnu11 -fdump-tree-all -c test_tree_codes.c */
/* Additional flags to try: -std=gnu++17 -Wall -Wextra -Werror */

#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define COMPLEX_CHOOSE(x) __builtin_choose_expr(__builtin_constant_p(x), (x)*2, (x))
#define TYPE_COMPATIBLE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

/* Statement expression creating complex tree structure */
#define STMT_EXPR(val) ({ \
    typeof(val) __tmp = (val); \
    __builtin_constant_p(__tmp) ? __tmp : __tmp + 1; \
})

/* Function with static array parameter (creates specialized tree nodes) */
static int process_array(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_choose_expr in type-dependent context */
static inline int choose_based_on_type(int x, double y) {
    return __builtin_choose_expr(
        TYPE_COMPATIBLE(x, int),
        x + (int)y,
        (int)(x * y)
    );
}

/* Function with attribute in unusual place */
static void __attribute__((always_inline)) 
inline_func_with_attrs(int __attribute__((unused)) param) {
    volatile int local __attribute__((unused)) = 0;
}

/* Use _Generic selection expression (C11) */
#define TYPE_ID(x) _Generic((x), \
    int: 1, \
    double: 2, \
    float: 3, \
    default: 0 \
)

/* OpenMP helper function */
static void omp_helper(int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
}

/* Function using case ranges (GNU extension) */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x + 5;
        default:
            return -1;
    }
}

/* Function that might trigger internal error recovery paths */
#ifdef TEST_ERROR_PATH
static void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    /* Use GNU extensions */
    int a = 42;
    double b = 3.14;
    
    /* Complex expression using statement expression */
    int result1 = STMT_EXPR(a);
    printf("Result1: %d\n", result1);
    
    /* __builtin_choose_expr with constant condition */
    int result2 = COMPLEX_CHOOSE(10);
    printf("Result2: %d\n", result2);
    
    /* Type-based selection */
    int result3 = choose_based_on_type(a, b);
    printf("Result3: %d\n", result3);
    
    /* _Generic selector */
    int type_id = TYPE_ID(a);
    printf("Type ID: %d\n", type_id);
    
    /* Array with static index */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    int sum = process_array(arr);
    printf("Array sum: %d\n", sum);
    
    /* Case range test */
    int range_result = case_range_test(3);
    printf("Range test: %d\n", range_result);
    
    /* Inline function with attributes */
    inline_func_with_attrs(100);
    
    /* OpenMP region */
    omp_helper(100);
    
    /* Additional OpenMP construct - target teams */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    printf("After OpenMP target: %d\n", sum);
    
    /* Try to create unusual tree nodes with nested builtins */
    int complex_expr = __builtin_constant_p(
        __builtin_types_compatible_p(int, typeof(a))
    ) ? 1 : 0;
    
    /* Use attribute on variable in middle of statement */
    int x = 1, y __attribute__((unused)) = 2, z = 3;
    printf("x=%d, z=%d\n", x, z);
    
    /* Potential error path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger
       internal compiler paths during error recovery */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        arr[i] *= 2;
    }
    #endif
    
    printf("Done\n");
    return 0;
}

/* Additional function using C++-like features if compiled as C++ */
#ifdef __cplusplus
/* User-defined literal (C++11) */
constexpr unsigned long long operator"" _custom(unsigned long long n) {
    return n * 2;
}

/* Template with trailing return type */
template<typename T, typename U>
auto template_func(T t, U u) -> decltype(t + u) {
    return t + u;
}
#endif
