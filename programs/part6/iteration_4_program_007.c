/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Test 1A: Static assertions with 128-bit constants */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_VAL = ((int128_t)0x7FFFFFFFFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF;
static const int128_t HUGE_VAL2 = ((int128_t)0x7FFFFFFFFFFFFFFE << 64) | 0xFFFFFFFFFFFFFFFE;

/* Force compile-time comparisons through static assertions */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* These will trigger double_int::cmp during constant folding */
STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(HUGE_VAL > HUGE_VAL2);
STATIC_ASSERT(HUGE_VAL != HUGE_VAL2);
STATIC_ASSERT(VERY_LARGE_POS < HUGE_VAL);
STATIC_ASSERT(VERY_LARGE_NEG > -HUGE_VAL);

/* Test 1B: __builtin_constant_p with large comparisons */
static int test_constant_folding(void) {
    int result = 0;
    
    /* These comparisons should be evaluated at compile time */
    if (__builtin_constant_p(VERY_LARGE_POS > 1000)) {
        result |= 1;
    }
    
    if (__builtin_constant_p(HUGE_VAL > VERY_LARGE_POS)) {
        result |= 2;
    }
    
    /* Mixed high/low part comparisons */
    const int128_t val1 = ((int128_t)0x12345678 << 64) | 0x9ABCDEF0;
    const int128_t val2 = ((int128_t)0x12345678 << 64) | 0x9ABCDEF1;
    
    if (__builtin_constant_p(val1 < val2)) {
        result |= 4;
    }
    
    return result;
}

/* Test 1C: Arithmetic producing wide integers */
static int test_large_arithmetic(void) {
    int result = 0;
    
    /* Multiplication that produces 128-bit intermediate results */
    const int64_t large1 = 0x7FFFFFFFFFFFFFFF;
    const int64_t large2 = 0x7FFFFFFFFFFFFFFF;
    
    /* This multiplication overflows 64-bit but should be handled with double_int */
    int128_t product = (int128_t)large1 * (int128_t)large2;
    
    /* Comparisons on the product */
    if (product > VERY_LARGE_POS) {
        result |= 1;
    }
    
    /* Left shift beyond 63 bits */
    int128_t shifted = (int128_t)1 << 100;
    if (shifted > product) {
        result |= 2;
    }
    
    return result;
}
#endif

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test 2A: Overflow builtins with comparisons */
static int test_overflow_builtins(void) {
    int result = 0;
    
    /* Test multiplication overflow */
    long long a = LLONG_MAX;
    long long b = 2;
    long long res;
    
    if (__builtin_mul_overflow(a, b, &res)) {
        /* Overflow occurred - internal double_int comparison was used */
        result |= 1;
    }
    
    /* Test addition overflow */
    long long c = LLONG_MAX;
    long long d = 1;
    
    if (__builtin_add_overflow(c, d, &res)) {
        result |= 2;
    }
    
    /* Test __builtin_mul_overflow_p for constant evaluation */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        result |= 4;
    }
    
    /* Chain overflow operations */
    long long x = 1000000000000000000LL;
    long long y = 1000000000000000000LL;
    long long z;
    
    if (__builtin_mul_overflow(x, y, &z)) {
        result |= 8;
    } else if (__builtin_add_overflow(z, x, &z)) {
        result |= 16;
    }
    
    return result;
}

/* Test 2B: Complex overflow patterns */
static int test_overflow_patterns(void) {
    int result = 0;
    long long vals[] = {LLONG_MAX, LLONG_MIN, 0, 1, -1};
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            long long res;
            if (__builtin_mul_overflow(vals[i], vals[j], &res)) {
                result++;
            }
        }
    }
    
    return result;
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Test 3A: Value Range Propagation with large bounds */
static int test_vrp_large_ranges(void) {
    int result = 0;
    
    /* Create variables with known bounds */
    int x;
    
    /* Simulate input - in real code this would come from function parameters */
    /* We'll use constants to help the analyzer */
    #define X_MIN 1000000000
    #define X_MAX 2000000000
    
    /* These conditions create value ranges */
    if (x > X_MIN && x < X_MAX) {
        /* Multiplication that requires double_int for range analysis */
        long long y = (long long)x * x;
        
        /* Additional conditions that require comparing ranges */
        if (y > (long long)X_MIN * X_MIN) {
            result |= 1;
        }
        
        if (y < (long long)X_MAX * X_MAX) {
            result |= 2;
        }
    }
    
    /* Test with negative ranges */
    if (x > -X_MAX && x < -X_MIN) {
        long long y = (long long)x * x;
        if (y > (long long)X_MIN * X_MIN) {
            result |= 4;
        }
    }
    
    return result;
}

/* Test 3B: Loop induction variables with potential overflow */
static int test_induction_variables(void) {
    int result = 0;
    
    /* Loop with large step that might cause wrap-around analysis */
    for (long long i = LLONG_MAX - 1000; i < LLONG_MAX; i += 100) {
        /* This addition might overflow - VRP will analyze it */
        long long next = i + 100;
        if (next > i) {  /* Comparison that might use double_int */
            result++;
        }
    }
    
    /* Another loop crossing zero */
    for (long long i = -1000; i < 1000; i += 100) {
        long long square = i * i;
        if (square >= 0) {  /* Always true, but VRP must prove it */
            result++;
        }
    }
    
    return result;
}

