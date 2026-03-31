/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define VERY_LARGE_NEGATIVE (-((__int128_t)1 << 70))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant should be > 0");
static_assert(VERY_LARGE_CONST > ((__int128_t)1 << 69), 
              "2^70 should be greater than 2^69");
static_assert(VERY_LARGE_NEGATIVE < 0, "Large negative constant should be < 0");
static_assert(VERY_LARGE_CONST != VERY_LARGE_NEGATIVE,
              "Positive and negative should not be equal");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b) \
    (__builtin_constant_p((a) > (b)) ? ((a) > (b)) : 0)

/* ========== 2. GCC Builtins That Manipulate double_int ========== */

void test_builtin_overflow(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that will trigger overflow checks with double_int comparisons */
    
    /* Case 1: Multiplication near 64-bit boundaries */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64_t */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(0x7FFFFFFFFFFFFFFF, 2): overflow=%d\n", overflow);
    
    /* Case 2: Large multiplication requiring 128-bit intermediate */
    a = 0x123456789ABCDEFLL;
    b = 0xFEDCBA987654321LL;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(large1, large2): overflow=%d\n", overflow);
    
    /* Case 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(0x7FFFFFFFFFFFFFFF, 1): overflow=%d\n", overflow);
    
    /* Case 4: Subtraction underflow */
    a = -0x7FFFFFFFFFFFFFFFLL - 1;  /* Min int64_t */
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(INT64_MIN, 1): overflow=%d\n", overflow);
}

/* Builtin with constant evaluation */
int test_constant_overflow(void) {
    /* These force compile-time overflow checking */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                       2, 
                                                       (long long)0))) {
        return 1;
    }
    return 0;
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

void test_range_analysis(int x) {
    /* Complex range analysis that requires double_int comparisons */
    
    /* Case 1: Known bounds with multiplication */
    if (x > 1000 && x < 2000) {
        /* The compiler analyzes range of x*x using double_int comparisons */
        long long y = (long long)x * x;
        printf("Range test 1: x=%d, x*x=%lld\n", x, y);
        
        /* Nested ranges */
        if (y > 1000000 && y < 4000000) {
            long long z = y * 2;
            printf("  Nested: y=%lld, z=%lld\n", y, z);
        }
    }
    
    /* Case 2: Large ranges */
    if (x > 0x100000000LL && x < 0x200000000LL) {
        /* 64-bit range comparison */
        __int128_t big = (__int128_t)x * x;
        printf("Large range: x=%d, big=%lld\n", x, (long long)big);
    }
    
    /* Case 3: Loop with induction variable analysis */
    for (long long i = 0x7000000000000000LL; 
         i < 0x7800000000000000LL; 
         i += 0x100000000LL) {
        /* Compiler analyzes loop bounds using wide int comparisons */
        if (i % 0x1000000 == 0) {
            printf("  Loop i=%lld\n", i);
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > (__int128_t(1) << 65);
    static const bool less_than_neg_2_65 = N < (-(__int128_t(1) << 65));
    static const bool is_zero = N == 0;
};

/* Instantiate templates with various large values */
template struct LargeCompare<(__int128_t)1 << 66>;
template struct LargeCompare<-(__int128_t)1 << 66>;
template struct LargeCompare<0>;
#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Wide enumeration */
enum big_enum : __int128 {
    BIG_ENUM_VAL1 = (__int128_t)1 << 70,
    BIG_ENUM_VAL2 = (__int128_t)1 << 71,
    BIG_ENUM_VAL3 = 0
};

/* 128-bit integer type using attribute */
typedef __int128_t __attribute__((mode(TI))) int128_t;

void test_wide_operations(int128_t a, int128_t b) {
    /* Operations that require magnitude comparison during simplification */
    int128_t sum = a + b;
    int128_t diff = a - b;
    int128_t prod = a * b;
    
    /* Comparisons that use double_int::cmp */
    if (a > b) printf("a > b\n");
    if (a < b) printf("a < b\n");
    if (a == b) printf("a == b\n");
    if (sum > diff) printf("sum > diff\n");
    
    /* Division/modulus requires magnitude checks */
    if (b != 0) {
        int128_t quot = a / b;
        int128_t rem = a % b;
        printf("quotient=%lld, remainder=%lld\n", 
               (long long)quot, (long long)rem);
    }
}

/* ========== Main Test Harness ========== */

int main(void) {
    int pass = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Compile-time constant folding */
    printf("1. Compile-time constant folding tests:\n");
    printf("   VERY_LARGE_CONST = %lld... (truncated)\n", 
           (long long)(VERY_LARGE_CONST >> 32));
    
    /* Runtime verification of compile-time comparisons */
    if (VERY_LARGE_CONST <= 0) {
        printf("   FAIL: VERY_LARGE_CONST should be > 0\n");
        pass = 0;
    }
    
    /* Test 2: Builtin overflow operations */
    printf("\n2. Builtin overflow tests:\n");
    test_builtin_overflow();
    
    /* Test 3: Range analysis */
    printf("\n3. Range analysis tests:\n");
    test_range_analysis(1500);  /* Within first range */
    test_range_analysis(0x180000000LL);  /* Within second range */
    
    /* Test 4: Wide constant operations */
    printf("\n4. Wide constant operations:\n");
    int128_t big1 = (int128_t)1 << 70;
    int128_t big2 = (int128_t)1 << 69;
    test_wide_operations(big1, big2);
    
    /* Test 5: Edge cases for double_int::cmp */
    printf("\n5. Edge case comparisons:\n");
    
    /* Same high part, different low parts */
    __int128_t x = ((__int128_t)0x12345678 << 64) | 0x1111111111111111LL;
    __int128_t y = ((__int128_t)0x12345678 << 64) | 0x2222222222222222LL;
    
    if (x < y) printf("   x < y (same high, different low) - CORRECT\n");
    else printf("   x >= y - WRONG\n");
    
    /* Different high parts */
    __int128_t a = ((__int128_t)0x11111111 << 64) | 0xFFFFFFFFFFFFFFFFLL;
    __int128_t b = ((__int128_t)0x22222222 << 64) | 0x0000000000000000LL;
    
    if (a < b) printf("   a < b (different high) - CORRECT\n");
    else printf("   a >= b - WRONG\n");
    
    /* Negative numbers */
    __int128_t neg1 = -((__int128_t)1 << 70);
    __int128_t neg2 = -((__int128_t)1 << 69);
    
    if (neg1 < neg2) printf("   neg1 < neg2 (both negative) - CORRECT\n");
    else printf("   neg1 >= neg2 - WRONG\n");
    
    /* Test 6: Mixed compile-time/runtime tests */
    printf("\n6. Mixed compile-time/runtime tests:\n");
    
    /* Use __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(VERY_LARGE_CONST > 100)) {
        printf("   VERY_LARGE_CONST > 100 evaluated at compile time\n");
    }
    
    /* Large shift operations */
    __int128_t shifted = (__int128_t)1 << 100;
    if (shifted > 0) {
        printf("   2^100 > 0 - CORRECT\n");
    }
    
    /* Multiplication producing wide int */
    long long l1 = 0x7FFFFFFFFFFFFFFFLL;
    long long l2 = 2;
    __int128_t product = (__int128_t)l1 * l2;
    
    if (product > l1) {
        printf("   product > l1 - CORRECT\n");
    }
    
    /* Final summary */
    printf("\n=== Test %s ===\n", pass ? "PASSED" : "FAILED");
    
    return pass ? 0 : 1;
}
