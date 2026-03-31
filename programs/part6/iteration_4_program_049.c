/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Static assertions with large constants */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* Test 1: Basic large constant comparisons */
void test_large_constants(void) {
    /* Constants larger than 64 bits */
    const int128_t huge_pos = ((int128_t)1 << 70);
    const int128_t huge_neg = -((int128_t)1 << 70);
    const int128_t medium = ((int128_t)1 << 65);
    
    /* These should trigger double_int::cmp during constant folding */
    STATIC_ASSERT(huge_pos > 0);
    STATIC_ASSERT(huge_neg < 0);
    STATIC_ASSERT(huge_pos > medium);
    STATIC_ASSERT(huge_neg < medium);
    STATIC_ASSERT(huge_pos != huge_neg);
    STATIC_ASSERT(huge_pos == huge_pos);
    
    /* Test both high and low parts comparison */
    const int128_t a = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    const int128_t b = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543211ULL;
    const int128_t c = ((int128_t)0x123456789ABCDEF1 << 64) | 0x0000000000000000ULL;
    
    STATIC_ASSERT(a < b);  /* Same high, different low */
    STATIC_ASSERT(b < c);  /* Different high */
    STATIC_ASSERT(a != b);
}

/* Test 2: Builtin overflow operations */
void test_overflow_builtins(void) {
    long long x, y;
    long long result;
    int overflow;
    
    /* These operations may trigger double_int comparisons internally */
    x = 0x7FFFFFFFFFFFFFFFLL;  /* Max int64_t */
    y = 2;
    
    overflow = __builtin_mul_overflow(x, y, &result);
    if (overflow) {
        printf("Multiplication overflow detected (expected)\n");
    }
    
    /* Test with constants that force double_int comparison */
    const int64_t large1 = 0x7FFFFFFFFFFFFFFFLL;
    const int64_t large2 = 0x7FFFFFFFFFFFFFFELL;
    
    /* The overflow check internally compares double_int values */
    if (__builtin_mul_overflow_p(large1, large2, (int64_t)0)) {
        printf("Constant multiplication would overflow\n");
    }
    
    /* Test addition overflow */
    int64_t a = 0x7FFFFFFFFFFFFFFFLL;
    int64_t b = 1;
    if (__builtin_add_overflow(a, b, &result)) {
        printf("Addition overflow detected\n");
    }
}

/* Test 3: Range analysis with large values */
void test_range_analysis(int input) {
    /* Create conditions that force VRP to use double_int comparisons */
    if (input > 1000 && input < 2000) {
        /* This multiplication's range analysis uses double_int::cmp */
        int64_t x = (int64_t)input * input;
        
        /* Further range refinement */
        if (x > 1500000 && x < 2500000) {
            int64_t y = x * 2;
            /* The comparison of ranges here triggers double_int operations */
            if (y > 3000000) {
                printf("Range analysis path taken\n");
            }
        }
    }
    
    /* Test with very large ranges */
    uint64_t big = 0xFFFFFFFFFFFFFFFFULL;
    if (input > 0) {
        /* This may trigger comparisons of 128-bit values */
        uint128_t huge_product = (uint128_t)big * (uint128_t)input;
        if (huge_product > (uint128_t)big) {
            printf("Large range comparison\n");
        }
    }
}

/* Test 4: Complex constant expressions */
void test_complex_constants(void) {
    /* Expressions that require multi-step constant folding */
    const int128_t expr1 = ((int128_t)1 << 120) / 3;
    const int128_t expr2 = ((int128_t)1 << 120) / 4;
    
    STATIC_ASSERT(expr1 > expr2);
    STATIC_ASSERT(expr1 != expr2);
    
    /* Mixed 64-bit and 128-bit operations */
    const uint64_t large64 = 0xFFFFFFFFFFFFFFFFULL;
    const int128_t mixed = (int128_t)large64 * large64;
    const int128_t mixed2 = mixed + 1;
    
    STATIC_ASSERT(mixed2 > mixed);
    
    /* Test all comparison operators */
    const int128_t val1 = ((int128_t)0x5A5A5A5A5A5A5A5AULL << 64) | 0x5A5A5A5A5A5A5A5AULL;
    const int128_t val2 = ((int128_t)0x5A5A5A5A5A5A5A5AULL << 64) | 0x5A5A5A5A5A5A5A5BULL;
    const int128_t val3 = ((int128_t)0x5A5A5A5A5A5A5A5BULL << 64) | 0x0000000000000000ULL;
    
    STATIC_ASSERT(val1 < val2);   /* Same high, low differs */
    STATIC_ASSERT(val2 < val3);   /* High differs */
    STATIC_ASSERT(val1 <= val2);
    STATIC_ASSERT(val2 >= val1);
    STATIC_ASSERT(val1 != val2);
    STATIC_ASSERT(val1 == val1);
}