/* ========== 4. Template Metaprogramming (C++ only) ========== */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool greater_than_zero = N > 0;
    static const bool less_than_max = N < ((int128_t)1 << 120);
    static const bool equal_self = N == N;
    static const bool not_equal = N != (N + 1);
};

template <int128_t A, int128_t B>
struct CompareValues {
    static const int cmp_result = (A > B) ? 1 : ((A < B) ? -1 : 0);
    static const bool a_greater = A > B;
    static const bool b_greater = B > A;
    static const bool equal = A == B;
};

/* Instantiate templates with various large values */
template struct CompareValues<((int128_t)1 << 70), ((int128_t)1 << 69)>;
template struct CompareValues<((int128_t)1 << 64) | 0xFFFFFFFF, ((int128_t)1 << 64)>;
template struct CompareValues<-((int128_t)1 << 70), 0>;

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Test 5A: Wide enumerations */
#ifdef __SIZEOF_INT128__
enum wide_enum : int128_t {
    BIG_ENUM_VAL = ((int128_t)1 << 80),
    BIGGER_ENUM_VAL = ((int128_t)1 << 90),
    NEG_ENUM_VAL = -((int128_t)1 << 80)
};

/* Test 5B: Operations on wide constants */
static int test_wide_constant_ops(void) {
    int result = 0;
    
    /* Division of large constants - requires magnitude comparison */
    const int128_t huge1 = ((int128_t)1 << 100);
    const int128_t huge2 = ((int128_t)1 << 99);
    
    if (huge1 / huge2 == 2) {
        result |= 1;
    }
    
    /* Modulus operation */
    const int128_t huge3 = ((int128_t)0x123456789ABCDEF << 64) | 0xFEDCBA9876543210;
    const int128_t huge4 = ((int128_t)1 << 60);
    
    if (huge3 % huge4 > 0) {
        result |= 2;
    }
    
    /* Bitwise operations */
    const int128_t mask = ((int128_t)0xFFFFFFFF << 64) | 0xFFFFFFFF;
    if ((huge3 & mask) == huge3) {
        result |= 4;
    }
    
    return result;
}
#endif

/* ========== Main Test Harness ========== */

int main(void) {
    int total_score = 0;
    int tests_passed = 0;
    
    printf("Testing double_int::cmp coverage...\n");
    printf("====================================\n");
    
#ifdef __SIZEOF_INT128__
    /* Test 1: Constant Folding */
    printf("\nTest 1: Constant Folding with Large Integers\n");
    int cf_result = test_constant_folding();
    printf("  Constant folding test result: 0x%02X\n", cf_result);
    if (cf_result == 0x07) {
        printf("  [PASS]\n");
        total_score += 25;
        tests_passed++;
    }
    
    int la_result = test_large_arithmetic();
    printf("  Large arithmetic test result: 0x%02X\n", la_result);
    if (la_result == 0x03) {
        printf("  [PASS]\n");
        total_score += 25;
        tests_passed++;
    }
    
    /* Test 5: Wide Constants */
    int wc_result = test_wide_constant_ops();
    printf("  Wide constant ops test result: 0x%02X\n", wc_result);
    if (wc_result == 0x07) {
        printf("  [PASS]\n");
        total_score += 25;
        tests_passed++;
    }
#else
    printf("\nNote: 128-bit integers not supported on this platform\n");
    printf("      Skipping tests 1 and 5\n");
#endif
    
    /* Test 2: Overflow Builtins */
    printf("\nTest 2: GCC Overflow Builtins\n");
    int of_result = test_overflow_builtins();
    printf("  Overflow builtins test result: 0x%02X\n", of_result);
    if (of_result & 0x1F) {
        printf("  [PASS - triggered overflow checks]\n");
        total_score += 25;
        tests_passed++;
    }
    
    int ofp_result = test_overflow_patterns();
    printf("  Overflow patterns test result: %d\n", ofp_result);
    if (ofp_result > 0) {
        printf("  [PASS - tested multiple overflow cases]\n");
        total_score += 25;
        tests_passed++;
    }
    
    /* Test 3: Range Calculations */
    printf("\nTest 3: Range Calculations and VRP\n");
    int vrp_result = test_vrp_large_ranges();
    printf("  VRP test result: 0x%02X\n", vrp_result);
    /* This test may return 0 if x doesn't meet conditions at runtime */
    printf("  [INFO - VRP tests are compile-time focused]\n");
    
    int iv_result = test_induction_variables();
    printf("  Induction variables test result: %d\n", iv_result);
    if (iv_result > 0) {
        printf("  [PASS - tested loop induction]\n");
        total_score += 25;
        tests_passed++;
    }
    
#ifdef __cplusplus
    printf("\nTest 4: Template Metaprogramming (C++ only)\n");
    printf("  Templates instantiated with large values\n");
    printf("  [INFO - Template comparisons done at compile time]\n");
    total_score += 25;
    tests_passed++;
#endif
    
    printf("\n====================================\n");
    printf("Total tests passed: %d\n", tests_passed);
    printf("Total score: %d/100\n", total_score);
    
    if (total_score >= 75) {
        printf("\nOVERALL RESULT: PASS\n");
        return 0;
    } else {
        printf("\nOVERALL RESULT: FAIL\n");
        return 1;
    }
}
