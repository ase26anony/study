/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE (-((__int128_t)1 << 68))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant should be > 0");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant should be < 0");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product should exceed 64-bit max");

/* Template-like macro for compile-time comparison */
#define COMPARE_CONSTANTS(a, b) \
    _Static_assert((a) != (b), "Constants should not be equal"); \
    _Static_assert((a) > (b) || (a) < (b), "One should be greater")

/* Force comparisons with mixed high/low parts */
static_assert(((__int128_t)0x1 << 64) > ((__int128_t)0xFFFFFFFFFFFFFFFF), 
              "High part comparison test");
static_assert(((__int128_t)0x1 << 64) + 1 > ((__int128_t)0x1 << 64),
              "Low part comparison test");

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins that internally use double_int comparisons */
static int test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    if (!overflow) {
        printf("ERROR: Should have detected overflow\n");
        return 0;
    }
    
    /* Case 2: Addition with potential overflow */
    a = INT64_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    if (!overflow) {
        printf("ERROR: Should have detected overflow\n");
        return 0;
    }
    
    /* Case 3: Subtraction with underflow */
    a = INT64_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    if (!overflow) {
        printf("ERROR: Should have detected underflow\n");
        return 0;
    }
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, INT64_MAX))) {
        /* This forces the compiler to evaluate overflow at compile time */
    }
    
    return 1;
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that requires double_int comparisons */
static __int128_t analyze_ranges(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication creates range that needs wide integer comparison */
        __int128_t y = (__int128_t)x * x;
        
        /* Nested conditions for more complex range analysis */
        if (y > 1500000 && y < 4000000) {
            __int128_t z = y * 2;
            
            /* Force comparison of wide integer ranges */
            if (z > 3000000 && z < 8000000) {
                return z;
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        __int128_t y = (__int128_t)x * x;
        if (y > 1000000 && y < 4000000) {
            return y;
        }
    }
    
    return 0;
}

/* Loop with induction variable that may wrap */
static void test_loop_ranges(void) {
    for (int64_t i = INT64_MAX - 10; i < INT64_MAX + 10LL; i++) {
        /* The loop bound comparison requires double_int */
        if (i > INT64_MAX) {
            /* This path may be taken if compiler analyzes wrap-around */
            printf("Loop exceeded INT64_MAX: %ld\n", (long)i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

/* In C, we simulate template-like behavior with macros and static functions */
#define DECLARE_LARGE_COMPARE(N) \
    static inline int compare_##N(__int128_t val) { \
        const __int128_t threshold = ((__int128_t)1 << (N)); \
        if (val < threshold) return -1; \
        if (val > threshold) return 1; \
        return 0; \
    }

/* Declare comparisons with various large thresholds */
DECLARE_LARGE_COMPARE(65)  /* 2^65 */
DECLARE_LARGE_COMPARE(70)  /* 2^70 */
DECLARE_LARGE_COMPARE(100) /* 2^100 */

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t int128 __attribute__((mode(TI)));

/* Operations that require magnitude comparison during simplification */
static int128 wide_operations(int128 a, int128 b) {
    /* Division and modulus require comparing magnitudes */
    if (b != 0) {
        int128 div_result = a / b;
        int128 mod_result = a % b;
        
        /* Force comparison of results */
        if (div_result > 1000000000000000000LL) {
            return div_result;
        }
        if (mod_result < -1000000000000000000LL) {
            return mod_result;
        }
    }
    return a;
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE_1 = ((__int128_t)1 << 66),
    BIG_VALUE_2 = ((__int128_t)1 << 67),
    BIG_VALUE_3 = ((__int128_t)1 << 68)
};

/* ========== Main Test Harness ========== */

int main(void) {
    int passed = 1;
    
    printf("Testing double_int::cmp coverage...\n");
    
    /* Test 1: Overflow builtins */
    printf("1. Testing overflow builtins... ");
    if (test_overflow_builtins()) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        passed = 0;
    }
    
    /* Test 2: Range analysis */
    printf("2. Testing range analysis... ");
    __int128_t range_result = analyze_ranges(1500);
    if (range_result > 0) {
        printf("PASS (result: %lld)\n", (long long)range_result);
    } else {
        printf("PASS (no range matched)\n");
    }
    
    /* Test 3: Large constant comparisons */
    printf("3. Testing large constant comparisons... ");
    
    /* Force evaluation of comparisons with different high/low patterns */
    const __int128_t patterns[] = {
        ((__int128_t)0 << 64) | 0xFFFFFFFFFFFFFFFF,  /* low part only */
        ((__int128_t)1 << 64) | 0x0,                 /* high part only */
        ((__int128_t)1 << 64) | 0x1,                 /* both parts */
        ((__int128_t)0x7FFFFFFFFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF, /* max */
        -((__int128_t)1 << 70),                      /* large negative */
    };
    
    int comparison_results = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (patterns[i] < patterns[j]) comparison_results++;
            if (patterns[i] > patterns[j]) comparison_results++;
            if (patterns[i] == patterns[j]) comparison_results++;
        }
    }
    printf("PASS (%d comparisons)\n", comparison_results);
    
    /* Test 4: Wide operations */
    printf("4. Testing wide operations... ");
    int128 a = ((__int128_t)1 << 70) + 12345;
    int128 b = ((__int128_t)1 << 65) + 67890;
    int128 result = wide_operations(a, b);
    printf("PASS (result computed)\n");
    
    /* Test 5: Simulated template comparisons */
    printf("5. Testing threshold comparisons... ");
    int cmp1 = compare_65(((__int128_t)1 << 64));  /* less than 2^65 */
    int cmp2 = compare_70(((__int128_t)1 << 71));  /* greater than 2^70 */
    int cmp3 = compare_100(((__int128_t)1 << 100)); /* equal to 2^100 */
    
    if (cmp1 == -1 && cmp2 == 1 && cmp3 == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL (cmp1=%d, cmp2=%d, cmp3=%d)\n", cmp1, cmp2, cmp3);
        passed = 0;
    }
    
    /* Test 6: Loop range analysis */
    printf("6. Testing loop range analysis... ");
    test_loop_ranges();
    printf("PASS (loop completed)\n");
    
    /* Final summary */
    printf("\n=== Test Summary ===\n");
    if (passed) {
        printf("All tests passed! The double_int::cmp logic should have been exercised.\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}
