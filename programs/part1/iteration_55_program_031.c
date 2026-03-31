/* test_tree_codes.c - Comprehensive test for GCC internal tree node generation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_test(void) {
    /* Creates specialized expression nodes */
    int x = __builtin_choose_expr(1, 42, 0/0);
    return __builtin_choose_expr(0, 0/0, x);
}

/* Statement expression in constant context */
#define STMT_EXPR_TEST(val) ({ \
    int _result = (val); \
    typeof(_result) _double = _result * 2; \
    _double; \
})

/* Type-dependent expression with __builtin_types_compatible_p */
static int type_compatible_test(int a, double b) {
    /* May generate comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return a + (int)b;
    }
    return __builtin_types_compatible_p(typeof(b), double) ? 1 : 0;
}

/* Function with static array parameter (C99 feature) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_constant_p in complex expression */
static int constant_p_test(int x) {
    return __builtin_constant_p(x) ? x : (x + STMT_EXPR_TEST(5));
}

/* C11 _Generic selection expression */
#define TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Attribute syntax in unusual places */
static int __attribute__((unused)) unused_var __attribute__((aligned(16))) = 0;

/* Switch with case ranges (GNU extension) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val + 5;
        default:
            return -val;
    }
}

/* OpenMP helper function */
static void omp_helper(int *data, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; ++i) {
        data[i] = data[i] * 2 + i;
    }
}

/* Function with void parameter */
static void void_func(void) {
    /* Empty */
}

#ifdef TEST_ERROR_PATH
/* This should generate error recovery nodes if compiled */
static void test_error_recovery(void) {
    /* Calling void function with argument (semantically invalid) */
    void_func(42);  /* Will cause error but may generate special nodes */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose */
    result += builtin_choose_test();
    
    /* Test statement expression */
    result += STMT_EXPR_TEST(10);
    
    /* Test type compatibility */
    result += type_compatible_test(5, 3.14);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test __builtin_constant_p */
    result += constant_p_test(20);
    
    /* Test _Generic */
    const char *type_str = TYPE_NAME(result);
    (void)type_str;  /* Use variable */
    
    /* Test case ranges */
    result += case_range_test(3);
    
    /* OpenMP section - generates various OpenMP tree nodes */
    int omp_data[100];
    #pragma omp target teams distribute parallel for simd map(tofrom: omp_data)
    for (int i = 0; i < 100; ++i) {
        omp_data[i] = i * i;
    }
    
    /* Additional OpenMP construct */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            omp_helper(omp_data, 100);
        }
        #pragma omp section
        {
            for (int i = 0; i < 50; ++i) {
                omp_data[i] += 1;
            }
        }
    }
    
    /* Use attribute in declaration */
    int __attribute__((vector_size(16))) v1 = {1, 2, 3, 4};
    int __attribute__((vector_size(16))) v2 = {5, 6, 7, 8};
    int __attribute__((vector_size(16))) v3;
    
    /* Vector operations (may generate special nodes) */
    v3 = v1 + v2;
    result += ((int*)&v3)[0];
    
    /* Array with designated initializers (C99) */
    struct {
        int a;
        int b[3];
    } s = {
        .a = 1,
        .b = {2, 3, 4}
    };
    result += s.a + s.b[1];
    
    /* Compound literal */
    result += ((int[]){10, 20, 30})[1];
    
    printf("Result: %d\n", result);
    printf("Type of result: %s\n", TYPE_NAME(result));
    
    return 0;
}

/* Additional C++ style constructs if compiled as C++ */
#ifdef __cplusplus
namespace TestNamespace {
    template<typename T>
    T template_func(T x) {
        return x * 2;
    }
    
    auto lambda_test = []() -> int {
        return 42;
    };
}
#endif
