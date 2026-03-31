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
static_assert(VERY_LARGE_NEGATIVE < 0, "Large negative constant should be < 0");
static_assert(VERY_LARGE_CONST > VERY_LARGE_NEGATIVE, 
              "Positive should be greater than negative");
static_assert(LARGE_PRODUCT > INT64_MAX, 
              "Product should exceed 64-bit maximum");

/* Compile-time function using __builtin_constant_p */
static int compile_time_compare(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > 1000)) {
        return 1;
    }
    if (__builtin_constant_p(VERY_LARGE_NEGATIVE < -1000)) {
        return 2;
    }
    
    /* These comparisons should trigger double_int::cmp */
    const __int128_t a = ((__int128_t)1 << 65) + 1;
    const __int128_t b = ((__int128_t)1 << 65);
    if (__builtin_constant_p(a > b)) {
        return 3;
    }
    if (__builtin_constant_p(a != b)) {
        return 4;
    }
    
    return 0;
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
static void test_overflow_builtins(void) {
    int64_t large1 = INT64_MAX;
    int64_t large2 = 2;
    int64_t result;
    int overflow;
    
    /* Multiplication that overflows 64-bit */
    overflow = __builtin_mul_overflow(large1, large2, &result);
    printf("mul_overflow: %lld * 2 = %lld (overflow: %d)\n", 
           (long long)large1, (long long)result, overflow);
    
    /* Addition that overflows */
    int64_t max_val = INT64_MAX;
    int64_t one = 1;
    overflow = __builtin_add_overflow(max_val, one, &result);
    printf("add_overflow: %lld + 1 = %lld (overflow: %d)\n",
           (long long)max_val, (long long)result, overflow);
    
    /* Subtraction that underflows */
    int64_t min_val = INT64_MIN;
    overflow = __builtin_sub_overflow(min_val, one, &result);
    printf("sub_overflow: %lld - 1 = %lld (overflow: %d)\n",
           (long long)min_val, (long long)result, overflow);
    
    /* Test with __int128 to force double_int usage */
    __int128_t huge1 = ((__int128_t)INT64_MAX) * 1000;
    __int128_t huge2 = ((__int128_t)INT64_MAX) * 999;
    __int128_t huge_result;
    
    /* These comparisons during overflow checking use double_int::cmp */
    if (__builtin_mul_overflow_p(huge1, huge2, (__int128_t)0)) {
        printf("mul_overflow_p predicts overflow for huge values\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that requires double_int comparisons */
static void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication creates range that needs double_int comparison */
        long long y = (long long)x * x;
        
        /* Further range refinement */
        if (y > 1500000 && y < 3000000) {
            long long z = y * 2;
            printf("Range analysis: x=%d, y=%lld, z=%lld\n", x, y, z);
        }
    }
    
    /* Test with potential overflow in range calculation */
    int a = 1000000;
    int b = 1000000;
    if (a > 0 && b > 0) {
        long long product = (long long)a * b;
        /* Range analysis for product comparison */
        if (product > 500000000LL && product < 2000000000LL) {
            printf("Product in range: %lld\n", product);
        }
    }
}

/* Loop with induction variable analysis */
static void test_induction_variables(void) {
    for (int64_t i = INT64_MAX - 100; i < INT64_MAX; i += 10) {
        /* Loop analysis requires comparing i with INT64_MAX */
        if (i > INT64_MAX - 50) {
            printf("Near overflow: %lld\n", (long long)i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((__int128_t)1 << 65);
    static const bool is_very_large = N > ((__int128_t)1 << 100);
    
    /* Compare with another large value */
    template <__int128_t M>
    static const bool greater_than = N > M;
    
    template <__int128_t M>
    static const bool equal_to = N == M;
};

/* Instantiate templates with various large values */
using Compare1 = LargeCompare<((__int128_t)1 << 70)>;
using Compare2 = LargeCompare<((__int128_t)1 << 65) + 1>;
using Compare3 = LargeCompare<-((__int128_t)1 << 70)>;

/* Compile-time assertions using templates */
static_assert(Compare1::is_large, "Should be large");
static_assert(Compare1::is_very_large, "Should be very large");
static_assert(Compare1::greater_than<((__int128_t)1 << 65)>, 
              "Should be greater");
static_assert(Compare3::is_positive == false, "Should be negative");

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_ti;

/* Operations on wide types that require magnitude comparison */
static void test_wide_operations(void) {
    int128_ti huge1 = ((int128_ti)1 << 80) + 12345;
    int128_ti huge2 = ((int128_ti)1 << 80) + 12344;
    
    /* These comparisons should use double_int::cmp */
    if (huge1 > huge2) {
        printf("huge1 > huge2 (as expected)\n");
    }
    
    if (huge1 != huge2) {
        printf("huge1 != huge2 (as expected)\n");
    }
    
    /* Division/modulus operations require magnitude comparison */
    int128_ti quotient = huge1 / 2;
    int128_ti remainder = huge1 % 1000000;
    
    printf("Quotient: %llx%016llx, Remainder: %lld\n",
           (unsigned long long)(quotient >> 64),
           (unsigned long long)(quotient & 0xFFFFFFFFFFFFFFFF),
           (long long)remainder);
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE1 = ((__int128)1 << 66),
    BIG_VALUE2 = ((__int128)1 << 67),
    BIG_VALUE3 = BIG_VALUE1 + BIG_VALUE2
};

/* ========== Main Function ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test compile-time comparisons */
    printf("1. Compile-time comparisons:\n");
    int ct_result = compile_time_compare();
    printf("   compile_time_compare returned: %d\n", ct_result);
    
    /* 2. Test overflow builtins */
    printf("\n2. Overflow builtins:\n");
    test_overflow_builtins();
    
    /* 3. Test range analysis */
    printf("\n3. Range analysis:\n");
    test_range_analysis(1500);  /* Within range */
    test_range_analysis(500);   /* Outside range */
    
    /* 4. Test induction variables */
    printf("\n4. Induction variable analysis:\n");
    test_induction_variables();
    
    /* 5. Test wide operations */
    printf("\n5. Wide operations:\n");
    test_wide_operations();
    
    /* Runtime validation of compile-time comparisons */
    printf("\n6. Runtime validation:\n");
    
    /* Compare large 128-bit values at runtime */
    __int128_t runtime_large1 = ((__int128_t)1 << 70) + 123;
    __int128_t runtime_large2 = ((__int128_t)1 << 70) + 122;
    
    if (runtime_large1 > runtime_large2) {
        printf("   PASS: Runtime large comparison works\n");
    } else {
        printf("   FAIL: Runtime large comparison failed\n");
        return 1;
    }
    
    /* Test edge cases */
    __int128_t max128 = ((__int128_t)INT64_MAX << 64) | INT64_MAX;
    __int128_t min128 = ((__int128_t)INT64_MIN << 64);
    
    if (max128 > min128) {
        printf("   PASS: Max > Min comparison works\n");
    } else {
        printf("   FAIL: Max > Min comparison failed\n");
        return 1;
    }
    
    /* Test equality */
    __int128_t same1 = ((__int128_t)1 << 65) | 0x123456789ABCDEF;
    __int128_t same2 = ((__int128_t)1 << 65) | 0x123456789ABCDEF;
    
    if (same1 == same2) {
        printf("   PASS: Equality comparison works\n");
    } else {
        printf("   FAIL: Equality comparison failed\n");
        return 1;
    }
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
