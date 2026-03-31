/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. CONSTANT FOLDING WITH LARGE INTEGERS ========== */

/* Use __int128 for 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)0x7FFFFFFFFFFFFFFF) * 2;
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 80);

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");
_Static_assert(LARGE_UNSIGNED > UINT64_MAX, "Unsigned exceeds 64-bit");

/* Compile-time comparisons using __builtin_constant_p */
#define COMPILE_TIME_CMP(a, b) \
    (__builtin_constant_p((a) > (b)) ? ((a) > (b)) : 0)

/* ========== 2. GCC BUILTINS WITH OVERFLOW ========== */

/* Test overflow builtins that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b, result;
    int overflow;
    
    /* Test cases that should trigger overflow comparisons */
    a = LLONG_MAX;
    b = 2;
    
    /* Multiplication overflow - uses double_int for overflow check */
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d, result=%lld\n", 
           overflow, result);
    
    /* Addition overflow */
    overflow = __builtin_add_overflow(a, 1, &result);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d, result=%lld\n",
           overflow, result);
    
    /* Subtraction overflow */
    a = LLONG_MIN;
    overflow = __builtin_sub_overflow(a, 1, &result);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d, result=%lld\n",
           overflow, result);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, 0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. RANGE CALCULATIONS ========== */

/* Complex range analysis that uses double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons with the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y=%lld within expected range\n", y);
        }
        
        /* Test with large step values */
        for (int i = x; i < 10000; i += 1000000) {
            /* Loop analysis may use double_int for wrap-around checks */
            if (i > 5000) break;
        }
    }
    
    /* Test with very large ranges */
    if (x > -1000000000 && x < 1000000000) {
        /* Multiplication that could overflow 64-bit */
        int128_t big_product = (int128_t)x * 1000000000;
        
        /* Comparison of large values */
        if (big_product > -1000000000000 && big_product < 1000000000000) {
            printf("Large range product: %lld\n", (long long)big_product);
        }
    }
}

/* ========== 4. TEMPLATE METAPROGRAMMING (C++ VERSION) ========== */

#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > (int128_t(1) << 65);
    static const bool is_very_large = N > (int128_t(1) << 100);
    
    /* Force comparisons at compile time */
    static const int compare_to_mid = (N > (int128_t(1) << 63)) ? 1 : 
                                      (N < (int128_t(1) << 63)) ? -1 : 0;
};

/* Instantiate templates with various large values */
template struct LargeCompare<(int128_t(1) << 70)>;
template struct LargeCompare<-(int128_t(1) << 70)>;
template struct LargeCompare<(int128_t(1) << 90)>;

#endif

/* ========== 5. TREE NODE CONSTRUCTION ========== */

/* Use mode attributes for 128-bit types */
typedef int int128 __attribute__((mode(TI)));
typedef unsigned int uint128 __attribute__((mode(TI)));

/* Operations that create wide constants */
void test_wide_operations(void) {
    /* Division of large values - requires magnitude comparison */
    int128 big1 = ((int128)1 << 70);
    int128 big2 = ((int128)1 << 65);
    
    /* These operations use double_int comparisons internally */
    int128 quotient = big1 / big2;
    int128 remainder = big1 % big2;
    
    printf("Wide division: (1<<70) / (1<<65) = %lld\n", 
           (long long)quotient);
    
    /* Bitwise operations with large values */
    uint128 mask = ((uint128)0xFFFFFFFFFFFFFFFF) << 64;
    uint128 masked = LARGE_UNSIGNED & mask;
    
    /* Comparisons in conditional expressions */
    int cmp_result = (big1 > big2) ? 1 : 
                     (big1 < big2) ? -1 : 0;
    printf("Comparison result: %d\n", cmp_result);
}

/* ========== COMPREHENSIVE TEST FUNCTION ========== */

/* Test all comparison operators with large values */
void test_all_comparisons(void) {
    int128_t a = ((int128_t)1 << 70);
    int128_t b = ((int128_t)1 << 65);
    int128_t c = -((int128_t)1 << 70);
    
    /* Test all comparison operators */
    printf("Comparison tests:\n");
    printf("  a > b: %d\n", a > b);
    printf("  a < b: %d\n", a < b);
    printf("  a >= b: %d\n", a >= b);
    printf("  a <= b: %d\n", a <= b);
    printf("  a == b: %d\n", a == b);
    printf("  a != b: %d\n", a != b);
    
    printf("  c > a: %d\n", c > a);
    printf("  c < a: %d\n", c < a);
    
    /* Equality with same value different representation */
    int128_t d = ((int128_t)1 << 70);
    printf("  a == d: %d\n", a == d);
    
    /* Test with mixed signed/unsigned comparisons */
    uint128_t u = ((uint128_t)1 << 70);
    printf("  (uint128_t)a == u: %d\n", (uint128_t)a == u);
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    int test_value = 1500;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test overflow builtins */
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 2. Test range analysis */
    printf("2. Testing range analysis:\n");
    test_range_analysis(test_value);
    printf("\n");
    
    /* 3. Test wide operations */
    printf("3. Testing wide operations:\n");
    test_wide_operations();
    printf("\n");
    
    /* 4. Test all comparisons */
    printf("4. Testing all comparison operators:\n");
    test_all_comparisons();
    printf("\n");
    
    /* 5. Additional constant folding tests */
    printf("5. Additional constant folding:\n");
    
    /* Force evaluation of large constant expressions */
    const int128_t complex_expr = 
        ((int128_t)0x123456789ABCDEF << 32) + 0xFEDCBA987654321;
    
    /* Multiple comparisons in a single expression */
    int result = (complex_expr > 0) +
                 (complex_expr < ((int128_t)1 << 80)) +
                 (complex_expr != 0);
    printf("   Complex expression comparisons: %d/3 passed\n", result);
    
    /* Test with boundary values */
    int128_t max_positive = ((int128_t)1 << 63) - 1;
    int128_t min_negative = -((int128_t)1 << 63);
    
    printf("   Boundary comparisons:\n");
    printf("     max_positive > min_negative: %d\n", 
           max_positive > min_negative);
    printf("     max_positive < -min_negative: %d\n",
           max_positive < -min_negative);
    
    printf("\n=== All tests completed ===\n");
    
    /* Runtime validation */
    if (VERY_LARGE_POS > 0 && VERY_LARGE_NEG < 0 && 
        HUGE_PRODUCT > INT64_MAX && LARGE_UNSIGNED > UINT64_MAX) {
        printf("PASS: All compile-time comparisons correct\n");
        return 0;
    } else {
        printf("FAIL: Some comparisons incorrect\n");
        return 1;
    }
}
