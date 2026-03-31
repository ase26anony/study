/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Test 1: Large integer constant comparisons */
static void test_large_constants(void) {
    /* Use __int128 for 128-bit integers */
    const __int128_t huge_pos = ((__int128_t)1 << 70);
    const __int128_t huge_neg = -((__int128_t)1 << 70);
    const __int128_t medium = ((__int128_t)1 << 65);
    
    /* Static assertions force compile-time evaluation */
    _Static_assert(huge_pos > 0, "Large positive constant");
    _Static_assert(huge_neg < 0, "Large negative constant");
    _Static_assert(huge_pos > medium, "Comparison of large constants");
    _Static_assert(huge_neg < medium, "Signed comparison with negative");
    
    /* Force constant folding with __builtin_constant_p */
    if (__builtin_constant_p(huge_pos > medium)) {
        printf("Constant folded: huge_pos > medium\n");
    }
    
    /* Test equality with different high parts */
    const __int128_t a = ((__int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128_t b = ((__int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
    const __int128_t c = ((__int128_t)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;
    
    _Static_assert(a != b, "Different low parts");
    _Static_assert(a != c, "Different high parts");
    _Static_assert(c > a, "High part comparison");
    _Static_assert(b > a, "Low part comparison when high parts equal");
}

/* Test 2: Builtin overflow operations */
static void test_overflow_builtins(void) {
    long long x, y;
    long long result;
    int overflow;
    
    /* Test cases that trigger overflow comparisons */
    x = LLONG_MAX;
    y = 2;
    overflow = __builtin_mul_overflow(x, y, &result);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    x = 1LL << 62;
    y = 1LL << 62;
    overflow = __builtin_mul_overflow(x, y, &result);
    printf("mul_overflow(2^62, 2^62): overflow=%d\n", overflow);
    
    /* Test with __builtin_constant_p */
    const long long const_a = LLONG_MAX;
    const long long const_b = 1;
    if (__builtin_constant_p(__builtin_mul_overflow_p(const_a, const_b, (long long)0))) {
        printf("Constant overflow check performed\n");
    }
    
    /* Addition overflow */
    x = LLONG_MAX;
    y = 1;
    overflow = __builtin_add_overflow(x, y, &result);
    printf("add_overflow(LLONG_MAX, 1): overflow=%d\n", overflow);
    
    /* Subtraction overflow */
    x = LLONG_MIN;
    y = 1;
    overflow = __builtin_sub_overflow(x, y, &result);
    printf("sub_overflow(LLONG_MIN, 1): overflow=%d\n", overflow);
}

/* Test 3: Range analysis with complex conditions */
static void test_range_analysis(int input) {
    /* Create known value ranges */
    int x = input;
    
    if (x > 1000 && x < 2000) {
        /* Multiplication that requires range analysis */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000LL && y < 4000000LL) {
            printf("Range analysis: y = %lld within expected range\n", y);
        }
        
        /* Chain comparisons */
        int z = x * 2;
        if (z > 2000 && z < 4000) {
            /* Nested range checks */
            long long w = (long long)z * z;
            if (w > 4000000LL && w < 16000000LL) {
                printf("Nested range analysis passed\n");
            }
        }
    }
    
    /* Test with large values that might wrap */
    unsigned long long big = 1ULL << 63;
    if (big > (1ULL << 62)) {
        unsigned long long bigger = big * 2;
        if (bigger > big) {
            printf("Unsigned large value comparison\n");
        }
    }
}

/* Test 4: Loop induction variables with large steps */
static void test_loop_induction(void) {
    for (int64_t i = 0; i < 100; i += (INT64_MAX / 100)) {
        /* The compiler analyzes loop bounds using wide integers */
        if (i > INT64_MAX / 2) {
            printf("Loop induction large step: %ld\n", (long)i);
            break;
        }
    }
    
    /* Test wrap-around analysis */
    unsigned long long counter = ULLONG_MAX - 10;
    for (int j = 0; j < 20; j++) {
        counter++;
        /* Compiler analyzes if counter wraps around */
        if (counter == 0) {
            printf("Wrap-around detected at iteration %d\n", j);
        }
    }
}

/* Test 5: Bitwise operations with wide integers */
static void test_bitwise_operations(void) {
    const __int128_t mask_high = ((__int128_t)0xFFFF000000000000ULL << 64);
    const __int128_t mask_low = 0x0000FFFFFFFFFFFFULL;
    const __int128_t value = ((__int128_t)0x123456789ABCDEF0ULL << 64) | 0x0FEDCBA987654321ULL;
    
    /* Comparisons after bitwise operations */
    _Static_assert((value & mask_high) > 0, "High bits set");
    _Static_assert((value & mask_low) > 0, "Low bits set");
    
    /* Shift operations that create large values */
    const __int128_t shifted = value << 16;
    _Static_assert(shifted > value, "Left shift increases value");
    
    const __int128_t right_shifted = value >> 16;
    _Static_assert(right_shifted < value, "Right shift decreases value");
}

/* Test 6: Mixed signed/unsigned comparisons */
static void test_mixed_comparisons(void) {
    const __int128_t signed_big = ((__int128_t)1 << 70);
    const unsigned __int128_t unsigned_big = ((unsigned __int128_t)1 << 70);
    
    /* These force different comparison paths */
    _Static_assert(signed_big > 0, "Large signed positive");
    _Static_assert(unsigned_big > 0, "Large unsigned");
    
    /* Compare signed and unsigned directly */
    if ((unsigned __int128_t)signed_big == unsigned_big) {
        printf("Signed/unsigned equivalence at bit level\n");
    }
}

/* Test 7: Complex arithmetic expressions */
static void test_complex_expressions(void) {
    /* Create expressions that require multi-step constant folding */
    const __int128_t a = ((__int128_t)1 << 66);
    const __int128_t b = ((__int128_t)1 << 64);
    const __int128_t c = ((__int128_t)1 << 62);
    
    /* Complex expression with multiple comparisons */
    _Static_assert(a + b > c, "Addition comparison");
    _Static_assert(a - b > c, "Subtraction comparison");
    _Static_assert(a * 2 > a + b, "Multiplication comparison");
    
    /* Division with large numbers */
    const __int128_t dividend = ((__int128_t)1 << 70);
    const __int128_t divisor = ((__int128_t)1 << 68);
    _Static_assert(dividend / divisor == 4, "Division of large constants");
}

/* Test 8: Runtime validation of compile-time comparisons */
static void test_runtime_validation(void) {
    /* Use variables to prevent complete compile-time elimination */
    volatile __int128_t v1 = ((__int128_t)1 << 70);
    volatile __int128_t v2 = ((__int128_t)1 << 69);
    
    int result1 = (v1 > v2) ? 1 : 0;
    int result2 = (v1 < v2) ? 1 : 0;
    int result3 = (v1 == v2) ? 1 : 0;
    
    printf("Runtime comparisons: %d %d %d\n", result1, result2, result3);
    assert(result1 == 1 && result2 == 0 && result3 == 0);
    
    /* Test with negative values */
    volatile __int128_t neg1 = -((__int128_t)1 << 70);
    volatile __int128_t neg2 = -((__int128_t)1 << 69);
    
    int result4 = (neg1 < neg2) ? 1 : 0;
    int result5 = (neg1 > neg2) ? 1 : 0;
    
    printf("Negative comparisons: %d %d\n", result4, result5);
    assert(result4 == 1 && result5 == 0);
}

/* Main test driver */
int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    printf("Test 1: Large integer constant comparisons\n");
    test_large_constants();
    
    printf("\nTest 2: Builtin overflow operations\n");
    test_overflow_builtins();
    
    printf("\nTest 3: Range analysis with complex conditions\n");
    test_range_analysis(1500);  /* Mid-range value */
    test_range_analysis(500);   /* Below range */
    
    printf("\nTest 4: Loop induction variables\n");
    test_loop_induction();
    
    printf("\nTest 5: Bitwise operations\n");
    test_bitwise_operations();
    
    printf("\nTest 6: Mixed signed/unsigned comparisons\n");
    test_mixed_comparisons();
    
    printf("\nTest 7: Complex arithmetic expressions\n");
    test_complex_expressions();
    
    printf("\nTest 8: Runtime validation\n");
    test_runtime_validation();
    
    printf("\n=== All tests completed successfully ===\n");
    return 0;
}
