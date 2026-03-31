/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Constant Folding with Large Integers ========== */

/* Static assertions with large 128-bit constants */
#define STATIC_ASSERT(cond) typedef char static_assert_##__LINE__[(cond) ? 1 : -1]

/* Large constants that require double_int representation */
const __int128_t VERY_LARGE_POS = ((__int128_t)1 << 70);
const __int128_t VERY_LARGE_NEG = -((__int128_t)1 << 70);
const __int128_t HUGE_PRODUCT = ((__int128_t)0x7FFFFFFFFFFFFFFFLL) * 2;

/* Compile-time comparisons */
STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(VERY_LARGE_POS > VERY_LARGE_NEG);
STATIC_ASSERT(HUGE_PRODUCT > INT64_MAX);

/* Function to force constant folding in different contexts */
static int test_constant_folding(void) {
    int result = 0;
    
    /* Comparisons in constant contexts */
    if (__builtin_constant_p(VERY_LARGE_POS > 1000)) {
        result |= 1;
    }
    
    /* Arithmetic that produces wide integers */
    __int128_t a = ((__int128_t)INT64_MAX) << 5;
    __int128_t b = ((__int128_t)INT64_MAX) << 3;
    
    if (a > b) result |= 2;
    if (b < a) result |= 4;
    if (a != b) result |= 8;
    
    /* Edge cases around zero */
    __int128_t near_zero_pos = ((__int128_t)1) << 63;
    __int128_t near_zero_neg = -near_zero_pos;
    
    if (near_zero_pos > 0) result |= 16;
    if (near_zero_neg < 0) result |= 32;
    
    return result;
}

/* ========== 2. GCC Builtins with Overflow Detection ========== */

static int test_overflow_builtins(void) {
    int result = 0;
    
    /* Multiplication overflow checks */
    int64_t x = INT64_MAX;
    int64_t y = 2;
    int64_t prod;
    
    if (__builtin_mul_overflow(x, y, &prod)) {
        result |= 1;  /* Overflow occurred */
    }
    
    /* Addition overflow with large values */
    int64_t a = INT64_MAX;
    int64_t b = 1;
    int64_t sum;
    
    if (__builtin_add_overflow(a, b, &sum)) {
        result |= 2;
    }
    
    /* Subtraction overflow */
    int64_t c = INT64_MIN;
    int64_t d = 1;
    int64_t diff;
    
    if (__builtin_sub_overflow(c, d, &diff)) {
        result |= 4;
    }
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, (int64_t)0))) {
        result |= 8;
    }
    
    return result;
}

/* ========== 3. Range Calculations and VRP ========== */

static int test_range_analysis(void) {
    int result = 0;
    
    /* Complex range analysis */
    for (int64_t i = 1000; i < 2000; i += 100) {
        /* Multiplication that creates wide range */
        int64_t j = i * i;
        
        /* Nested conditions for VRP */
        if (j > 1000000 && j < 4000000) {
            result |= 1;
        }
    }
    
    /* Induction variable with potential wrap-around */
    int64_t x = 0;
    for (int i = 0; i < 100; i++) {
        x += INT64_MAX / 100;
        if (x > INT64_MAX / 2) {
            result |= 2;
        }
    }
    
    /* Range comparisons */
    int64_t low = INT64_MIN + 1000;
    int64_t high = INT64_MAX - 1000;
    
    if (low < high) result |= 4;
    if (low != high) result |= 8;
    
    /* Signed-unsigned comparisons that require careful analysis */
    uint64_t u = UINT64_MAX;
    int64_t s = -1;
    
    if ((__int128_t)u > (__int128_t)s) {
        result |= 16;
    }
    
    return result;
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((__int128_t)1 << 65);
    static const bool is_very_large = N > ((__int128_t)1 << 100);
    
    static const __int128_t doubled = N * 2;
    static const bool doubled_is_larger = doubled > N;
};

/* Instantiate templates with various large values */
template struct LargeCompare<((__int128_t)1) << 66>;
template struct LargeCompare<-((__int128_t)1) << 66>;
template struct LargeCompare<((__int128_t)0x7FFFFFFFFFFFFFFF) * 3>;

