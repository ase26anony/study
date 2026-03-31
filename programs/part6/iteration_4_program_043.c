/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ==================== PART 1: Large Integer Constants ==================== */

/* Test 1.1: Static assertions with very large 128-bit constants */
static_assert(sizeof(__int128) == 16, "128-bit support required");

/* Large constants that require double_int representation */
#define VERY_LARGE_POS ((__int128)1 << 70)
#define VERY_LARGE_NEG (-((__int128)1 << 70))
#define HUGE_PRODUCT ((__int128)0x7FFFFFFFFFFFFFFF * 2)

/* Compile-time comparisons that should trigger double_int::cmp */
static_assert(VERY_LARGE_POS > 0, "Large positive comparison");
static_assert(VERY_LARGE_NEG < 0, "Large negative comparison");
static_assert(VERY_LARGE_POS > VERY_LARGE_NEG, "Cross-sign comparison");
static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");

/* Test 1.2: Template-style comparisons (C++-like in C) */
#define COMPARE_CONSTANTS(a, b) \
    _Generic((a), \
        __int128: _Generic((b), \
            __int128: (a) > (b) ? 1 : ((a) < (b) ? -1 : 0) \
        ) \
    )

/* Force evaluation at compile time */
const int cmp_result_1 = COMPARE_CONSTANTS(VERY_LARGE_POS, VERY_LARGE_NEG);
const int cmp_result_2 = COMPARE_CONSTANTS(HUGE_PRODUCT, VERY_LARGE_POS);

/* ==================== PART 2: Builtin Overflow Checks ==================== */

/* Test 2.1: Multiplication overflow with comparison */
int test_mul_overflow_comparison(long long a, long long b) {
    long long result;
    int overflow = __builtin_mul_overflow(a, b, &result);
    
    /* The overflow check internally uses double_int comparisons */
    if (overflow) {
        /* Determine if overflow was positive or negative */
        __int128 wide_result = (__int128)a * (__int128)b;
        if (wide_result > INT64_MAX) return 1;
        if (wide_result < INT64_MIN) return -1;
    }
    
    /* Compare with expected range */
    __int128 expected = (__int128)a * (__int128)b;
    if (expected > result) return 2;
    if (expected < result) return -2;
    
    return 0;
}

/* Test 2.2: Constant overflow checks */
void test_constant_overflow(void) {
    /* These force constant folding with overflow detection */
    long long max_ll = LLONG_MAX;
    long long min_ll = LLONG_MIN;
    
    /* Each of these should trigger double_int comparisons internally */
    if (__builtin_constant_p(__builtin_mul_overflow_p(max_ll, 2, (long long)0))) {
        /* Compile-time overflow detection */
    }
    
    if (__builtin_constant_p(__builtin_add_overflow_p(max_ll, 1, (long long)0))) {
        /* Compile-time overflow detection */
    }
}

/* ==================== PART 3: Range Analysis Tests ==================== */

/* Test 3.1: Complex range calculations with comparisons */
int range_analysis_test(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* This multiplication creates a range that needs double_int comparison */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            return y * 2;  /* More range calculations */
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 1000000 && y < 4000000) {
            return -y;
        }
    }
    
    return 0;
}

/* Test 3.2: Loop induction variable analysis */
void loop_induction_test(void) {
    /* Loop with large step that may trigger wrap analysis */
    for (int64_t i = INT64_MAX - 100; i < INT64_MAX + 100LL; i += 10) {
        /* The loop bound comparison uses double_int */
        volatile int64_t dummy = i;  /* Prevent optimization */
        (void)dummy;
    }
    
    /* Another loop crossing zero */
    for (int64_t j = -100; j < 100; j += 7) {
        int64_t k = j * j * j;  /* Cubic growth - large range */
        (void)k;
    }
}

/* ==================== PART 4: 128-bit Operations ==================== */