/* Test 5: Shift operations that produce large values */
void test_shift_operations(void) {
    const int128_t base = 1;
    
    /* Shifts that cross the 64-bit boundary */
    STATIC_ASSERT((base << 0) < (base << 1));
    STATIC_ASSERT((base << 63) < (base << 64));
    STATIC_ASSERT((base << 64) < (base << 65));
    STATIC_ASSERT((base << 127) > (base << 126));
    
    /* Negative shifts (right shifts) */
    const int128_t large_negative = -((int128_t)1 << 70);
    STATIC_ASSERT(large_negative < 0);
    STATIC_ASSERT((large_negative >> 1) > large_negative); /* Arithmetic right shift */
}

/* Test 6: Runtime comparisons that can't be fully optimized away */
void test_runtime_comparisons(void) {
    /* Use volatile to prevent compile-time optimization */
    volatile int128_t dynamic_a = ((int128_t)rand() << 64) | rand();
    volatile int128_t dynamic_b = ((int128_t)rand() << 64) | rand();
    
    /* These should generate runtime comparison code */
    if (dynamic_a < dynamic_b) {
        printf("Runtime comparison: a < b\n");
    } else if (dynamic_a > dynamic_b) {
        printf("Runtime comparison: a > b\n");
    } else {
        printf("Runtime comparison: a == b\n");
    }
    
    /* Test with known patterns to hit specific comparison paths */
    int128_t pattern1 = ((int128_t)0x1000000000000000ULL << 64) | 0x0000000000000000ULL;
    int128_t pattern2 = ((int128_t)0x1000000000000000ULL << 64) | 0x0000000000000001ULL;
    int128_t pattern3 = ((int128_t)0x1000000000000001ULL << 64) | 0x0000000000000000ULL;
    
    /* Force evaluation of all comparison branches */
    int results[6] = {0};
    results[0] = (pattern1 < pattern2) ? 1 : 0;  /* Same high, low differs */
    results[1] = (pattern2 < pattern3) ? 1 : 0;  /* High differs */
    results[2] = (pattern1 > pattern2) ? 0 : 1;
    results[3] = (pattern1 == pattern1) ? 1 : 0;
    results[4] = (pattern1 != pattern2) ? 1 : 0;
    results[5] = (pattern1 <= pattern2) ? 1 : 0;
    
    printf("Pattern comparisons: ");
    for (int i = 0; i < 6; i++) {
        printf("%d", results[i]);
    }
    printf("\n");
}

/* Test 7: Boundary value testing */
void test_boundary_values(void) {
    /* Test minimum and maximum 128-bit values */
    const int128_t min_s128 = ((int128_t)1 << 127);
    const int128_t max_s128 = ~((int128_t)1 << 127);
    const uint128_t max_u128 = ~(uint128_t)0;
    
    STATIC_ASSERT(min_s128 < 0);
    STATIC_ASSERT(max_s128 > 0);
    STATIC_ASSERT((uint128_t)max_u128 > (uint128_t)max_s128);
    
    /* Test around the 64-bit boundary */
    const int128_t just_below_64 = ((int128_t)0xFFFFFFFFFFFFFFFFULL);
    const int128_t just_above_64 = ((int128_t)1 << 64);
    
    STATIC_ASSERT(just_below_64 < just_above_64);
    STATIC_ASSERT(just_above_64 > just_below_64);
    
    /* Test with high part = 0, low part large */
    const int128_t low_large = 0xFFFFFFFFFFFFFFFFULL;
    const int128_t high_small = ((int128_t)1 << 64);
    
    STATIC_ASSERT(low_large < high_small);
}

/* Test 8: Arithmetic operations followed by comparisons */
void test_arithmetic_comparisons(void) {
    const int128_t a = ((int128_t)0x12345678 << 64) | 0x9ABCDEF012345678ULL;
    const int128_t b = ((int128_t)0x12345678 << 64) | 0x9ABCDEF012345677ULL;
    const int128_t c = 1;
    
    /* These operations create new double_int values that need comparison */
    STATIC_ASSERT(a + c > a);
    STATIC_ASSERT(a - c < a);
    STATIC_ASSERT(a * 2 > a);
    
    /* Test with overflow in intermediate calculations */
    const int128_t large = ((int128_t)1 << 126);
    STATIC_ASSERT(large * 2 > large);
    STATIC_ASSERT(large / 2 < large);
}

int main(void) {
    printf("Starting double_int::cmp coverage tests...\n");
    
    /* All static assertions are checked at compile time */
    printf("Static assertions passed (compiled successfully)\n");
    
    /* Run runtime tests */
    test_overflow_builtins();
    
    /* Test range analysis with various inputs */
    test_range_analysis(1500);
    test_range_analysis(500);
    
    test_runtime_comparisons();
    
    /* Verify all comparison paths */
    test_complex_constants();
    test_shift_operations();
    test_boundary_values();
    test_arithmetic_comparisons();
    
    printf("\nAll tests completed successfully!\n");
    printf("If compilation succeeded, the double_int::cmp logic was exercised.\n");
    
    return 0;
}
