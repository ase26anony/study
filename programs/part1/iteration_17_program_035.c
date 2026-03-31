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
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > (__int128)0, 
               "High-bit shift must be positive");
_Static_assert(((__int128)0 - ((__int128)HIGH_BIT_64 << 64)) < (__int128)0,
               "Negative large value check");

/* Test function that forces range analysis on __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = start; i < end && i < start + 100; ++i) {
        /* Mix operations that may overflow */
        sum += i * 3;
        
        /* Force comparisons in both high and low words */
        if (i > (__int128)HIGH_BIT_64 << 63) {
            sum -= HIGH_BIT_64;
        } else if (i < -((__int128)HIGH_BIT_64 << 63)) {
            sum += HIGH_BIT_64;
        }
    }
    return sum;
}

/* Function using builtin overflow checks with __int128 */
static int check_overflow_operations(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger internal double_int comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) return -2;
    
    overflow = __builtin_sub_overflow(a, b, &result);
    if (overflow) return -3;
    
    return 0;
}

/* Function with switch statement using __int128 case labels */
static int switch_on_128(__int128 val) {
    /* GCC may generate comparison trees for these cases */
    switch (val) {
        case ((__int128)0x1ULL << 120):  /* Crosses 64-bit boundary */
            return 1;
        case -((__int128)0x1ULL << 120):
            return 2;
        case ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64:
            return 3;
        case 0:
            return 4;
        default:
            /* Force comparison of high words */
            if (val > ((__int128)HIGH_BIT_64 << 64))
                return 5;
            if (val < -((__int128)HIGH_BIT_64 << 64))
                return 6;
            return 0;
    }
}

/* Mixed precision comparisons */
static int mixed_comparisons(__int128 a, unsigned __int128 b, long long c) {
    int result = 0;
    
    /* Compare __int128 with narrower types */
    if (a > c) result |= 0x01;
    if ((unsigned __int128)a < b) result |= 0x02;
    if (a == (__int128)c) result |= 0x04;
    
    /* Ternary with mixed types */
    __int128 ternary_result = (a > 0) ? a : (__int128)c;
    if (ternary_result != a && ternary_result != (__int128)c) {
        result |= 0x08;
    }
    
    return result;
}

/* Bitwise operations crossing 64-bit boundary */
static unsigned __int128 cross_boundary_bitops(unsigned __int128 x) {
    /* Shift across 64-bit boundary */
    unsigned __int128 shifted = (x << 65) | (x >> 63);
    
    /* Mask operations affecting both words */
    unsigned __int128 masked = shifted & 
        (((unsigned __int128)MAX_64 << 64) | HIGH_BIT_64);
    
    /* Count bits - may trigger internal wide int ops */
    int popcount = __builtin_popcountll((unsigned long long)(masked >> 64)) +
                   __builtin_popcountll((unsigned long long)masked);
    
    return masked + popcount;
}

int main(void) {
    /* Array of test values exercising different comparison paths */
    __int128 test_values[8] = {
        /* 1. High words differ (both positive) */
        ((__int128)HIGH_BIT_64 << 64) | 0x1ULL,          /* High word: 0x8000000000000000 */
        ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x1ULL,   /* High word: 0x4000000000000000 */
        
        /* 2. High words equal, low words differ */
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111ULL,
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222ULL,
        
        /* 3. Negative values with different high words */
        -((__int128)HIGH_BIT_64 << 64) | 0x1ULL,
        -((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x1ULL,
        
        /* 4. Boundary values */
        ((__int128)MAX_64 << 64) | MAX_64,  /* Near INT128_MAX */
        -((__int128)MAX_64 << 64) | MAX_64, /* Near INT128_MIN */
    };
    
    unsigned __int128 unsigned_values[4] = {
        ((unsigned __int128)MAX_64 << 64) | MAX_64,  /* UINT128_MAX */
        ((unsigned __int128)HIGH_BIT_64 << 64),      /* High bit set */
        0xFFFFFFFF00000000ULL,                       /* Only low word */
        ((unsigned __int128)0x1ULL << 127),          /* Maximum single bit */
    };
    
    int checksum = 0;
    
    /* Test 1: Direct comparisons between array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_values[i] < test_values[j]) checksum += 1;
            if (test_values[i] > test_values[j]) checksum += 2;
            if (test_values[i] == test_values[j]) checksum += 4;
            
            /* Force unsigned comparison of signed values */
            if ((unsigned __int128)test_values[i] < 
                (unsigned __int128)test_values[j]) {
                checksum += 8;
            }
        }
    }
    
    /* Test 2: Range analysis with loops */
    __int128 range_result = process_range(
        -((__int128)HIGH_BIT_64 << 62),  /* Negative large */
        ((__int128)HIGH_BIT_64 << 62)    /* Positive large */
    );
    checksum += (int)(range_result & 0x7FFFFFFF);
    
    /* Test 3: Overflow checking */
    checksum += check_overflow_operations(
        ((__int128)HIGH_BIT_64 << 63),
        ((__int128)HIGH_BIT_64 << 63)
    );
    
    /* Test 4: Switch statement with __int128 */
    for (int i = 0; i < 8; i++) {
        checksum += switch_on_128(test_values[i]);
    }
    
    /* Test 5: Mixed precision comparisons */
    for (int i = 0; i < 4; i++) {
        checksum += mixed_comparisons(
            test_values[i],
            unsigned_values[i],
            (long long)test_values[i]
        );
    }
    
    /* Test 6: Bitwise operations */
    for (int i = 0; i < 4; i++) {
        unsigned __int128 bit_result = cross_boundary_bitops(unsigned_values[i]);
        checksum += (int)(bit_result & 0x7FFFFFFF);
    }
    
    /* Test 7: Builtin expect with __int128 comparisons */
    for (int i = 0; i < 7; i++) {
        if (__builtin_expect(test_values[i] < test_values[i+1], 1)) {
            checksum += i;
        }
    }
    
    /* Test 8: Variadic function with __int128 conversion */
    printf("Checksum: %d\n", checksum);
    
    /* Additional printf to force conversions */
    for (int i = 0; i < 2; i++) {
        printf("Value %d low: %lld, high: %lld\n", 
               i,
               (long long)(test_values[i] & MAX_64),
               (long long)((test_values[i] >> 64) & MAX_64));
    }
    
    return checksum != 0 ? 0 : 1;
}
