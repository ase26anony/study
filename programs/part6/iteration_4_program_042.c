/* test_double_int_cmp.c - Comprehensive test for GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Use __int128 for 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t HUGE_POS = ((int128_t)1 << 100);
const int128_t HUGE_NEG = -((int128_t)1 << 100);

/* Static assertions that force compile-time comparisons */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* These will trigger double_int::cmp during constant folding */
STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(HUGE_POS > VERY_LARGE_POS);
STATIC_ASSERT(HUGE_NEG < VERY_LARGE_NEG);
STATIC_ASSERT(VERY_LARGE_POS != VERY_LARGE_NEG);
STATIC_ASSERT(((int128_t)1 << 65) < ((int128_t)1 << 66));

/* Complex constant expressions */
STATIC_ASSERT((VERY_LARGE_POS + 1) > VERY_LARGE_POS);
STATIC_ASSERT((VERY_LARGE_NEG - 1) < VERY_LARGE_NEG);
STATIC_ASSERT((VERY_LARGE_POS >> 1) < VERY_LARGE_POS);
STATIC_ASSERT((VERY_LARGE_NEG >> 1) > VERY_LARGE_NEG);

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that should trigger overflow comparisons */
    a = LLONG_MAX;
    b = 2;
    
    /* __builtin_mul_overflow uses double_int internally */
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2) = %d (res: %lld)\n", overflow, res);
    
    /* Test with constants that force double_int comparison */
    a = 1LL << 62;
    b = 1LL << 62;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(2^62, 2^62) = %d (res: %lld)\n", overflow, res);
    
    /* Test __builtin_add_overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1) = %d (res: %lld)\n", overflow, res);
    
    /* Test __builtin_sub_overflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1) = %d (res: %lld)\n", overflow, res);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, 0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Functions that create complex value ranges */
void test_range_calculations(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication creates a range that needs double_int comparison */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test passed: x=%d, y=%lld\n", x, y);
        }
    }
    
    /* Test with larger numbers */
    if (x > 1000000 && x < 2000000) {
        /* This will create a very large range */
        int128_t big_y = (int128_t)x * x * x;
        
        /* Comparisons that need double_int */
        if (big_y > ((int128_t)1 << 60)) {
            printf("Large range test: x=%d, big_y is huge\n", x);
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int128_t i = 0; i < ((int128_t)1 << 30); i += (1 << 20)) {
        /* The loop analysis will compare i against the limit using double_int */
        if (i > ((int128_t)1 << 29)) {
            printf("Induction variable reached threshold\n");
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool greater_than_zero = N > 0;
    static const bool less_than_huge = N < ((int128_t)1 << 65);
    static const bool equal_to_self = N == N;
    static const bool not_equal = N != (N + 1);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((int128_t)1 << 64)>;
template struct LargeCompare<-((int128_t)1 << 64)>;
template struct LargeCompare<((int128_t)1 << 63)>;
template struct LargeCompare<((int128_t)1 << 63) + 1>;

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use __int128 with attributes */
typedef __int128 int128_attr __attribute__((mode(TI)));

/* Enumeration with large values */
enum big_enum : int128_t {
    BIG_VALUE = ((int128_t)1 << 70),
    BIGGER_VALUE = ((int128_t)1 << 71),
    BIG_NEGATIVE = -((int128_t)1 << 70)
};

/* Complex operations on wide constants */
int128_t complex_wide_operation(int128_t a, int128_t b) {
    /* Operations that require magnitude comparisons */
    if (a == 0) return b;
    if (b == 0) return a;
    
    /* Division requires comparing magnitudes */
    if (llabs((long long)(a >> 64)) > llabs((long long)(b >> 64))) {
        return a / b;
    } else {
        return b / a;
    }
}

/* ========== Main Test Function ========== */

int main(void) {
    int test_passes = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Basic large constant comparisons */
    printf("1. Testing large constant comparisons:\n");
    if (VERY_LARGE_POS > 0) {
        printf("  VERY_LARGE_POS > 0: PASS\n");
        test_passes++;
    }
    
    if (VERY_LARGE_NEG < 0) {
        printf("  VERY_LARGE_NEG < 0: PASS\n");
        test_passes++;
    }
    
    if (HUGE_POS > VERY_LARGE_POS) {
        printf("  HUGE_POS > VERY_LARGE_POS: PASS\n");
        test_passes++;
    }
    
    /* Test 2: Overflow builtins */
    printf("\n2. Testing overflow builtins:\n");
    test_overflow_builtins();
    test_passes++;
    
    /* Test 3: Range calculations */
    printf("\n3. Testing range calculations:\n");
    test_range_calculations(1500);
    test_range_calculations(1500000);
    test_passes++;
    
    /* Test 4: Induction variables */
    printf("\n4. Testing induction variables:\n");
    test_induction_variables();
    test_passes++;
    
    /* Test 5: Complex wide operations */
    printf("\n5. Testing complex wide operations:\n");
    int128_t result = complex_wide_operation(VERY_LARGE_POS, VERY_LARGE_POS >> 1);
    printf("  complex_wide_operation result computed\n");
    test_passes++;
    
    /* Additional edge cases */
    printf("\n6. Testing edge cases:\n");
    
    /* Compare values with same high part but different low parts */
    int128_t same_high_diff_low1 = ((int128_t)0x12345678 << 64) | 0x1;
    int128_t same_high_diff_low2 = ((int128_t)0x12345678 << 64) | 0x2;
    
    if (same_high_diff_low1 < same_high_diff_low2) {
        printf("  Same high, different low comparison: PASS\n");
        test_passes++;
    }
    
    /* Compare maximum and minimum values */
    int128_t max_128 = ~((int128_t)0) >> 1;
    int128_t min_128 = ~max_128;
    
    if (min_128 < max_128) {
        printf("  Min < Max comparison: PASS\n");
        test_passes++;
    }
    
    /* Test with __builtin_constant_p */
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        printf("  __builtin_constant_p comparison: PASS\n");
        test_passes++;
    }
    
    printf("\n=== All tests completed ===\n");
    printf("Total checks passed: %d\n", test_passes);
    
    return 0;
}
