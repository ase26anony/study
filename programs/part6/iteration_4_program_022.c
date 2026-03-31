/* test_double_int_cmp.c - Comprehensive test to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE ((__int128_t)-1 << 80)
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 2)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");

/* Compile-time function using __builtin_constant_p */
static int compile_time_check(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > HUGE_NEGATIVE)) {
        return 1;
    }
    return 0;
}

/* ========== 2. GCC Builtins with Overflow Detection ========== */

/* Test overflow builtins that internally use double_int comparisons */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test: %lld * %lld = %lld (overflow: %d)\n", 
           a, b, res, overflow);
    
    /* Case 2: Addition with potential overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("Add overflow test: %lld + %lld = %lld (overflow: %d)\n",
           a, b, res, overflow);
    
    /* Case 3: Constant overflow check */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                      2, 0))) {
        printf("Constant overflow detection works\n");
    }
}

/* ========== 3. Range Analysis with Complex Conditions ========== */

/* Function with complex range analysis */
static void range_analysis_test(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons with the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y = %lld within expected range\n", y);
        }
        
        /* Test with large step values */
        for (int i = x; i < 1000000; i *= 2) {
            /* Loop analysis may use double_int for wrap-around checks */
            if (i > 1000000000) break;
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long z = (long long)x * x;
        if (z > 1000000) {
            printf("Negative range: z = %lld\n", z);
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
/* C++ template version for compile-time comparisons */
template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_half_max = N > ((__int128_t)1 << 65);
    static const bool less_than_min = N < ((__int128_t)-1 << 65);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((__int128_t)1 << 66)>;
template struct LargeCompare<((__int128_t)-1 << 66)>;
template struct LargeCompare<((__int128_t)0x7FFFFFFFFFFFFFFF * 3)>;
#endif

/* ========== 5. Wide Constants and Tree Node Construction ========== */

/* Use 128-bit types with attributes */
typedef __int128_t wide_int __attribute__((mode(TI)));

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE = ((__int128_t)1 << 70),
    BIGGER_VALUE = ((__int128_t)1 << 80),
    NEG_BIG_VALUE = ((__int128_t)-1 << 70)
};

/* Operations on wide integers that require magnitude comparison */
static wide_int wide_operations(wide_int a, wide_int b) {
    /* Division/modulus operations compare magnitudes */
    if (b != 0) {
        wide_int quotient = a / b;
        wide_int remainder = a % b;
        
        /* Comparisons during division */
        if (quotient > 0 && remainder < b) {
            return quotient;
        }
    }
    return a;
}

/* ========== 6. Additional Test Cases ========== */

/* Test all comparison operators with large integers */
static void test_all_comparisons(void) {
    __int128_t a = ((__int128_t)1 << 70) + 123;
    __int128_t b = ((__int128_t)1 << 70) + 456;
    __int128_t c = ((__int128_t)1 << 69);
    
    /* Force compiler to evaluate all comparison types */
    int results[6];
    results[0] = (a < b) ? 1 : 0;   // less than
    results[1] = (a > b) ? 1 : 0;   // greater than
    results[2] = (a <= b) ? 1 : 0;  // less or equal
    results[3] = (a >= b) ? 1 : 0;  // greater or equal
    results[4] = (a == b) ? 1 : 0;  // equal
    results[5] = (a != b) ? 1 : 0;  // not equal
    
    /* Also test with different high parts */
    results[0] |= (a > c) ? 2 : 0;
    results[1] |= (c < a) ? 2 : 0;
    
    printf("Comparison results: ");
    for (int i = 0; i < 6; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
}

/* Test shift operations that create large values */
static void test_large_shifts(void) {
    long long base = 1;
    
    /* Shifts that exceed 64 bits */
    __int128_t shifted[5];
    shifted[0] = (__int128_t)base << 32;
    shifted[1] = (__int128_t)base << 64;
    shifted[2] = (__int128_t)base << 96;
    shifted[3] = (__int128_t)base << 70;
    shifted[4] = (__int128_t)base << 80;
    
    /* Compare all shifted values */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i != j) {
                int cmp = (shifted[i] > shifted[j]) ? 1 : 
                         (shifted[i] < shifted[j]) ? -1 : 0;
                if (cmp != 0) {
                    printf("Shift compare: <<%d %s <<%d\n", 
                           (i+1)*16, cmp > 0 ? ">" : "<", (j+1)*16);
                }
            }
        }
    }
}

/* ========== Main Function ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Constant folding tests */
    printf("1. Constant folding tests:\n");
    printf("   VERY_LARGE_CONST = %llx (high bits)\n", 
           (unsigned long long)(VERY_LARGE_CONST >> 64));
    printf("   compile_time_check() = %d\n", compile_time_check());
    
    /* 2. Overflow builtin tests */
    printf("\n2. Overflow builtin tests:\n");
    test_overflow_builtins();
    
    /* 3. Range analysis tests */
    printf("\n3. Range analysis tests:\n");
    range_analysis_test(1500);
    range_analysis_test(-1500);
    
    /* 4. Wide constant operations */
    printf("\n4. Wide constant operations:\n");
    wide_int w1 = BIG_VALUE;
    wide_int w2 = BIGGER_VALUE;
    wide_int result = wide_operations(w2, w1);
    printf("   wide_operations result high bits: %llx\n",
           (unsigned long long)(result >> 64));
    
    /* 5. All comparison tests */
    printf("\n5. All comparison tests:\n");
    test_all_comparisons();
    
    /* 6. Large shift tests */
    printf("\n6. Large shift tests:\n");
    test_large_shifts();
    
    /* Additional: Test with random values */
    printf("\n7. Random value tests:\n");
    for (int i = 0; i < 5; i++) {
        long long r1 = rand() * (long long)rand();
        long long r2 = rand() * (long long)rand();
        __int128_t big1 = (__int128_t)r1 * r2;
        __int128_t big2 = (__int128_t)r2 * (r1 + 1);
        
        if (big1 > big2) {
            printf("   Random pair %d: big1 > big2\n", i);
        } else if (big1 < big2) {
            printf("   Random pair %d: big1 < big2\n", i);
        } else {
            printf("   Random pair %d: big1 == big2\n", i);
        }
    }
    
    printf("\n=== All tests completed ===\n");
    
    /* Final validation */
    if (VERY_LARGE_CONST > 0 && 
        HUGE_NEGATIVE < 0 && 
        LARGE_PRODUCT > INT64_MAX) {
        printf("PASS: All compile-time comparisons correct\n");
        return 0;
    } else {
        printf("FAIL: Compile-time comparison error\n");
        return 1;
    }
}
