/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE ((__int128_t)-1 << 80)
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant comparison");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant comparison");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit range");

/* Template-like macro for compile-time comparison */
#define COMPARE_CONSTANTS(a, b) \
    do { \
        if (__builtin_constant_p((a) > (b))) { \
            static_assert((a) > (b), "Compile-time comparison failed"); \
        } \
    } while(0)

/* ========== 2. GCC Builtins That Use double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Multiplication that may overflow */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d, res=%lld\n", overflow, res);
    
    /* Addition with potential overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d, res=%lld\n", overflow, res);
    
    /* Subtraction with underflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d, res=%lld\n", overflow, res);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that uses double_int comparisons */
void test_range_calculations(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further range refinement */
        if (y > 1500000 && y < 3000000) {
            long long z = y * 2;
            printf("Range test: x=%d, y=%lld, z=%lld\n", x, y, z);
        }
    }
    
    /* Test with large ranges */
    if (x > -1000000000 && x < 1000000000) {
        /* Multiplication that could exceed 64-bit in intermediate calculations */
        __int128_t big = (__int128_t)x * 1000000000000LL;
        if (big > 0) {
            printf("Large range positive: %lld\n", (long long)(big >> 32));
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = 0; i < 1000000000000LL; i += 1000000000LL) {
        /* Compiler analyzes loop bounds using wide integers */
        if (i % 10000000000LL == 0) {
            printf("Induction: %lld\n", (long long)i);
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_max = N > (__int128_t(1) << 65);
    static const bool less_than_min = N < -((__int128_t(1) << 65));
    static const bool in_range = (N >= -((__int128_t(1) << 62))) && 
                                 (N <= ((__int128_t(1) << 62)));
};

/* Instantiate templates with various large values */
template struct LargeCompare<( __int128_t(1) << 70)>;
template struct LargeCompare<-(__int128_t(1) << 70)>;
template struct LargeCompare<0x7FFFFFFFFFFFFFFFLL * 3>;

#endif

/* ========== 5. Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_t_attr;

/* Operations on wide constants that require magnitude comparison */
void test_wide_constant_operations(void) {
    /* Division of large values - requires comparing magnitudes */
    const __int128_t huge1 = ((__int128_t)1 << 100);
    const __int128_t huge2 = ((__int128_t)1 << 90);
    
    if (huge1 > huge2) {
        __int128_t quotient = huge1 / huge2;
        printf("Division of large constants: %lld / %lld = %lld\n",
               (long long)(huge1 >> 64), (long long)(huge2 >> 64),
               (long long)(quotient >> 64));
    }
    
    /* Modulus operation with wide integers */
    const __int128_t large_mod = ((__int128_t)1 << 80) + 123;
    const __int128_t mod_base = ((__int128_t)1 << 60);
    
    if (large_mod > mod_base) {
        __int128_t remainder = large_mod % mod_base;
        printf("Modulus: remainder = %lld\n", (long long)remainder);
    }
    
    /* Bitfield-like operations */
    struct {
        unsigned long long low : 64;
        unsigned long long high : 64;
    } wide_struct = { .low = ULLONG_MAX, .high = 1 };
    
    /* Access triggers internal conversions to double_int */
    unsigned long long combined = wide_struct.low + (wide_struct.high << 32);
    printf("Bitfield access: %llu\n", combined);
}

/* ========== 6. Additional Comprehensive Tests ========== */

/* Test all comparison operators with wide integers */
void test_all_comparisons(__int128_t a, __int128_t b) {
    printf("Comparison tests:\n");
    printf("  %lld <  %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a < b);
    printf("  %lld >  %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a > b);
    printf("  %lld <= %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a <= b);
    printf("  %lld >= %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a >= b);
    printf("  %lld == %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a == b);
    printf("  %lld != %lld : %d\n", (long long)(a >> 64), (long long)(b >> 64), a != b);
}

/* Mixed-size comparisons */
void test_mixed_size_comparisons(void) {
    int64_t medium = LLONG_MAX;
    __int128_t large = ((__int128_t)medium) * 2;
    
    /* These comparisons require converting 64-bit to 128-bit */
    if (large > medium) {
        printf("Mixed size: 128-bit > 64-bit\n");
    }
    
    if (medium < large) {
        printf("Mixed size: 64-bit < 128-bit\n");
    }
    
    /* Chain comparisons */
    int64_t small = 100;
    if (small < medium && medium < large) {
        printf("Chain comparison holds\n");
    }
}

/* ========== Main Function ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test overflow builtins */
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 2. Test range calculations */
    printf("2. Testing range calculations:\n");
    test_range_calculations(1500);
    printf("\n");
    
    /* 3. Test wide constant operations */
    printf("3. Testing wide constant operations:\n");
    test_wide_constant_operations();
    printf("\n");
    
    /* 4. Test all comparisons */
    printf("4. Testing all comparison operators:\n");
    __int128_t val1 = ((__int128_t)1 << 70);
    __int128_t val2 = ((__int128_t)1 << 69);
    test_all_comparisons(val1, val2);
    printf("\n");
    
    /* 5. Test mixed size comparisons */
    printf("5. Testing mixed size comparisons:\n");
    test_mixed_size_comparisons();
    printf("\n");
    
    /* 6. Test induction variables */
    printf("6. Testing induction variables (first few iterations):\n");
    for (int64_t i = 0; i < 5000000000LL && i < 1000000000000LL; i += 1000000000LL) {
        printf("  i = %lld\n", (long long)i);
    }
    printf("\n");
    
    /* Verify compile-time comparisons worked */
    printf("7. Verifying compile-time comparisons:\n");
    COMPARE_CONSTANTS(VERY_LARGE_CONST, 0);
    COMPARE_CONSTANTS(0, HUGE_NEGATIVE);
    printf("  All compile-time comparisons passed\n");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
