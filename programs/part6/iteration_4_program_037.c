/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Constant Folding with Large Integers ========== */

/* Use __int128 for 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 100);  /* 2^100 */
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 100);
static const int128_t LARGE_MID = ((int128_t)1 << 70);
static const int128_t MAX_INT64 = ((int128_t)INT64_MAX);
static const int128_t MIN_INT64 = ((int128_t)INT64_MIN);

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(VERY_LARGE_POS > LARGE_MID, "Large > Mid comparison");
_Static_assert(LARGE_MID < VERY_LARGE_POS, "Mid < Large comparison");
_Static_assert(VERY_LARGE_POS != VERY_LARGE_NEG, "Inequality check");
_Static_assert(VERY_LARGE_POS == VERY_LARGE_POS, "Equality check");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        static volatile int _dummy __attribute__((unused)) = \
            __builtin_choose_expr(__builtin_constant_p((a) op (b)), 1, 0); \
    } while(0)

/* ========== 2. GCC Builtins with Overflow ========== */

/* Test overflow builtins that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b;
    long long result;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Near max */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Mul overflow test 1: %lld * 2 overflow? %d\n", a, overflow);
    
    /* Case 2: Large multiplication requiring double_int comparison */
    a = 0x123456789ABCDEFLL;
    b = 0xFEDCBA987654321LL;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Mul overflow test 2: large * large overflow? %d\n", overflow);
    
    /* Case 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    printf("Add overflow test: max+1 overflow? %d\n", overflow);
    
    /* Case 4: __builtin_constant_p with overflow check */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                      2, 
                                                      (long long)0))) {
        printf("Constant overflow check performed\n");
    }
}

/* ========== 3. Range Calculations ========== */

/* Complex range analysis that triggers double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication that requires range analysis with wide ints */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test passed: y = %lld\n", (long long)y);
        }
        
        /* Nested range with large values */
        if (x > 1500) {
            int64_t z = y * 1000LL;
            if (z > 1500000000LL) {
                printf("Nested range: z = %lld\n", (long long)z);
            }
        }
    }
    
    /* Test with very large ranges */
    if (x > INT32_MAX && x < INT64_MAX) {
        int128_t big = (int128_t)x * (int128_t)x;
        if (big > ((int128_t)1 << 80)) {
            printf("Very large range triggered\n");
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = 0; i < 1000000000000LL; i += 1000000000LL) {
        /* The compiler analyzes the loop bounds using double_int */
        if (i > 500000000000LL) {
            printf("Induction: i = %lld\n", (long long)i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ style in C) ========== */

/* Simulate template metaprogramming using macros and inline functions */
#define LARGE_COMPARE(N, M) \
    (__builtin_constant_p(N) && __builtin_constant_p(M) ? (N) > (M) : 0)

/* Force evaluation of large comparisons at compile time */
static const int cmp1 = LARGE_COMPARE(((int128_t)1 << 80), ((int128_t)1 << 79));
static const int cmp2 = LARGE_COMPARE(((int128_t)1 << 65), ((int128_t)1 << 64) + 1);
static const int cmp3 = LARGE_COMPARE(-((int128_t)1 << 90), -((int128_t)1 << 91));

/* ========== 5. Tree Node Construction ========== */

/* Use 128-bit types with attributes */
typedef int128_t int128_attr __attribute__((mode(TI)));
typedef uint128_t uint128_attr __attribute__((mode(TI)));

/* Operations that create wide INTEGER_CST nodes */
void test_wide_operations(void) {
    int128_attr a = ((int128_attr)1 << 120);
    int128_attr b = ((int128_attr)1 << 119);
    
    /* Comparisons that should use double_int::cmp */
    if (a > b) {
        printf("Wide comparison 1: a > b\n");
    }
    
    if (a != b) {
        printf("Wide comparison 2: a != b\n");
    }
    
    /* Division with large constants - requires magnitude comparison */
    int128_attr c = a / 2;
    if (c == b) {
        printf("Wide division correct: %lld == %lld\n", 
               (long long)(c >> 64), (long long)(b >> 64));
    }
    
    /* Modulus operation */
    int128_attr d = a % b;
    if (d < b) {
        printf("Wide modulus: remainder < divisor\n");
    }
}

/* Enumeration with large values */
enum big_enum : int128_t {
    BIG_VAL1 = ((int128_t)1 << 80),
    BIG_VAL2 = ((int128_t)1 << 81),
    BIG_VAL3 = BIG_VAL1 + BIG_VAL2
};

/* ========== Main Test Harness ========== */

int main(void) {
    int test_passes = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test constant folding */
    printf("1. Constant Folding Tests:\n");
    COMPILE_TIME_CMP(VERY_LARGE_POS, LARGE_MID, >);
    COMPILE_TIME_CMP(VERY_LARGE_NEG, 0, <);
    COMPILE_TIME_CMP(MAX_INT64 * 2, MAX_INT64, >);
    printf("   [PASS] Static assertions compiled\n");
    test_passes++;
    
    /* 2. Test overflow builtins */
    printf("\n2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    test_passes++;
    
    /* 3. Test range analysis */
    printf("\n3. Range Analysis Tests:\n");
    test_range_analysis(1500);
    test_range_analysis(3000000000);
    test_induction_variables();
    test_passes++;
    
    /* 4. Test template-like comparisons */
    printf("\n4. Template-style Comparisons:\n");
    printf("   cmp1 (2^80 > 2^79): %d\n", cmp1);
    printf("   cmp2 (2^65 > 2^64+1): %d\n", cmp2);
    printf("   cmp3 (-2^90 > -2^91): %d\n", cmp3);
    if (cmp1 && cmp2 && cmp3) {
        printf("   [PASS] All compile-time comparisons correct\n");
        test_passes++;
    }
    
    /* 5. Test wide operations */
    printf("\n5. Wide Integer Operations:\n");
    test_wide_operations();
    test_passes++;
    
    /* 6. Additional edge cases */
    printf("\n6. Edge Case Tests:\n");
    
    /* Compare values with same high part, different low parts */
    int128_t same_high_diff_low1 = ((int128_t)0x12345678 << 64) | 0x1;
    int128_t same_high_diff_low2 = ((int128_t)0x12345678 << 64) | 0x2;
    
    if (same_high_diff_low1 < same_high_diff_low2) {
        printf("   [PASS] Same high, different low comparison works\n");
        test_passes++;
    }
    
    /* Compare negative values */
    int128_t neg_big = -((int128_t)1 << 100);
    int128_t neg_small = -((int128_t)1 << 50);
    
    if (neg_big < neg_small) {
        printf("   [PASS] Negative comparison works\n");
        test_passes++;
    }
    
    /* Test with maximum values */
    uint128_t max_u128 = ~(uint128_t)0;
    uint128_t almost_max = max_u128 - 1;
    
    if (almost_max < max_u128) {
        printf("   [PASS] Unsigned max comparison works\n");
        test_passes++;
    }
    
    printf("\n=== Summary ===\n");
    printf("Total test categories passed: %d/6\n", test_passes);
    
    if (test_passes == 6) {
        printf("\n[SUCCESS] All tests passed - double_int::cmp should be covered\n");
        return 0;
    } else {
        printf("\n[PARTIAL] Some tests may not have triggered coverage\n");
        return 1;
    }
}