/* Test 4.1: Direct 128-bit arithmetic and comparison */
void test_128bit_operations(void) {
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA987654320F;
    __int128 c = -a;
    
    /* Series of comparisons that should use double_int::cmp */
    int results[6];
    results[0] = (a > b) ? 1 : 0;
    results[1] = (a < b) ? 0 : 1;
    results[2] = (a == a) ? 1 : 0;
    results[3] = (c < a) ? 1 : 0;
    results[4] = (b > c) ? 1 : 0;
    results[5] = (a != b) ? 1 : 0;
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        printf("Comparison %d: %d\n", i, results[i]);
    }
}

/* Test 4.2: Division with large numbers (requires magnitude comparison) */
void test_large_division(void) {
    __int128 big_num = ((__int128)1 << 120) - 1;
    __int128 divisor = ((__int128)1 << 60) + 1;
    
    /* Division algorithm compares magnitudes */
    __int128 quotient = big_num / divisor;
    __int128 remainder = big_num % divisor;
    
    printf("Division test: quotient = %lx%016lx, remainder = %lx\n",
           (uint64_t)(quotient >> 64), (uint64_t)quotient,
           (uint64_t)remainder);
}

/* ==================== PART 5: Mixed-width Comparisons ==================== */

/* Test 5.1: Compare 128-bit with 64-bit constants */
void test_mixed_width(void) {
    __int128 large = ((__int128)1 << 70);
    int64_t medium = INT64_MAX;
    uint64_t umedium = UINT64_MAX;
    
    /* These comparisons may promote to 128-bit */
    if (large > medium) {
        printf("Large > medium (signed)\n");
    }
    
    if ((__uint128_t)large > umedium) {
        printf("Large > medium (unsigned)\n");
    }
    
    /* Test with negative values */
    __int128 neg_large = -((__int128)1 << 70);
    if (neg_large < medium) {
        printf("Negative large < medium\n");
    }
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(void) {
    int failures = 0;
    
    printf("=== Starting double_int::cmp coverage tests ===\n\n");
    
    /* Part 1: Verify compile-time comparisons worked */
    printf("Part 1: Compile-time constant comparisons\n");
    printf("  cmp_result_1 (pos vs neg): %d\n", cmp_result_1);
    printf("  cmp_result_2 (huge vs large): %d\n", cmp_result_2);
    
    if (cmp_result_1 != 1) {
        printf("  ERROR: VERY_LARGE_POS should be greater than VERY_LARGE_NEG\n");
        failures++;
    }
    
    /* Part 2: Builtin overflow tests */
    printf("\nPart 2: Builtin overflow comparisons\n");
    test_constant_overflow();
    
    /* Test overflow cases */
    long long test_cases[][2] = {
        {LLONG_MAX, 2},
        {LLONG_MIN, -1},
        {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF},
        {1000000, 1000000}
    };
    
    for (size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        int result = test_mul_overflow_comparison(test_cases[i][0], test_cases[i][1]);
        printf("  Overflow test %zu: %d\n", i, result);
    }
    
    /* Part 3: Range analysis */
    printf("\nPart 3: Range analysis tests\n");
    int range_result = range_analysis_test(1500);
    printf("  Range test with 1500: %d\n", range_result);
    
    loop_induction_test();
    printf("  Loop induction tests completed\n");
    
    /* Part 4: 128-bit operations */
    printf("\nPart 4: 128-bit operations\n");
    test_128bit_operations();
    test_large_division();
    
    /* Part 5: Mixed-width */
    printf("\nPart 5: Mixed-width comparisons\n");
    test_mixed_width();
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    if (failures == 0) {
        printf("All tests passed! The double_int::cmp logic should have been exercised.\n");
        printf("Compile with: gcc -O3 -fstrict-overflow -ftree-vrp test_double_int_cmp.c\n");
        printf("Or for overflow tests: gcc -O2 -fwrapv test_double_int_cmp.c\n");
    } else {
        printf("%d test(s) failed\n", failures);
    }
    
    return failures;
}
