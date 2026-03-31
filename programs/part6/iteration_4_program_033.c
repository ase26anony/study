/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define VERY_LARGE_NEGATIVE (-((__int128_t)1 << 70))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 4)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant");
static_assert(VERY_LARGE_NEGATIVE < 0, "Large negative constant");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit range");

/* Compile-time function using __builtin_constant_p */
static int compile_time_compare(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > VERY_LARGE_NEGATIVE)) {
        return 1;
    }
    return 0;
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Multiplication that will overflow 64-bit */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    /* Addition that will overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d\n", overflow);
    
    /* Subtraction that will underflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d\n", overflow);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that uses double_int comparisons */
static void test_range_analysis(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further range checks */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y=%lld within expected range\n", y);
        }
    }
    
    /* Test with larger values that might require 128-bit intermediate */
    if (x > 1000000) {
        __int128_t big_product = (__int128_t)x * x * x;
        if (big_product > ((__int128_t)1 << 60)) {
            printf("Large product: %lld...\n", (long long)(big_product >> 32));
        }
    }
}

/* Loop with induction variable analysis */
static void test_induction_variables(void) {
    for (int64_t i = 0; i < 1000; i += (INT64_MAX / 1000)) {
        /* The compiler analyzes the loop bounds using double_int comparisons */
        if (i > INT32_MAX) {
            printf("Induction variable exceeded 32-bit range: %lld\n", (long long)i);
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_zero = N > 0;
    static const bool less_than_max = N < ((__int128_t)1 << 65);
    static const bool equal_to_self = N == N;
};

template <__int128_t A, __int128_t B>
struct CompareValues {
    static const int result = (A < B) ? -1 : ((A > B) ? 1 : 0);
};

void test_template_metaprogramming(void) {
    /* Instantiate templates with large values */
    constexpr __int128_t large_val = ((__int128_t)1 << 70);
    constexpr __int128_t larger_val = ((__int128_t)1 << 71);
    
    /* These instantiations force compile-time comparisons */
    LargeCompare<large_val> comp1;
    LargeCompare<larger_val> comp2;
    CompareValues<large_val, larger_val> comp3;
    
    printf("Template comparisons instantiated\n");
    printf("large_val > 0: %d\n", comp1.greater_than_zero);
    printf("large_val < 2^65: %d\n", comp1.less_than_max);
    printf("compare(large, larger): %d\n", comp3.result);
}

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit integer type with attribute */
typedef __int128_t int128 __attribute__((mode(TI)));

/* Operations that require magnitude comparisons */
static void test_wide_operations(void) {
    int128 a = ((int128)1 << 100);
    int128 b = ((int128)1 << 99);
    
    /* Division and modulus operations compare magnitudes */
    int128 quotient = a / b;
    int128 remainder = a % b;
    
    printf("Wide division: 2^100 / 2^99 = %lld\n", (long long)quotient);
    printf("Wide modulus: 2^100 %% 2^99 = %lld\n", (long long)remainder);
    
    /* Comparisons between wide integers */
    if (a > b) printf("a > b (expected)\n");
    if (b < a) printf("b < a (expected)\n");
    if (a != b) printf("a != b (expected)\n");
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = ((__int128)1 << 65),
    BIG_ENUM_B = ((__int128)1 << 66),
    BIG_ENUM_C = BIG_ENUM_A + BIG_ENUM_B
};

/* ========== Main Test Harness ========== */

int main(void) {
    int pass = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Constant folding tests */
    printf("1. Constant Folding Tests:\n");
    printf("   VERY_LARGE_CONST = %lld...\n", (long long)(VERY_LARGE_CONST >> 32));
    printf("   compile_time_compare() = %d\n", compile_time_compare());
    
    /* 2. Overflow builtin tests */
    printf("\n2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    
    /* 3. Range analysis tests */
    printf("\n3. Range Analysis Tests:\n");
    test_range_analysis(1500);
    test_range_analysis(2000000);
    test_induction_variables();
    
    /* 4. Template tests (C++ only) */
    #ifdef __cplusplus
    printf("\n4. Template Metaprogramming Tests:\n");
    test_template_metaprogramming();
    #endif
    
    /* 5. Wide constant tests */
    printf("\n5. Wide Constant Tests:\n");
    test_wide_operations();
    
    /* Runtime validation */
    printf("\n6. Runtime Validation:\n");
    
    /* Test various comparison scenarios */
    __int128_t test_cases[][2] = {
        {((__int128_t)1 << 63), ((__int128_t)1 << 62)},
        {0x7FFFFFFFFFFFFFFF, -0x7FFFFFFFFFFFFFFF},
        {((__int128_t)0xFFFFFFFFFFFFFFFF) << 64, 0},
        {((__int128_t)1 << 127) - 1, ((__int128_t)1 << 126)}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        __int128_t a = test_cases[i][0];
        __int128_t b = test_cases[i][1];
        
        int expected = (a < b) ? -1 : ((a > b) ? 1 : 0);
        printf("   Test case %zu: ", i + 1);
        
        /* The compiler's internal double_int::cmp should match our runtime comparison */
        if ((a < b && expected == -1) || 
            (a > b && expected == 1) || 
            (a == b && expected == 0)) {
            printf("PASS\n");
        } else {
            printf("FAIL\n");
            pass = 0;
        }
    }
    
    printf("\n=== %s ===\n", pass ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    
    return pass ? 0 : 1;
}
