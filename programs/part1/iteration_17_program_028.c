/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "128-bit comparison with high word difference");

/* Test function that exercises __int128 comparisons */
static __int128 test_comparisons(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Direct comparisons that should use double_int::cmp */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    return result;
}

/* Test unsigned comparisons */
static unsigned __int128 test_unsigned_comparisons(unsigned __int128 a, 
                                                   unsigned __int128 b) {
    unsigned __int128 result = 0;
    
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    return result;
}

/* Function to trigger range analysis with __int128 */
static void analyze_ranges(__int128 start, __int128 end) {
    for (__int128 i = start; i < end; i = i + 1) {
        /* Force VRP to track 128-bit ranges */
        if (i > (start + (end - start) / 2)) {
            volatile __int128 temp = i * 2;  /* Prevent optimization */
            (void)temp;
        }
    }
}

/* Test overflow operations that require wide comparisons */
static int test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow_add = 0, overflow_sub = 0, overflow_mul = 0;
    
    /* Use builtins that may trigger double_int comparisons */
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_sub = __builtin_sub_overflow(a, b, &diff);
    overflow_mul = __builtin_mul_overflow(a, b, &prod);
    
    return overflow_add | (overflow_sub << 1) | (overflow_mul << 2);
}

/* Mixed-precision comparisons */
static int test_mixed_comparisons(__int128 a, long long b, size_t c) {
    int result = 0;
    
    /* Compare __int128 with narrower types */
    if (a > b) result |= 1;
    if (a < c) result |= 2;
    
    /* Ternary with mixed types */
    __int128 ternary_result = (a > 0) ? b : c;
    (void)ternary_result;
    
    return result;
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 test_bitwise_ops(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Operations that affect both high and low words */
    result = a & b;
    result |= a << 65;  /* Shift across 64-bit boundary */
    result |= b >> 33;
    
    /* Use builtins on parts of 128-bit values */
    int clz_high = __builtin_clzll((unsigned long long)(a >> 64));
    int popcount_low = __builtin_popcountll((unsigned long long)a);
    
    (void)clz_high;
    (void)popcount_low;
    
    return result;
}

/* Switch statement with __int128 cases (compile-time constants) */
static int test_switch(__int128 value) {
    switch ((unsigned __int128)value) {
        case 0ULL:
            return 0;
        case ((unsigned __int128)1 << 64):
            return 1;  /* Case where high word = 1, low word = 0 */
        case ((unsigned __int128)MAX_64 << 64) | MAX_64:
            return 2;  /* Maximum unsigned 128-bit value */
        case ((unsigned __int128)HIGH_BIT_64 << 64):
            return 3;  /* High bit set in high word */
        default:
            return -1;
    }
}

/* Main test function with array operations */
int main(void) {
    /* Array of test values that exercise different comparison paths */
    __int128 signed_values[8] = {
        0,
        -1,
        ((__int128)1 << 64),                     /* High word = 1 */
        ((__int128)-1 << 64),                    /* High word = -1 */
        ((__int128)HIGH_BIT_64 << 64),           /* INT128_MIN */
        ~((__int128)HIGH_BIT_64 << 64),          /* INT128_MAX */
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        -((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
    };
    
    unsigned __int128 unsigned_values[8] = {
        0,
        MAX_64,
        ((unsigned __int128)1 << 64),            /* High word = 1 */
        ((unsigned __int128)MAX_64 << 64),       /* High word = all 1s */
        ((unsigned __int128)MAX_64 << 64) | MAX_64, /* UINT128_MAX */
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)HIGH_BIT_64 << 64)   /* High bit set */
    };
    
    __int128 checksum = 0;
    
    /* Test 1: Compare values where high words differ */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += test_comparisons(signed_values[i], signed_values[j]);
            checksum += test_unsigned_comparisons(unsigned_values[i], unsigned_values[j]);
        }
    }
    
    /* Test 2: Range analysis with loops */
    analyze_ranges(-100, 100);
    analyze_ranges(((__int128)1 << 62), ((__int128)1 << 66));
    
    /* Test 3: Overflow operations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += test_overflow_ops(signed_values[i], signed_values[j]);
        }
    }
    
    /* Test 4: Mixed precision comparisons */
    for (int i = 0; i < 8; i++) {
        checksum += test_mixed_comparisons(signed_values[i], 
                                          (long long)signed_values[i],
                                          (size_t)unsigned_values[i]);
    }
    
    /* Test 5: Bitwise operations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += test_bitwise_ops(signed_values[i], signed_values[j]);
        }
    }
    
    /* Test 6: Switch statements */
    for (int i = 0; i < 8; i++) {
        checksum += test_switch(signed_values[i]);
        checksum += test_switch(unsigned_values[i]);
    }
    
    /* Test 7: Builtin expect with 128-bit comparisons */
    for (int i = 0; i < 8; i++) {
        if (__builtin_expect(signed_values[i] > 0, 1)) {
            checksum += 1;
        }
        if (__builtin_expect(unsigned_values[i] < ((unsigned __int128)1 << 127), 0)) {
            checksum += 2;
        }
    }
    
    /* Test 8: Variadic function with __int128 (triggers conversions) */
    for (int i = 0; i < 8; i++) {
        /* printf may trigger conversion sequences */
        volatile long long low_part = (long long)signed_values[i];
        volatile long long high_part = (long long)(signed_values[i] >> 64);
        (void)low_part;
        (void)high_part;
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum (low 64 bits): %lld\n", (long long)checksum);
    printf("Checksum (high 64 bits): %lld\n", (long long)(checksum >> 64));
    
    return 0;
}
