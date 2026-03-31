/* test_tree_codes.c - Complex program to trigger various GCC internal tree nodes */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions for complex tree node generation */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(a, b)
#define IS_CONSTANT(expr) __builtin_constant_p(expr)

/* Statement expression creating complex tree structure */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        int z;
        if (IS_CONSTANT(x))
            z = y + 1;
        else
            z = y - 1;
        z;
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using __builtin_choose_expr in type-dependent context */
static int choose_based_on_type(int x, double y) {
    return CHOOSE_EXPR(
        TYPE_COMPAT(typeof(x), int),
        x + (int)y,
        (int)(x * y)
    );
}

/* Function with unusual attribute placement */
int __attribute__((unused)) unused_var __attribute__((aligned(16))) = 42;

/* C11 _Generic selector (creates distinct tree node) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* OpenMP helper function */
void omp_helper(int n, int *result) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    *result = sum;
}

/* Function with switch case ranges (parsed into specific internal representation) */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 10;
        case 6 ... 10:
            return x * 20;
        case 11 ... 20:
            return x * 30;
        default:
            return -1;
    }
}

/* Function using all the complex constructs */
int complex_tree_generator(int mode) {
    int result = 0;
    
    /* Use statement expression */
    result += stmt_expr_test(mode);
    
    /* Use type-based choice */
    result += choose_based_on_type(mode, 3.14);
    
    /* Use _Generic selector */
    const char *type_str = type_name(mode);
    if (type_str[0] == 'i') result += 1;
    
    /* Array with static parameter */
    int arr[10];
    static_array_param(arr);
    result += arr[mode % 10];
    
    /* Switch with ranges */
    result += switch_with_ranges(mode % 20);
    
    return result;
}

/* OpenMP target region for offloading tree nodes */
#pragma omp declare target
int target_function(int x) {
    return x * x + 1;
}
#pragma omp end declare target

/* Main function combining all patterns */
int main(void) {
    int total = 0;
    
    /* Test complex tree generator */
    for (int i = 0; i < 5; i++) {
        total += complex_tree_generator(i);
    }
    
    /* OpenMP parallel region */
    int omp_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            omp_helper(100, &omp_result);
        }
    }
    total += omp_result;
    
    /* OpenMP target teams distribute */
    int target_result = 0;
    #pragma omp target teams distribute parallel for map(tofrom: target_result)
    for (int i = 0; i < 50; i++) {
        #pragma omp atomic
        target_result += target_function(i);
    }
    total += target_result;
    
    /* Test error recovery path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would generate error recovery nodes if uncommented */
    void foo(void);
    /* foo(1); */  /* Would create error recovery tree nodes */
    #endif
    
    /* Use attribute in declaration */
    int __attribute__((vector_size(16))) v1 = {1, 2, 3, 4};
    int __attribute__((vector_size(16))) v2 = {5, 6, 7, 8};
    int __attribute__((vector_size(16))) v3;
    
    /* Vector operations (may create VEC* tree codes) */
    v3 = v1 + v2;
    total += v3[0];
    
    /* Final print to prevent optimization */
    printf("Result: %d\n", total);
    
    return 0;
}

/* Additional C++-style constructs if compiled as C++ */
#ifdef __cplusplus
/* User-defined literal */
constexpr unsigned long long operator"" _custom(unsigned long long n) {
    return n * 2;
}

/* Template with trailing return type */
template<typename T, typename U>
auto template_func(T t, U u) -> decltype(t + u) {
    return t + u;
}

/* nullptr usage */
void* get_null() {
    return nullptr;
}
#endif
