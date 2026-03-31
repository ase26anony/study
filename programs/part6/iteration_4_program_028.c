/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX) * ((int128_t)INT64_MAX);
static const uint128_t LARGE_MASK = ~((uint128_t)0);

/* Static assertions forcing compile-time comparisons */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(HUGE_PRODUCT > VERY_LARGE_POS, "Product comparison");
_Static_assert(LARGE_MASK > 0xFFFFFFFFFFFFFFFFULL, "Unsigned large comparison");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static volatile int dummy = ((a) op (b)) ? 1 : 0; \
            (void)dummy; \
        } \
    } while(0)

/* ========== 2. GCC Builtins That Use double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    /* Case 2: Addition with potential overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d\n", overflow);
    
    /* Case 3: Subtraction with underflow */
    a = LLONG_MIN;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d\n", overflow);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis triggering double_int comparisons */
void test_range_analysis(int x) {
    /* Known bounds that will be analyzed by VRP */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Further range refinement */
        if (y > 1500000 && y < 3000000) {
            int64_t z = y * 2;
            printf("Range analysis: x=%d, y=%ld, z=%ld\n", x, (long)y, (long)z);
        }
    }
    
    /* Large value range analysis */
    if (x > INT32_MAX) {
        int128_t big = (int128_t)x * (int128_t)INT64_MAX;
        if (big > ((int128_t)1 << 80)) {
            printf("Very large range detected\n");
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int128_t i = 0; i < ((int128_t)1 << 65); i += ((int128_t)1 << 40)) {
        /* Loop analysis may compare induction variable bounds */
        if (i > ((int128_t)1 << 60)) {
            printf("Induction variable reached large value\n");
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ style in C) ========== */

/* Simulate template-like behavior with macros */
#define LARGE_COMPARE(N, threshold, result_var) \
    do { \
        if (__builtin_constant_p(N)) { \
            result_var = (N > threshold) ? 1 : 0; \
        } else { \
            result_var = ((int128_t)(N) > (int128_t)(threshold)) ? 1 : 0; \
        } \
    } while(0)

/* ========== 5. Force Tree Node Construction ========== */

/* Use 128-bit types with attributes */
typedef int128_t __attribute__((mode(TI))) ti_int;
typedef uint128_t __attribute__((mode(TI))) tu_int;

/* Operations that create wide INTEGER_CST nodes */
void test_wide_operations(ti_int a, ti_int b) {
    ti_int sum = a + b;
    ti_int diff = a - b;
    ti_int prod = a * b;
    
    /* Comparisons that may use double_int::cmp */
    if (sum > diff) {
        printf("sum > diff\n");
    }
    if (prod < ((ti_int)1 << 66)) {
        printf("product within range\n");
    }
    
    /* Division/modulus with large values */
    if (b != 0) {
        ti_int quot = a / b;
        ti_int rem = a % b;
        if (quot > 0 && rem < b) {
            printf("division properties hold\n");
        }
    }
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = ((__int128)1 << 70),
    BIG_ENUM_B = ((__int128)1 << 71),
    BIG_ENUM_C = BIG_ENUM_A + BIG_ENUM_B
};

/* ========== Main Test Harness ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Trigger compile-time constant folding */
    printf("1. Constant Folding Tests:\n");
    COMPILE_TIME_CMP(VERY_LARGE_POS, VERY_LARGE_NEG, >);
    COMPILE_TIME_CMP(HUGE_PRODUCT, VERY_LARGE_POS, >);
    COMPILE_TIME_CMP(LARGE_MASK, 0, >);
    printf("   Static assertions passed\n\n");
    
    /* 2. Test overflow builtins */
    printf("2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 3. Range analysis tests */
    printf("3. Range Analysis Tests:\n");
    test_range_analysis(1500);
    test_range_analysis(INT32_MAX + 1000L);
    printf("\n");
    
    /* 4. Template-like comparisons */
    printf("4. Large Value Comparisons:\n");
    int result1, result2;
    LARGE_COMPARE(BIG_ENUM_B, BIG_ENUM_A, result1);
    LARGE_COMPARE(((int128_t)1 << 65), ((int128_t)1 << 64), result2);
    printf("   BIG_ENUM_B > BIG_ENUM_A: %d\n", result1);
    printf("   2^65 > 2^64: %d\n\n", result2);
    
    /* 5. Wide operations */
    printf("5. Wide Integer Operations:\n");
    test_wide_operations(
        ((int128_t)1 << 70) + 123,
        ((int128_t)1 << 60) + 456
    );
    
    /* 6. Induction variable test */
    printf("\n6. Induction Variable Test:\n");
    test_induction_variables();
    
    /* 7. Runtime validation of compile-time results */
    printf("\n7. Runtime Validation:\n");
    
    /* Verify that our large constant comparisons are correct */
    assert(VERY_LARGE_POS > 0);
    assert(VERY_LARGE_NEG < 0);
    assert(HUGE_PRODUCT > VERY_LARGE_POS);
    assert(BIG_ENUM_B > BIG_ENUM_A);
    
    /* Test edge cases for comparison */
    int128_t max128 = ~((int128_t)0) >> 1;
    int128_t min128 = -max128 - 1;
    assert(max128 > min128);
    assert(min128 < 0);
    
    /* Test unsigned comparisons */
    uint128_t u1 = ~((uint128_t)0);
    uint128_t u2 = u1 >> 1;
    assert(u1 > u2);
    
    printf("\nAll tests completed successfully!\n");
    printf("If compiled with coverage, double_int::cmp should be exercised.\n");
    
    return 0;
}
