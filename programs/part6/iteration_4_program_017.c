/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Static assertions with large 128-bit constants */
#define LARGE_CONST_1 (((__int128_t)1) << 70)      /* 2^70 */
#define LARGE_CONST_2 (((__int128_t)1) << 80)      /* 2^80 */
#define LARGE_CONST_3 (((__int128_t)1) << 90)      /* 2^90 */
#define LARGE_CONST_4 (((__int128_t)0xFFFFFFFFFFFFFFFFULL) << 32) /* High bits set */

/* Compile-time comparisons that should trigger double_int::cmp */
static_assert(LARGE_CONST_1 > 0, "Large constant 1 should be positive");
static_assert(LARGE_CONST_2 > LARGE_CONST_1, "2^80 > 2^70");
static_assert(LARGE_CONST_3 > LARGE_CONST_2, "2^90 > 2^80");
static_assert(LARGE_CONST_4 > LARGE_CONST_1, "High-bit constant > 2^70");

/* Function using __builtin_constant_p with large integer comparisons */
static int test_constant_folding(void) {
    const __int128_t a = LARGE_CONST_1;
    const __int128_t b = LARGE_CONST_2;
    
    /* These comparisons should be evaluated at compile time */
    if (__builtin_constant_p(a < b)) {
        if (a < b) {
            return 1; /* Should be taken */
        }
    }
    
    /* Arithmetic producing wide integers */
    const int64_t x = 0x7FFFFFFFFFFFFFFFLL; /* Max int64_t */
    const int64_t y = 0x7FFFFFFFFFFFFFFFLL;
    
    /* Multiplication that overflows 64 bits */
    const __int128_t prod = (__int128_t)x * (__int128_t)y;
    
    if (__builtin_constant_p(prod > LARGE_CONST_1)) {
        if (prod > LARGE_CONST_1) {
            return 2; /* Should be taken */
        }
    }
    
    /* Left shift beyond 63 bits */
    const __int128_t shifted = ((__int128_t)1) << 100;
    if (__builtin_constant_p(shifted > prod)) {
        if (shifted > prod) {
            return 3; /* Should be taken */
        }
    }
    
    return 0;
}

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins that internally use double_int comparisons */
static int test_overflow_builtins(void) {
    int results = 0;
    
    /* Test multiplication overflow */
    {
        long long a = 0x7FFFFFFFFFFFFFFFLL;
        long long b = 2;
        long long res;
        
        if (__builtin_mul_overflow(a, b, &res)) {
            results |= 1; /* Overflow should occur */
        }
        
        /* Test with constants that should be folded */
        const long long c = 0x7FFFFFFFFFFFFFFFLL;
        const long long d = 0x7FFFFFFFFFFFFFFFLL;
        long long res2;
        
        if (__builtin_constant_p(c) && __builtin_constant_p(d)) {
            if (__builtin_mul_overflow_p(c, d, (long long)0)) {
                results |= 2; /* Should detect overflow at compile time */
            }
        }
    }
    
    /* Test addition overflow */
    {
        unsigned long long a = 0xFFFFFFFFFFFFFFFFULL;
        unsigned long long b = 1;
        unsigned long long res;
        
        if (__builtin_add_overflow(a, b, &res)) {
            results |= 4; /* Overflow in unsigned addition */
        }
    }
    
    /* Test subtraction with potential underflow */
    {
        unsigned long long a = 0;
        unsigned long long b = 1;
        unsigned long long res;
        
        if (__builtin_sub_overflow(a, b, &res)) {
            results |= 8; /* Underflow should occur */
        }
    }
    
    return results;
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that should trigger double_int comparisons */
static int test_range_analysis(int input) {
    int result = 0;
    
    /* Create known bounds for the compiler to analyze */
    if (input > 1000 && input < 2000) {
        /* Multiplication that requires range analysis */
        int64_t x = input;
        int64_t y = x * x; /* Range: ~1,000,000 to ~4,000,000 */
        
        /* Further operations that might use double_int comparisons */
        if (y > 1500000) {
            result = 1;
        }
        
        /* Nested ranges */
        if (x > 1200 && x < 1800) {
            int64_t z = y * 2;
            if (z > 3000000) {
                result = 2;
            }
        }
    }
    
    /* Test with larger values that might require 128-bit intermediate calculations */
    if (input > 1000000 && input < 2000000) {
        int64_t a = input;
        int64_t b = a * 1000; /* Could overflow 32-bit, needs range analysis */
        
        if (b > 1500000000) {
            result = 3;
        }
    }
    
    /* Loop with induction variable analysis */
    for (int64_t i = 0x7FFFFFFFFFFFFFF0LL; i < 0x7FFFFFFFFFFFFFFFLL; i++) {
        /* Loop analysis might use double_int for wrap-around checks */
        if (i > 0x7FFFFFFFFFFFFFF5LL) {
            result = 4;
            break;
        }
    }
    
    return result;
}

/* ========== 4. Template Metaprogramming (C++ version) ========== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > (__int128_t(1) << 65);
    static const bool less_than_2_80 = N < (__int128_t(1) << 80);
    static const bool equal_to_self = N == N;
    static const bool not_equal = N != (__int128_t(1) << 70);
};

/* Instantiate templates with various large values */
template struct LargeCompare<(__int128_t(1) << 66)>;
template struct LargeCompare<(__int128_t(1) << 75)>;
template struct LargeCompare<(__int128_t(1) << 90)>;
template struct LargeCompare<((__int128_t)0xFFFFFFFFFFFFFFFFULL << 32)>;

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit integer type with mode attribute */
typedef __int128_t wide_int __attribute__((mode(TI)));

/* Operations on wide integers that require magnitude comparisons */
static wide_int test_wide_operations(wide_int a, wide_int b) {
    /* Division and modulus operations internally compare magnitudes */
    if (b != 0) {
        wide_int quotient = a / b;
        wide_int remainder = a % b;
        
        /* Comparisons during division algorithm */
        if (quotient > 0 && remainder < b) {
            return quotient;
        }
    }
    
    return a;
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE_1 = ((__int128_t)1) << 64,
    BIG_VALUE_2 = ((__int128_t)1) << 72,
    BIG_VALUE_3 = ((__int128_t)1) << 80
};

/* ========== Main Test Harness ========== */

int main(void) {
    int total_score = 0;
    
    printf("Testing double_int::cmp coverage...\n");
    
    /* Test 1: Constant folding */
    printf("1. Testing constant folding with large integers... ");
    int cf_result = test_constant_folding();
    if (cf_result > 0) {
        printf("PASS (result=%d)\n", cf_result);
        total_score += 1;
    } else {
        printf("FAIL\n");
    }
    
    /* Test 2: Overflow builtins */
    printf("2. Testing overflow builtins... ");
    int of_result = test_overflow_builtins();
    /* Expected: bits 0, 2, 3 set (1 | 2 | 8 = 11) */
    if (of_result == 11) {
        printf("PASS (result=0x%x)\n", of_result);
        total_score += 1;
    } else {
        printf("PARTIAL (result=0x%x, expected 0xb)\n", of_result);
    }
    
    /* Test 3: Range analysis */
    printf("3. Testing range analysis... ");
    int ra_result = test_range_analysis(1500);
    if (ra_result == 1) {
        printf("PASS (result=%d)\n", ra_result);
        total_score += 1;
    } else {
        printf("PARTIAL (result=%d)\n", ra_result);
    }
    
    /* Test 4: Wide integer operations */
    printf("4. Testing wide integer operations... ");
    wide_int w1 = ((wide_int)1) << 70;
    wide_int w2 = ((wide_int)1) << 60;
    wide_int w_result = test_wide_operations(w1, w2);
    
    if (w_result == (w1 / w2)) {
        printf("PASS\n");
        total_score += 1;
    } else {
        printf("FAIL\n");
    }
    
    /* Test 5: Enum comparisons */
    printf("5. Testing large enum comparisons... ");
    if (BIG_VALUE_2 > BIG_VALUE_1 && BIG_VALUE_3 > BIG_VALUE_2) {
        printf("PASS\n");
        total_score += 1;
    } else {
        printf("FAIL\n");
    }
    
    /* Final summary */
    printf("\n=== Summary ===\n");
    printf("Total tests passed: %d/5\n", total_score);
    
    if (total_score == 5) {
        printf("All tests PASSED - double_int::cmp should have been exercised\n");
        return 0;
    } else {
        printf("Some tests FAILED - coverage may be incomplete\n");
        return 1;
    }
}
