/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ===== 1. Trigger Constant Folding with Large Integers ===== */

/* Use __int128 for wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t HUGE_VAL = ((int128_t)1 << 100);
const uint128_t HUGE_UNSIGNED = ((uint128_t)1 << 100);

/* Static assertions with large integer comparisons */
static_assert(VERY_LARGE_POS > 0, "Large positive constant");
static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
static_assert(HUGE_VAL > VERY_LARGE_POS, "Hierarchical comparison");
static_assert(HUGE_UNSIGNED > (uint128_t)VERY_LARGE_POS, "Unsigned comparison");

/* Compile-time function using __builtin_constant_p */
static int compile_time_check(void) {
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        return 1;
    }
    if (__builtin_constant_p(HUGE_VAL < 0)) {
        return 0;
    }
    return -1;
}

/* Arithmetic producing wide integers */
static int128_t wide_multiply(int64_t a, int64_t b) {
    return (int128_t)a * (int128_t)b;
}

static uint128_t wide_shift(uint64_t a, int shift) {
    return (uint128_t)a << shift;
}

/* ===== 2. GCC Builtins That Return or Manipulate double_int ===== */

/* Overflow builtins with large types */
static void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Test cases that may trigger overflow comparisons */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2) = %d, res = %lld\n", overflow, (long long)res);
    
    a = LLONG_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MIN, -1) = %d, res = %lld\n", overflow, (long long)res);
    
    /* Test with __int128 */
    int128_t a128, b128, res128;
    a128 = ((int128_t)LLONG_MAX) * 2;
    b128 = ((int128_t)LLONG_MAX) * 3;
    /* Simulate overflow check using comparisons */
    if (a128 > b128) {
        printf("a128 > b128\n");
    } else if (a128 < b128) {
        printf("a128 < b128\n");
    }
}

/* Constant overflow checks */
static void test_constant_overflow(void) {
    /* These force compile-time overflow analysis */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
    
    /* Large constant multiplication */
    const int128_t prod = (int128_t)LLONG_MAX * (int128_t)LLONG_MAX;
    if (prod > 0) {
        printf("Large product is positive\n");
    }
}

/* ===== 3. Range Calculations That Compare Bounds ===== */

/* Function with complex range analysis */
static void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* Multiplication that requires range analysis with double_int */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Further bounds checking */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis: y = %lld within bounds\n", (long long)y);
        }
        
        /* Nested ranges */
        if (x > 1500) {
            int64_t z = y * 2;
            if (z > 3000000) {
                printf("Nested range triggered\n");
            }
        }
    }
    
    /* Test with potential overflow in range */
    if (x > INT_MAX / 2) {
        int64_t big = (int64_t)x * 3;
        if (big > INT_MAX) {
            printf("Potential overflow in range\n");
        }
    }
}

/* Loop with induction variable analysis */
static void test_induction_variables(void) {
    for (int64_t i = LLONG_MAX - 100; i < LLONG_MAX; i += 10) {
        /* Loop analysis may use double_int for wrap-around checks */
        if (i > LLONG_MAX - 50) {
            printf("Near overflow: %lld\n", (long long)i);
        }
    }
    
    /* Large step value */
    for (int128_t j = 0; j < ((int128_t)1 << 70); j += ((int128_t)1 << 60)) {
        /* Empty loop - compiler analyzes bounds */
        (void)j;
    }
}

/* ===== 4. Template Metaprogramming (C++ version available) ===== */

/* In C, we simulate with macros and static functions */
#define LARGE_COMPARE(N) ((N) > ((int128_t)1 << 65))

static const int is_large = LARGE_COMPARE(VERY_LARGE_POS);
static const int is_small = LARGE_COMPARE(100);

/* ===== 5. Force Tree Node Construction for Wide Constants ===== */

/* Use mode attribute for 128-bit types */
typedef int int128_attr __attribute__((mode(TI)));
typedef unsigned int uint128_attr __attribute__((mode(TI)));

/* Operations on attributed types */
static int128_attr test_attributed_types(int128_attr a, int128_attr b) {
    /* Various operations that require magnitude comparison */
    if (a > b) {
        return a / b;
    } else if (a < b) {
        return b / a;
    }
    return a % (b + 1);
}

/* Enumeration with large values */
enum big_enum {
    BIG_VAL = (int)((int128_t)1 << 63),
    BIGGER_VAL = (int)((int128_t)1 << 70)  /* May be truncated but triggers wide constant */
};

/* ===== Main Test Harness ===== */

int main(void) {
    int all_passed = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Constant folding tests */
    printf("1. Constant Folding Tests:\n");
    if (VERY_LARGE_POS <= 0) {
        printf("  FAIL: VERY_LARGE_POS should be positive\n");
        all_passed = 0;
    } else {
        printf("  PASS: VERY_LARGE_POS > 0\n");
    }
    
    if (VERY_LARGE_NEG >= 0) {
        printf("  FAIL: VERY_LARGE_NEG should be negative\n");
        all_passed = 0;
    } else {
        printf("  PASS: VERY_LARGE_NEG < 0\n");
    }
    
    int ct_check = compile_time_check();
    printf("  Compile-time check returned: %d\n", ct_check);
    
    /* Test wide arithmetic */
    int128_t prod = wide_multiply(LLONG_MAX, 2);
    if (prod < 0) {
        printf("  FAIL: LLONG_MAX * 2 should be positive\n");
        all_passed = 0;
    } else {
        printf("  PASS: LLONG_MAX * 2 is positive\n");
    }
    
    uint128_t shifted = wide_shift(1, 70);
    if (shifted <= ((uint128_t)1 << 69)) {
        printf("  FAIL: 1 << 70 should be greater than 1 << 69\n");
        all_passed = 0;
    } else {
        printf("  PASS: 1 << 70 > 1 << 69\n");
    }
    
    printf("\n2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    test_constant_overflow();
    
    printf("\n3. Range Analysis Tests:\n");
    test_range_analysis(1500);
    test_range_analysis(3000);
    
    printf("\n4. Induction Variable Tests:\n");
    test_induction_variables();
    
    printf("\n5. Attributed Type Tests:\n");
    int128_attr a = ((int128_attr)1 << 70);
    int128_attr b = ((int128_attr)1 << 69);
    int128_attr result = test_attributed_types(a, b);
    printf("  Result of attributed type operation: (value depends on comparison)\n");
    
    /* Final validation */
    printf("\n=== Summary ===\n");
    if (all_passed) {
        printf("All basic checks PASSED\n");
    } else {
        printf("Some checks FAILED\n");
    }
    
    /* Additional runtime comparisons to ensure code isn't eliminated */
    volatile int128_t v1 = VERY_LARGE_POS;
    volatile int128_t v2 = VERY_LARGE_NEG;
    volatile int cmp_result = (v1 > v2) ? 1 : ((v1 < v2) ? -1 : 0);
    printf("Runtime comparison result: %d\n", cmp_result);
    
    return all_passed ? 0 : 1;
}
