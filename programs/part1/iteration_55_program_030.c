/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test tree_coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int builtin_chooser(int x) {
    /* This creates complex expression trees */
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

/* Function using __builtin_types_compatible_p in type-dependent expression */
static size_t type_comparison(void *ptr) {
    return __builtin_types_compatible_p(typeof(*ptr), int) ? 
           sizeof(int) : sizeof(long);
}

/* Function with static array index parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += arr[i];
    return sum;
}

/* Function with switch case ranges (parsed into specific internal representation) */
static int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        case 100 ... 200:
            return val + 100;
        default:
            return -1;
    }
}

/* Function using _Generic selection (creates distinct tree node) */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Function with attributes in unusual places */
static void attributed_vars(void) {
    int i __attribute__((unused)) = 0;
    int j __attribute__((aligned(16))) = 1;
    volatile int k __attribute__((deprecated)) = 2;
    
    /* Use them to prevent elimination */
    (void)i;
    (void)j;
    (void)k;
}

/* OpenMP function to generate OpenMP tree nodes */
static void openmp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths if enabled */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    /* Declare function with void parameter */
    void foo(void);
    
    /* This should cause an error but might generate special tree nodes */
    /* during error recovery */
    foo(1);  /* Wrong number of arguments */
    
    /* Another potential error case */
    int arr[5];
    arr[10] = 0;  /* Out of bounds */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin chooser */
    result += builtin_chooser(10);
    result += builtin_chooser(result);
    
    /* Test statement expressions */
    result += statement_expr_func(5);
    
    /* Test type comparisons */
    int x = 42;
    result += type_comparison(&x);
    
    /* Test static array parameter */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += static_array_param(arr);
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    result += switch_with_ranges(7);
    result += switch_with_ranges(150);
    
    /* Test _Generic */
    printf("Type of 42: %s\n", get_type_name(42));
    printf("Type of 3.14: %s\n", get_type_name(3.14));
    
    /* Test attributed variables */
    attributed_vars();
    
    /* Test OpenMP */
    openmp_test(100);
    
    /* Complex expression using multiple GNU extensions */
    int complex_expr = ({
        int a = 10;
        int b = __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(a), int),
            a * 2,
            a / 2
        );
        b + __builtin_constant_p(a) ? 5 : 10;
    });
    
    result += complex_expr;
    
    /* Use __builtin_constant_p in control flow */
    if (__builtin_constant_p(result)) {
        printf("Result is constant: %d\n", result);
    } else {
        printf("Result is not constant: %d\n", result);
    }
    
    /* Additional OpenMP construct for more tree node variety */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            result += 1;
        }
        #pragma omp section
        {
            result += 2;
        }
    }
    
    /* Try to create some unusual tree nodes with computed goto */
    void *labels[] = { &&label1, &&label2 };
    int choice = result % 2;
    
    goto *labels[choice];
    
label1:
    result += 100;
    goto end;
    
label2:
    result += 200;
    goto end;
    
end:
    printf("Final result: %d\n", result);
    
    /* Optional error path testing */
    #ifdef TEST_ERROR_PATH
    error_path_test();
    #endif
    
    return 0;
}

/* Additional C++ specific code if compiled as C++ */
#ifdef __cplusplus
#include <type_traits>

/* Template with trailing return type */
template<typename T>
auto template_func(T t) -> decltype(t + 1) {
    return t + 1;
}

/* User-defined literal */
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

/* nullptr usage in template context */
template<typename T>
void process_pointer(T* ptr) {
    if (ptr == nullptr) {
        // Handle null
    }
}
#endif
