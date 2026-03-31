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
static int check_large_constants(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > 100)) {
        return 1;
    }
    if (__builtin_constant_p(HUGE_NEGATIVE < -100)) {
        return 2;
    }
    return 0;
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Test cases designed to trigger overflow comparisons */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MAX, 2) = %d, overflow = %d\n", (int)res, overflow);
    
    a = INT64_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MIN, -1) = %d, overflow = %d\n", (int)res, overflow);
    
    /* Test with __int128 for larger range */
    __int128_t big_a = ((__int128_t)INT64_MAX) * 2;
    __int128_t big_b = ((__int128_t)INT64_MAX) * 3;
    int cmp = (big_a < big_b) ? -1 : (big_a > big_b);
    printf("__int128 comparison: %lld < %lld ? %d\n", 
           (long long)big_a, (long long)big_b, cmp);
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that uses double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons with the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test passed: y = %lld\n", y);
        }
    }
    
    /* Test with larger values that might overflow 64-bit */
    if (x > 1000000) {
        __int128_t big_y = (__int128_t)x * x * x;
        if (big_y > ((__int128_t)1 << 60)) {
            printf("Large range test: big_y > 2^60\n");
        }
    }
}

/* Loop with induction variables that have large step values */
void test_loop_induction(void) {
    for (__int128_t i = 0; i < ((__int128_t)1 << 70); i += ((__int128_t)1 << 40)) {
        /* The loop condition comparison uses double_int */
        if (i > ((__int128_t)1 << 60)) {
            printf("Loop induction: i = %lld (hex: %llx)\n", 
                   (long long)i, (long long)i);
            break; /* Don't actually run the full loop */
        }
    }
}

/* ========== 4. Template Metaprogramming with Large Values (C++) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > (__int128_t(1) << 65);
    static const bool less_than_neg_2_65 = N < -(__int128_t(1) << 65);
    static const int compare_to_zero = (N > 0) ? 1 : ((N < 0) ? -1 : 0);
};

/* Instantiate templates with various large values */
template struct LargeCompare<(__int128_t(1) << 66)>;
template struct LargeCompare<-(__int128_t(1) << 67)>;
template struct LargeCompare<0>;

/* Compile-time computation using templates */
template <__int128_t A, __int128_t B>
struct CompareValues {
    static const int result = (A > B) ? 1 : ((A < B) ? -1 : 0);
};

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use __int128 with operations that require magnitude comparison */
void test_wide_constant_operations(void) {
    __int128_t a = ((__int128_t)0x123456789ABCDEF0) << 32;
    __int128_t b = ((__int128_t)0xFEDCBA9876543210) >> 16;
    
    /* Operations that require comparing magnitudes */
    __int128_t sum = a + b;
    __int128_t diff = a - b;
    __int128_t prod = a / 2; /* Division requires magnitude comparison */
    
    printf("Wide constant operations:\n");
    printf("  a = %016llx%016llx\n", 
           (long long)(a >> 64), (long long)a);
    printf("  b = %016llx%016llx\n", 
           (long long)(b >> 64), (long long)b);
    printf("  a > b = %d\n", a > b);
    printf("  a == b = %d\n", a == b);
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = (__int128_t)1 << 65,
    BIG_ENUM_B = (__int128_t)1 << 66,
    BIG_ENUM_C = (__int128_t)1 << 67
};

/* ========== Main Test Harness ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test constant folding */
    printf("1. Constant folding tests:\n");
    printf("   VERY_LARGE_CONST > 0: %s\n", 
           VERY_LARGE_CONST > 0 ? "PASS" : "FAIL");
    printf("   HUGE_NEGATIVE < 0: %s\n", 
           HUGE_NEGATIVE < 0 ? "PASS" : "FAIL");
    printf("   check_large_constants() = %d\n", check_large_constants());
    printf("\n");
    
    /* 2. Test overflow builtins */
    printf("2. Overflow builtin tests:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 3. Test range analysis */
    printf("3. Range analysis tests:\n");
    test_range_analysis(1500);  /* Within range */
    test_range_analysis(10000000); /* Large value */
    printf("\n");
    
    /* 4. Test loop induction */
    printf("4. Loop induction test:\n");
    test_loop_induction();
    printf("\n");
    
    /* 5. Test wide constant operations */
    printf("5. Wide constant operation tests:\n");
    test_wide_constant_operations();
    printf("\n");
    
    /* Runtime validation of compile-time comparisons */
    printf("6. Runtime validation:\n");
    
    /* Test all comparison operators with large values */
    __int128_t x = ((__int128_t)1 << 70) + 5;
    __int128_t y = ((__int128_t)1 << 70) + 10;
    
    int results[6] = {
        x < y,   /* Should be 1 */
        x > y,   /* Should be 0 */
        x <= y,  /* Should be 1 */
        x >= y,  /* Should be 0 */
        x == y,  /* Should be 0 */
        x != y   /* Should be 1 */
    };
    
    const char* ops[] = {"<", ">", "<=", ">=", "==", "!="};
    int expected[] = {1, 0, 1, 0, 0, 1};
    
    int all_pass = 1;
    for (int i = 0; i < 6; i++) {
        if (results[i] != expected[i]) {
            printf("  FAIL: x %s y = %d (expected %d)\n", 
                   ops[i], results[i], expected[i]);
            all_pass = 0;
        }
    }
    
    if (all_pass) {
        printf("  All runtime comparisons PASS\n");
    }
    
    /* Test with negative large values */
    __int128_t neg_big = -((__int128_t)1 << 80);
    __int128_t pos_small = 100;
    
    printf("  Negative large comparison: %lld %s 100\n", 
           (long long)neg_big, neg_big < pos_small ? "<" : ">");
    
    printf("\n=== All tests completed ===\n");
    
    return all_pass ? 0 : 1;
}