#endif

/* ========== 5. Tree Node Construction for Wide Constants ========== */

/* Use attribute for 128-bit integers */
typedef __int128_t int128 __attribute__((mode(TI)));

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = ((__int128_t)1) << 70,
    BIG_ENUM_B = ((__int128_t)1) << 71,
    BIG_ENUM_C = BIG_ENUM_A * 2
};

/* Operations on wide constants that require magnitude comparison */
static int test_wide_operations(void) {
    int result = 0;
    
    int128 a = BIG_ENUM_A;
    int128 b = BIG_ENUM_B;
    
    /* Division and modulus operations */
    if (b / a == 2) result |= 1;
    if (b % a == 0) result |= 2;
    
    /* Bitwise operations */
    int128 shifted = a << 1;
    if (shifted == b) result |= 4;
    
    /* Comparisons across different representations */
    unsigned __int128 ua = (unsigned __int128)a;
    unsigned __int128 ub = (unsigned __int128)b;
    
    if (ua < ub) result |= 8;
    if (ua != ub) result |= 16;
    
    return result;
}

/* ========== Main Test Driver ========== */

int main(void) {
    int total_score = 0;
    int failures = 0;
    
    printf("Testing double_int::cmp coverage...\n");
    
    /* Test 1: Constant Folding */
    printf("1. Testing constant folding... ");
    int cf_result = test_constant_folding();
    if (cf_result == 0x3F) {  /* All bits set from our test */
        printf("PASS\n");
        total_score++;
    } else {
        printf("FAIL (got 0x%x)\n", cf_result);
        failures++;
    }
    
    /* Test 2: Overflow Builtins */
    printf("2. Testing overflow builtins... ");
    int of_result = test_overflow_builtins();
    if (of_result & 0x0F) {  /* At least some overflow detected */
        printf("PASS\n");
        total_score++;
    } else {
        printf("FAIL (got 0x%x)\n", of_result);
        failures++;
    }
    
    /* Test 3: Range Analysis */
    printf("3. Testing range analysis... ");
    int ra_result = test_range_analysis();
    if (ra_result & 0x1F) {  /* Multiple conditions triggered */
        printf("PASS\n");
        total_score++;
    } else {
        printf("FAIL (got 0x%x)\n", ra_result);
        failures++;
    }
    
    /* Test 4: Wide Operations */
    printf("4. Testing wide operations... ");
    int wo_result = test_wide_operations();
    if (wo_result == 0x1F) {  /* All bits set */
        printf("PASS\n");
        total_score++;
    } else {
        printf("FAIL (got 0x%x)\n", wo_result);
        failures++;
    }
    
    /* Test 5: Static Assertions (compile-time) */
    printf("5. Static assertions passed at compile-time: PASS\n");
    total_score++;
    
    printf("\nSummary: %d/5 tests passed, %d failures\n", total_score, failures);
    
    /* Runtime validation of compile-time comparisons */
    printf("\nRuntime validation:\n");
    
    __int128_t x = ((__int128_t)1) << 70;
    __int128_t y = ((__int128_t)1) << 69;
    
    printf("  (1 << 70) > (1 << 69): %s\n", 
           x > y ? "CORRECT" : "ERROR");
    printf("  (1 << 69) < (1 << 70): %s\n", 
           y < x ? "CORRECT" : "ERROR");
    printf("  (1 << 70) != (1 << 69): %s\n", 
           x != y ? "CORRECT" : "ERROR");
    
    /* Test all comparison operators */
    int cmp_results = 0;
    cmp_results |= (x > y) ? 1 : 0;
    cmp_results |= (x >= y) ? 2 : 0;
    cmp_results |= (x < y) ? 4 : 0;
    cmp_results |= (x <= y) ? 8 : 0;
    cmp_results |= (x == y) ? 16 : 0;
    cmp_results |= (x != y) ? 32 : 0;
    
    printf("  Comparison operators working: %s\n",
           cmp_results == 0x23 ? "YES" : "NO");  /* 0x23 = 00100011 = >, >=, != */
    
    return failures == 0 ? 0 : 1;
}
