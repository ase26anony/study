/* test_tree_codes.c - Comprehensive test for GCC internal tree node generation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with constant condition */
static inline int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x * 2, x / 2);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    __builtin_constant_p(_x) ? _x * 3 : _x + 1; \
})

/* Type-dependent expression with __builtin_types_compatible_p */
static inline int type_compat_test(void *ptr) {
    return __builtin_types_compatible_p(typeof(ptr), void*) 
           ? 0 : __builtin_types_compatible_p(typeof(ptr), int*) ? 1 : 2;
}

/* Function with static array parameter (creates specialized tree nodes) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with GNU attributes in unusual places */
int __attribute__((noinline)) 
attributed_function(int x __attribute__((unused)), 
                    int y __attribute__((deprecated))) {
    return y;
}

/* C11 _Generic selector expression */
#define TYPE_SELECTOR(x) _Generic((x), \
    int: 1, \
    double: 2, \
    char*: 3, \
    default: 0 \
)

/* OpenMP helper with complex reduction */
#pragma omp declare reduction(complex_add : int : omp_out += omp_in) \
    initializer(omp_priv = 0)

/* Test case ranges in switch (GNU extension) */
static int test_case_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 10 ... 20:
            return val * 20;
        default:
            return val;
    }
}

/* Function with trailing return type (C++-like in GNU C) */
#define AUTO_RETURN(x) ({ \
    auto __ret = (x); \
    __ret; \
})

/* Potential error recovery path */
#ifdef TEST_ERROR_PATH
static void test_error_recovery(void) {
    /* Declare void function */
    void void_func(void);
    
    /* This would normally be an error but might create recovery nodes */
    /* void_func(1); */  /* Uncomment to test error paths */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expressions */
    result += builtin_choose_test(5);
    
    /* Test statement expressions */
    result += COMPLEX_EXPR(10);
    
    /* Test type compatibility */
    int x = 42;
    result += type_compat_test(&x);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test attributed function */
    result += attributed_function(1, 2);
    
    /* Test _Generic selector */
    result += TYPE_SELECTOR(3.14);
    result += TYPE_SELECTOR("test");
    
    /* Test case ranges */
    result += test_case_ranges(3);
    result += test_case_ranges(15);
    
    /* OpenMP region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < 100; i++) {
        result += i % 10;
    }
    
    /* More complex OpenMP construct */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom:result) reduction(+:result)
    for (int i = 0; i < 50; i++) {
        result += i % 5;
    }
    
    /* Test auto return macro */
    result += AUTO_RETURN(result * 2);
    
    /* User-defined literal style (GNU extension) */
    int __attribute__((mode(QI))) byte_var = 255;
    result += byte_var;
    
    /* Compound literal with attributes */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
    } ps = {1, 2};
    result += ps.b;
    
    /* Transparent union (GNU extension) */
    typedef union __attribute__((transparent_union)) {
        int *intp;
        void *voidp;
    } transparent_union_t;
    
    /* Offsetof with member designators */
    result += __builtin_offsetof(struct { int a; char b[10]; }, b[5]);
    
    /* Builtin constant check in complex expression */
    if (__builtin_constant_p(result)) {
        result += 1000;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* OpenACC pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop reduction(+:result)
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    #endif
    
    return 0;
}

/* Additional C++-like constructs if compiled as C++ */
#ifdef __cplusplus
namespace test_ns {
    template<typename T>
    T template_func(T x) {
        return x + T(1);
    }
    
    /* nullptr usage */
    void* test_nullptr() {
        return nullptr;
    }
}
#endif
