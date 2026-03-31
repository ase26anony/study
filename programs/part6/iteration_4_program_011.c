/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== 1. Constant Folding with Large Integers ========== */

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX * INT64_MAX);
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 100);

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_POS > 0, "Large positive constant");
static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");
static_assert(LARGE_UNSIGNED > UINT64_MAX, "Unsigned exceeds 64-bit");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static_assert((a) op (b), #a " " #op " " #b); \
        } \
    } while(0)

/* ========== 2. GCC Builtins with Overflow ========== */

/* Test overflow builtins that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that should trigger overflow */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    a = LLONG_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MIN, -1): overflow=%d\n", overflow);
    
    /* Test with constant propagation */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, 0))) {
        printf("Constant overflow check passed\n");
    }
    
    /* Add overflow with large values */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d\n", overflow);
}

/* ========== 3. Range Calculations ========== */

/* Complex range analysis that requires double_int comparisons */
void test_range_analysis(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* These multiplications create ranges that need double_int comparison */
        int64_t y = (int64_t)x * x;  /* 1000*1000 to 2000*2000 */
        int64_t z = y * x;           /* Up to 8e9, fits in 64-bit */
        
        /* Comparisons that trigger VRP */
        if (y > 1500000) {
            printf("y is large: %lld\n", (long long)y);
        }
        
        /* Chain comparisons for complex range analysis */
        if (x > 1500 && y > 2250000) {
            printf("x and y in upper range\n");
        }
    }
    
    /* Test with potential overflow in range calculation */
    if (x > 1000000 && x < 2000000) {
        int64_t big = (int64_t)x * 1000;  /* Up to 2e9 */
        if (big > 1500000000) {
            printf("Very big: %lld\n", (long long)big);
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = 0; i < 10000000000LL; i += 1000000000LL) {
        /* Loop analysis uses double_int for wrap-around checks */
        if (i > 5000000000LL) {
            printf("i exceeded 5B: %lld\n", (long long)i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ style in C) ========== */

/* Simulate template-like behavior with macros */
#define DEFINE_LARGE_COMPARE(name, val1, val2) \
    static const int name##_gt = ((val1) > (val2)) ? 1 : 0; \
    static const int name##_lt = ((val1) < (val2)) ? 1 : 0; \
    static const int name##_eq = ((val1) == (val2)) ? 1 : 0

/* Instantiate with large values */
DEFINE_LARGE_COMPARE(cmp1, ((int128_t)1 << 65), ((int128_t)1 << 64));
DEFINE_LARGE_COMPARE(cmp2, -((int128_t)1 << 66), -((int128_t)1 << 65));
DEFINE_LARGE_COMPARE(cmp3, ((uint128_t)1 << 100), ((uint128_t)1 << 99));

/* ========== 5. Tree Node Construction ========== */

/* Use 128-bit types with attributes */
typedef int128_t __attribute__((mode(TI))) ti_int;
typedef uint128_t __attribute__((mode(TI))) ti_uint;

/* Operations that create INTEGER_CST nodes */
static const ti_int TI_CONST = ((ti_int)1 << 100);
static const ti_uint UTI_CONST = ((ti_uint)1 << 100);

/* Enumeration with large values */
enum big_enum : int128_t {
    BIG_ENUM_A = ((int128_t)1 << 70),
    BIG_ENUM_B = ((int128_t)1 << 71),
    BIG_ENUM_C = ((int128_t)1 << 72)
};

/* Division/modulus operations that require magnitude comparison */
void test_large_division(void) {
    ti_int a = TI_CONST;
    ti_int b = ((ti_int)1 << 50);
    
    /* These operations require comparing magnitudes */
    ti_int div_result = a / b;
    ti_int mod_result = a % b;
    
    printf("Division result: %lld (high word)\n", 
           (long long)(div_result >> 64));
    printf("Modulus result: %lld (high word)\n", 
           (long long)(mod_result >> 64));
}

/* ========== 6. Mixed Signed/Unsigned Comparisons ========== */

/* Test the specific uncovered lines with mixed comparisons */
void test_mixed_comparisons(void) {
    /* Create values where high parts differ in unsigned comparison */
    int128_t val1 = ((int128_t)0x8000000000000000LL) << 64;  /* High bit set */
    int128_t val2 = ((int128_t)0x7FFFFFFFFFFFFFFFLL) << 64;  /* High bit clear */
    
    /* These should trigger the high part comparison paths */
    if (val1 > val2) {
        printf("val1 > val2 (signed comparison)\n");
    }
    
    /* Force unsigned comparison of high parts */
    uint128_t uval1 = (uint128_t)val1;
    uint128_t uval2 = (uint128_t)val2;
    
    if (uval1 < uval2) {
        printf("uval1 < uval2 (unsigned comparison)\n");
    }
    
    /* Test equal high parts, different low parts */
    int128_t val3 = ((int128_t)0x123456789ABCDEF0LL << 64) | 0x1111;
    int128_t val4 = ((int128_t)0x123456789ABCDEF0LL << 64) | 0x2222;
    
    if (val3 < val4) {
        printf("val3 < val4 (same high, different low)\n");
    }
}

/* ========== 7. Runtime Validation ========== */

/* Verify that compile-time and runtime comparisons match */
void validate_comparisons(void) {
    int all_pass = 1;
    
    /* Test 1: Basic large constant comparisons */
    if (!(VERY_LARGE_POS > 0)) {
        printf("FAIL: VERY_LARGE_POS > 0\n");
        all_pass = 0;
    }
    
    if (!(VERY_LARGE_NEG < 0)) {
        printf("FAIL: VERY_LARGE_NEG < 0\n");
        all_pass = 0;
    }
    
    /* Test 2: Product comparisons */
    if (!(HUGE_PRODUCT > INT64_MAX)) {
        printf("FAIL: HUGE_PRODUCT > INT64_MAX\n");
        all_pass = 0;
    }
    
    /* Test 3: Template-style comparisons */
    if (cmp1_gt != 1 || cmp1_lt != 0 || cmp1_eq != 0) {
        printf("FAIL: cmp1 comparisons\n");
        all_pass = 0;
    }
    
    if (cmp2_gt != 0 || cmp2_lt != 1 || cmp2_eq != 0) {
        printf("FAIL: cmp2 comparisons\n");
        all_pass = 0;
    }
    
    /* Test 4: Enum comparisons */
    if (!(BIG_ENUM_B > BIG_ENUM_A)) {
        printf("FAIL: BIG_ENUM_B > BIG_ENUM_A\n");
        all_pass = 0;
    }
    
    if (all_pass) {
        printf("\n=== ALL VALIDATIONS PASSED ===\n");
    } else {
        printf("\n=== SOME VALIDATIONS FAILED ===\n");
    }
}

/* ========== Main Function ========== */

int main(void) {
    printf("Testing double_int::cmp coverage...\n\n");
    
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    printf("2. Testing range analysis:\n");
    test_range_analysis(1500);
    printf("\n");
    
    printf("3. Testing induction variables:\n");
    test_induction_variables();
    printf("\n");
    
    printf("4. Testing large division:\n");
    test_large_division();
    printf("\n");
    
    printf("5. Testing mixed comparisons:\n");
    test_mixed_comparisons();
    printf("\n");
    
    printf("6. Validating all comparisons:\n");
    validate_comparisons();
    
    return 0;
}
