/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_UINT64     0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)HIGH_BIT_64 << 64),
               "Comparison with different high words");

/* Test 1: Comparisons with different high words (signed) */
__attribute__((noinline))
int test_high_word_comparisons(void) {
    volatile __int128 a, b;
    int checksum = 0;
    
    /* Case 1: Positive values, high words differ */
    a = ((__int128)HIGH_BIT_64 << 32) | 0x123456789ABCDEF0ULL;
    b = ((__int128)HIGH_BIT_64 << 33) | 0x123456789ABCDEF0ULL;
    checksum += (a < b) ? 1 : 0;
    checksum += (a > b) ? 2 : 0;
    
    /* Case 2: Negative values, high words differ */
    a = -(((__int128)HIGH_BIT_64 << 32) | 0x123456789ABCDEF0ULL);
    b = -(((__int128)HIGH_BIT_64 << 33) | 0x123456789ABCDEF0ULL);
    checksum += (a < b) ? 4 : 0;
    checksum += (a > b) ? 8 : 0;
    
    /* Case 3: Mixed signs */
    a = ((__int128)HIGH_BIT_64 << 63);  /* Very large positive */
    b = -a;                             /* Very large negative */
    checksum += (a < b) ? 16 : 0;
    checksum += (a > b) ? 32 : 0;
    
    return checksum;
}

/* Test 2: Comparisons with equal high words, different low words */
__attribute__((noinline))
int test_low_word_comparisons(void) {
    volatile __int128 base = ((__int128)0x123456789ABCDEF0ULL << 64);
    volatile __int128 a, b;
    int checksum = 0;
    
    /* Same high word, low word differs */
    a = base | 0x1111111111111111ULL;
    b = base | 0x2222222222222222ULL;
    
    checksum += (a < b) ? 1 : 0;
    checksum += (a > b) ? 2 : 0;
    checksum += (a == b) ? 4 : 0;
    
    /* Edge case: low word overflow comparison */
    a = base | MAX_UINT64;
    b = base + 1;  /* This should increment high word */
    checksum += (a < b) ? 8 : 0;
    checksum += (a > b) ? 16 : 0;
    
    return checksum;
}

/* Test 3: Range analysis with loops */
__attribute__((noinline))
int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -((__int128)1 << 70); 
         i < ((__int128)1 << 70); 
         i += ((__int128)1 << 60)) {
        /* Force range analysis on comparisons */
        if (i < 0) checksum += 1;
        if (i > ((__int128)1 << 68)) checksum += 2;
        if (i == 0) checksum += 4;
    }
    
    /* Array operations to give optimizer work */
    __int128 arr[8] = {
        ((__int128)1 << 120),
        ((__int128)1 << 64),
        0,
        -((__int128)1 << 64),
        -((__int128)1 << 120),
        MAX_UINT64,
        HIGH_BIT_64,
        ((__int128)MAX_UINT64 << 64) | MAX_UINT64
    };
    
    for (int i = 0; i < 7; i++) {
        checksum += (arr[i] < arr[i+1]) ? (1 << i) : 0;
        checksum += (arr[i] > arr[i+1]) ? (1 << (i+4)) : 0;
    }
    
    return checksum;
}

/* Test 4: Overflow operations requiring wide comparisons */
__attribute__((noinline))
int test_overflow_checks(void) {
    int checksum = 0;
    __int128 a, b, result;
    int overflow;
    
    /* Test near boundaries */
    a = ((__int128)MAX_UINT64 << 63) | MAX_UINT64;
    b = 1;
    
    /* These should trigger overflow checks with wide comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 1 : 0;
    
    overflow = __builtin_mul_overflow(a, 2, &result);
    checksum += overflow ? 2 : 0;
    
    /* Test with unsigned __int128 */
    unsigned __int128 ua = (unsigned __int128)-1;  /* UINT128_MAX */
    unsigned __int128 ub = 1;
    unsigned __int128 uresult;
    
    overflow = __builtin_add_overflow(ua, ub, &uresult);
    checksum += overflow ? 4 : 0;
    
    return checksum;
}

