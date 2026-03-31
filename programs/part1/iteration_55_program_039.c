/* test_tree_codes.c - Comprehensive test for GCC tree node coverage */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
static inline int use_builtin_choose(void) {
    /* __builtin_choose_expr creates COND_EXPR nodes */
    return __builtin_choose_expr(1, 42, 0);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    __builtin_constant_p(_x) ? _x * 2 : _x + 1; \
})

/* Type-dependent expression with __builtin_types_compatible_p */
static int type_dependent_expr(int x) {
    return __builtin_types_compatible_p(typeof(x), int) ? x : 0;
}

/* C11 _Generic selector - creates GENERIC_SELECTOR tree node */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Function with static array parameter - creates specialized tree */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += arr[i];
    return sum;
}

/* Function with case ranges - creates CASE_LABEL_EXPR with range */
static int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:   return 1;
        case 6 ... 10:  return 2;
        default:        return 0;
    }
}

/* Attribute in unusual place */
static int __attribute__((unused)) unused_var = 0;

/* OpenMP helper that might generate various OMP nodes */
static void omp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        /* Empty - just for tree generation */
    }
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    void foo(void);  /* Declaration with void parameter */
    /* This would normally be an error but might generate special nodes */
    /* foo(1); */  /* Uncomment to test error recovery */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += use_builtin_choose();
    result += COMPLEX_EXPR(10);
    result += type_dependent_expr(20);
    
    /* Test _Generic selector */
    int x = 5;
    const char *type = type_name(x);
    (void)type;  /* Suppress unused warning */
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    
    /* Test OpenMP constructs */
    omp_test(100);
    
    /* Use attribute syntax */
    int i __attribute__((unused)) = result;
    
    /* Statement expression as function argument */
    printf("Result: %d\n", ({ int y = result; y * 2; }));
    
    /* Complex constant expression with __builtin_constant_p */
    if (__builtin_constant_p(1 + 2)) {
        result += 100;
    }
    
    /* Nested statement expressions */
    int z = ({
        int a = 5;
        int b = ({
            int c = 3;
            c * 2;
        });
        a + b;
    });
    result += z;
    
    /* OpenACC pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int j = 0; j < 10; j++) {
        result += j;
    }
    #endif
    
    printf("Final: %d\n", result);
    return 0;
}

/* Additional C++ specific constructs (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

/* User-defined literal */
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

/* Template with trailing return type */
template<typename T, typename U>
auto template_func(T t, U u) -> decltype(t + u) {
    return t + u;
}

/* nullptr in template context */
template<typename T>
void null_test(T* ptr = nullptr) {
    if (ptr) *ptr = T();
}

/* Lambda expression */
auto lambda = [](int x) { return x * 2; };
#endif
