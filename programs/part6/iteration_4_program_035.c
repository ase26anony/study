/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE (-((__int128_t)1 << 68))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 4)

/* Static assertions force compile-time comparisons */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");

/* Compile-time function using __builtin_constant_p */
static int compile_time_check(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > ((__int128_t)1 << 65))) {
        return 1;
    }
    if (__builtin_constant_p(HUGE_NEGATIVE < -((__int128_t)1 << 67))) {
        return 2;
    }
    return 0;
}

/* ========== 2. GCC Builtins That Manipulate double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MAX, 2): overflow=%d, res=%lld\n", 
           overflow, (long long)res);
    
    /* Case 2: Addition with potential overflow */
    a = INT64_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(INT64_MAX, 1): overflow=%d, res=%lld\n",
           overflow, (long long)res);
    
    /* Case 3: Subtraction with underflow */
    a = INT64_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(INT64_MIN, 1): overflow=%d, res=%lld\n",
           overflow, (long long)res);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, (int64_t)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis triggering double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        int64_t y = (int64_t)x * x;
        
        /* Further comparisons with large bounds */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y=%lld in valid range\n", (long long)y);
        }
        
        /* Nested range checks */
        int64_t z = y * 2;
        if (z > 2000000 && z < 8000000) {
            printf("Nested range: z=%lld\n", (long long)z);
        }
    }
    
    /* Large step induction variable */
    for (int64_t i = 0; i < INT64_MAX - 100; i += (INT64_MAX / 100)) {
        /* Loop analysis may use double_int for wrap-around checks */
        if (i > INT64_MAX / 2) {
            printf("Large induction: i=%lld\n", (long long)i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
template <__int128_t N>
struct LargeCompare {
    static const bool greater = N > (__int128_t(1) << 65);
    static const bool less = N < -((__int128_t(1) << 65));
    static const bool equal = N == (__int128_t(1) << 66);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((__int128_t)1 << 70)>;
template struct LargeCompare<-((__int128_t)1 << 68)>;
template struct LargeCompare<((__int128_t)1 << 66)>;
#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_t_attr;

/* Operations on wide types that require magnitude comparisons */
int128_t_attr wide_division(int128_t_attr a, int128_t_attr b) {
    /* Division requires comparing magnitudes */
    return a / b;
}

int128_t_attr wide_modulus(int128_t_attr a, int128_t_attr b) {
    /* Modulus requires magnitude comparisons */
    return a % b;
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = ((__int128_t)1 << 70),
    BIG_ENUM_B = ((__int128_t)1 << 69),
    BIG_ENUM_C = ((__int128_t)1 << 68)
};

/* ========== 6. Additional Test Cases ========== */

/* Direct comparison of 128-bit values */
void test_direct_comparisons(void) {
    __int128_t a = ((__int128_t)1 << 70);
    __int128_t b = ((__int128_t)1 << 69);
    __int128_t c = -((__int128_t)1 << 70);
    
    /* All these comparisons should trigger double_int::cmp */
    if (a > b) printf("a > b: correct\n");
    if (b < a) printf("b < a: correct\n");
    if (a != b) printf("a != b: correct\n");
    if (c < a) printf("c < a: correct\n");
    if (a > 0) printf("a > 0: correct\n");
    if (c < 0) printf("c < 0: correct\n");
    
    /* Equality with self */
    if (a == a) printf("a == a: correct\n");
    if (c == c) printf("c == c: correct\n");
}

/* Mixed-size comparisons */
void test_mixed_comparisons(void) {
    __int128_t large = ((__int128_t)1 << 70);
    int64_t medium = INT64_MAX;
    int32_t small = INT32_MAX;
    
    if (large > medium) printf("large > medium: correct\n");
    if (medium < large) printf("medium < large: correct\n");
    if (large > small) printf("large > small: correct\n");
    
    /* Chain comparisons */
    if (small < medium && medium < large) {
        printf("Chain comparison: small < medium < large\n");
    }
}

/* ========== Main Function ========== */

int main(void) {
    int result = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Constant folding tests */
    printf("1. Constant Folding Tests:\n");
    printf("   compile_time_check returned: %d\n", compile_time_check());
    
    /* 2. Overflow builtin tests */
    printf("\n2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    
    /* 3. Range analysis tests */
    printf("\n3. Range Analysis Tests:\n");
    test_range_analysis(1500);
    
    /* 6. Direct comparison tests */
    printf("\n4. Direct 128-bit Comparison Tests:\n");
    test_direct_comparisons();
    
    /* Mixed comparisons */
    printf("\n5. Mixed-size Comparison Tests:\n");
    test_mixed_comparisons();
    
    /* Wide type operations */
    printf("\n6. Wide Type Operation Tests:\n");
    int128_t_attr w1 = ((int128_t_attr)1 << 70);
    int128_t_attr w2 = ((int128_t_attr)1 << 69);
    int128_t_attr quot = wide_division(w1, w2);
    int128_t_attr rem = wide_modulus(w1, w2);
    printf("   Wide division: %lld\n", (long long)quot);
    printf("   Wide modulus: %lld\n", (long long)rem);
    
    /* Enumeration tests */
    printf("\n7. Enumeration Tests:\n");
    if (BIG_ENUM_A > BIG_ENUM_B) {
        printf("   BIG_ENUM_A > BIG_ENUM_B: correct\n");
    }
    
    printf("\n=== All tests completed ===\n");
    
    return result;
}
