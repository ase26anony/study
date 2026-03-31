/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Use __int128 for 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
#define VERY_LARGE_POS ((int128_t)1 << 70)
#define VERY_LARGE_NEG (-((int128_t)1 << 70))
#define HUGE_CONSTANT (((int128_t)0x123456789ABCDEFULL) << 64 | 0xFEDCBA9876543210ULL)

/* Static assertions forcing compile-time comparisons */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant should be > 0");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant should be < 0");
_Static_assert(HUGE_CONSTANT > VERY_LARGE_POS, "Huge constant should be larger");
_Static_assert((VERY_LARGE_POS << 1) > VERY_LARGE_POS, "Shifted constant should be larger");

/* Compile-time function using __builtin_constant_p */
static inline int test_constant_folding(void) {
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        return 1;
    }
    return 0;
}

/* ========== 2. GCC Builtins That Manipulate double_int ========== */

/* Test overflow builtins with large values */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that may trigger double_int comparisons in overflow detection */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    a = 1LL << 62;
    b = 1LL << 62;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(2^62, 2^62): overflow=%d\n", overflow);
    
    /* Test with __int128 for larger range */
    int128_t big_a = ((int128_t)1 << 63);
    int128_t big_b = ((int128_t)1 << 63);
    int128_t big_res;
    
    /* Simulate overflow check by comparing against limits */
    if (big_a > INT64_MAX / big_b) {
        printf("Manual overflow check triggered for 128-bit values\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Complex range analysis that should trigger double_int comparisons */
static void test_range_analysis(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* These operations should trigger range calculations with double_int */
        long long y = (long long)x * x;
        long long z = y * y;  /* May overflow to large values */
        
        /* Comparisons that require analyzing ranges */
        if (y > 500000 && y < 4000000) {
            printf("Range analysis: y=%lld within expected range\n", y);
        }
        
        /* Shift operations creating large values */
        long long shifted = y << 10;
        if (shifted > (1LL << 40)) {
            printf("Large shifted value: %lld\n", shifted);
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;  /* Becomes positive large */
        if (y > 1000000) {
            printf("Negative range produced large positive: %lld\n", y);
        }
    }
}

/* Induction variable with large step */
static void test_induction_variables(void) {
    for (int128_t i = 0; i < ((int128_t)1 << 70); i += ((int128_t)1 << 60)) {
        /* Loop condition comparison uses double_int::cmp */
        if (i > ((int128_t)1 << 65)) {
            printf("Induction variable exceeded threshold\n");
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
#include <type_traits>

/* Template with large integer comparisons */
template <int128_t N>
struct LargeCompare {
    static const bool greater_than_threshold = N > ((int128_t)1 << 65);
    static const bool less_than_huge = N < HUGE_CONSTANT;
    static const bool is_positive = N > 0;
};

/* Instantiate templates with various large values */
template struct LargeCompare<((int128_t)1 << 66)>;
template struct LargeCompare<((int128_t)1 << 64)>;
template struct LargeCompare<-((int128_t)1 << 66)>;

#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Enumeration with large values */
enum big_enum : uint64_t {
    BIG_VALUE1 = UINT64_MAX,
    BIG_VALUE2 = UINT64_MAX - 1
};

/* Operations on large enum values */
static void test_enum_comparisons(void) {
    if (BIG_VALUE1 > BIG_VALUE2) {
        printf("Large enum comparison correct\n");
    }
    
    /* Arithmetic that may require wide integer representation */
    uint64_t big_product = BIG_VALUE1 * 2ULL;
    if (big_product < BIG_VALUE1) {
        printf("Overflow in enum arithmetic detected\n");
    }
}

/* ========== Main Test Harness ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* 1. Test constant folding */
    printf("1. Constant folding tests:\n");
    printf("   VERY_LARGE_POS = %llx... (truncated)\n", 
           (unsigned long long)(VERY_LARGE_POS >> 64));
    printf("   VERY_LARGE_NEG = %llx... (truncated)\n", 
           (unsigned long long)(VERY_LARGE_NEG >> 64));
    printf("   Constant folding function returned: %d\n\n", 
           test_constant_folding());
    
    /* 2. Test overflow builtins */
    printf("2. Overflow builtin tests:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* 3. Test range analysis */
    printf("3. Range analysis tests:\n");
    test_range_analysis(1500);  /* Within positive range */
    test_range_analysis(-1500); /* Within negative range */
    printf("\n");
    
    /* 4. Test enum comparisons */
    printf("4. Large enum comparisons:\n");
    test_enum_comparisons();
    printf("\n");
    
    /* 5. Test induction variables (simulated) */
    printf("5. Induction variable test:\n");
    printf("   (Note: Full iteration would be very long)\n");
    /* Just test the comparison logic */
    int128_t test_val = ((int128_t)1 << 66);
    if (test_val > ((int128_t)1 << 65)) {
        printf("   Large value comparison works\n");
    }
    
    /* Additional test: Compare various large values */
    printf("\n6. Direct large value comparisons:\n");
    int128_t vals[] = {
        ((int128_t)1 << 63),
        ((int128_t)1 << 64),
        ((int128_t)1 << 65),
        -((int128_t)1 << 63),
        -((int128_t)1 << 64)
    };
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (vals[i] < vals[j]) {
                printf("   %d < %d\n", i, j);
            } else if (vals[i] > vals[j]) {
                printf("   %d > %d\n", i, j);
            }
        }
    }
    
    /* Test equality with large values */
    int128_t copy = ((int128_t)1 << 66);
    if (test_val == copy) {
        printf("   Large value equality works\n");
    }
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