/* Test 5: Mixed precision operations */
__attribute__((noinline))
int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with narrower types */
    __int128 wide_val = ((__int128)1 << 66);
    long long narrow_val = LLONG_MAX;
    
    checksum += (wide_val > narrow_val) ? 1 : 0;
    checksum += (wide_val < narrow_val) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 result = (wide_val > 0) ? wide_val : (__int128)narrow_val;
    checksum += (result == wide_val) ? 4 : 0;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 mask = ((__int128)0xFFFFFFFFULL << 32) | 0xFFFFFFFFULL;
    __int128 masked = wide_val & mask;
    checksum += (masked < wide_val) ? 8 : 0;
    
    /* Shifts that move bits across the 64-bit boundary */
    __int128 shifted = ((__int128)1 << 60) >> 32;
    checksum += (shifted > 0) ? 16 : 0;
    
    return checksum;
}

/* Test 6: Compiler builtins with wide integers */
__attribute__((noinline))
int test_builtins(void) {
    int checksum = 0;
    unsigned __int128 x = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                          0xFEDCBA9876543210ULL;
    
    /* Use builtins that may require wide comparisons internally */
    if (__builtin_expect(x > ((unsigned __int128)1 << 127), 0)) {
        checksum += 1;
    }
    
    /* Count leading zeros on high and low parts */
    int clz_high = __builtin_clzll(x >> 64);
    int clz_low = __builtin_clzll((unsigned long long)x);
    checksum += (clz_high < clz_low) ? 2 : 0;
    
    /* Population count across both words */
    int popcnt = __builtin_popcountll(x >> 64) + __builtin_popcountll((unsigned long long)x);
    checksum += (popcnt > 64) ? 4 : 0;
    
    return checksum;
}

/* Test 7: Switch statement with __int128 case labels */
__attribute__((noinline))
int test_switch_statement(__int128 value) {
    /* Force compiler to generate comparison tree for switch */
    switch (value) {
        case ((__int128)0x1000000000000000ULL << 64):
            return 1;
        case ((__int128)0x2000000000000000ULL << 64):
            return 2;
        case ((__int128)0x3000000000000000ULL << 64):
            return 3;
        case 0:
            return 4;
        case -((__int128)0x1000000000000000ULL << 64):
            return 5;
        default:
            return (value > 0) ? 6 : 7;
    }
}

/* Test 8: Variadic function with __int128 conversion */
__attribute__((noinline))
int test_variadic_conversion(void) {
    int checksum = 0;
    __int128 wide = ((__int128)0x123456789ABCDEF0ULL << 64);
    
    /* Force conversion sequences */
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), 
                      "High: %llu, Low: %llu",
                      (unsigned long long)(wide >> 64),
                      (unsigned long long)wide);
    checksum += len;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests to exercise different comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_range_analysis();
    total_checksum += test_overflow_checks();
    total_checksum += test_mixed_precision();
    total_checksum += test_builtins();
    
    /* Test switch with values that exercise high-word comparisons */
    total_checksum += test_switch_statement(((__int128)0x2000000000000000ULL << 64));
    total_checksum += test_switch_statement(0);
    total_checksum += test_switch_statement(-((__int128)0x1000000000000000ULL << 64));
    
    total_checksum += test_variadic_conversion();
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Additional constant expressions to force compile-time evaluation */
    const __int128 const_compare_1 = ((__int128)HIGH_BIT_64 << 64);
    const __int128 const_compare_2 = ((__int128)(HIGH_BIT_64 >> 1) << 64);
    
    /* These should trigger constant folding with double_int comparisons */
    if (__builtin_constant_p(const_compare_1 > const_compare_2)) {
        printf("Constant folding triggered for wide int comparison\n");
    }
    
    return total_checksum != 0 ? 0 : 1;
}
