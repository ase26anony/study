/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE ((__int128_t)-1 << 80)
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");

/* Compile-time function using __builtin_constant_p */
static int compile_time_compare(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > HUGE_NEGATIVE)) {
        return 1;
    }
    return 0;
}

/* ========== 2. GCC Builtins That Manipulate double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* These will trigger double_int comparisons in overflow checking */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Overflow test 1: %ld * 2 overflow? %s\n", 
           (long)a, overflow ? "yes" : "no");
    
    a = INT64_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Overflow test 2: %ld * -1 overflow? %s\n", 
           (long)a, overflow ? "yes" : "no");
    
    /* Test with __builtin_constant_p */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, INT64_MAX))) {
        printf("Constant overflow check performed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that uses double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication creates range that needs double_int comparison */
        int64_t y = (int64_t)x * x;
        
        /* Further range refinement */
        if (y > 1500000 && y < 3000000) {
            int64_t z = y * 2;
            printf("Range analysis: x=%d, y=%ld, z=%ld\n", x, (long)y, (long)z);
        }
    }
    
    /* Test with potential overflow in range */
    if (x > 0 && x < 100000) {
        int64_t large = (int64_t)x * 1000000000LL;
        if (large > 50000000000LL && large < 100000000000LL) {
            printf("Large range check passed\n");
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = INT64_MAX - 100; i < INT64_MAX; i += 10) {
        /* Compiler analyzes wrap-around behavior with double_int */
        if (i > INT64_MAX - 50) {
            printf("Near overflow: %ld\n", (long)i);
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_very_large = N > ((__int128_t)1 << 65);
    static const bool greater_than_max = N > INT64_MAX;
};

template <__int128_t A, __int128_t B>
struct CompareValues {
    static const int result = (A < B) ? -1 : ((A > B) ? 1 : 0);
};

void test_templates(void) {
    /* Instantiate templates with large values */
    const bool check1 = LargeCompare<((__int128_t)1 << 70)>::is_very_large;
    const bool check2 = LargeCompare<-((__int128_t)1 << 68)>::is_positive;
    const int cmp_result = CompareValues<((__int128_t)1 << 66), 
                                         ((__int128_t)1 << 67)>::result;
    
    printf("Template tests: check1=%d, check2=%d, cmp=%d\n",
           check1, check2, cmp_result);
}

#endif

/* ========== 5. Tree Node Construction for Wide Constants ========== */

/* Use __int128 with operations that require magnitude comparison */
void test_wide_constant_operations(void) {
    __int128 a = ((__int128_t)1 << 70) + 123;
    __int128 b = ((__int128_t)1 << 69) - 456;
    
    /* Operations that require comparing magnitudes */
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 prod = a / 2;  /* Division requires magnitude comparison */
    
    /* Comparisons at runtime (but compiler may analyze at compile time) */
    if (a > b) {
        printf("Wide compare: a > b\n");
    }
    
    if (sum > diff) {
        printf("Wide compare: sum > diff\n");
    }
    
    /* Modulus operation often uses double_int comparisons */
    __int128 mod_result = a % b;
    printf("Modulus result: high bits\n");
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE = ((__int128_t)1 << 72),
    BIGGER_VALUE = ((__int128_t)1 << 73),
    BIG_NEGATIVE = -((__int128_t)1 << 72)
};

/* ========== Main Test Harness ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Force evaluation of compile-time comparisons */
    int ct_result = compile_time_compare();
    printf("Compile-time compare result: %d\n", ct_result);
    
    /* Test overflow builtins */
    test_overflow_builtins();
    printf("\n");
    
    /* Test range analysis with various inputs */
    printf("Testing range analysis:\n");
    for (int i = 1001; i < 1005; i++) {
        test_range_analysis(i);
    }
    printf("\n");
    
    /* Test induction variable analysis */
    printf("Testing induction variables:\n");
    test_induction_variables();
    printf("\n");
    
    /* Test wide constant operations */
    printf("Testing wide constant operations:\n");
    test_wide_constant_operations();
    printf("\n");
    
#ifdef __cplusplus
    /* Test template metaprogramming */
    printf("Testing template metaprogramming:\n");
    test_templates();
    printf("\n");
#endif
    
    /* Additional complex comparisons */
    printf("Additional complex comparisons:\n");
    
    /* Chain of comparisons that might be optimized together */
    __int128 x = ((__int128_t)1 << 68);
    __int128 y = ((__int128_t)1 << 67);
    __int128 z = ((__int128_t)1 << 66);
    
    if (x > y && y > z && x > z) {
        printf("Chain comparison passed\n");
    }
    
    /* Mixed signed/unsigned comparison */
    unsigned __int128 ux = ((unsigned __int128)1 << 72);
    __int128 sx = ((__int128_t)1 << 71);
    
    if (ux > (unsigned __int128)sx) {
        printf("Mixed signedness comparison passed\n");
    }
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
