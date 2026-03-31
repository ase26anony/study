/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * for 128-bit integer operations during constant folding and optimization.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit set in __int128 positive comparison");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < -((__int128)HIGH_BIT_64 << 63),
               "Signed __int128 boundary comparison");

/* Test function that exercises range analysis with __int128 */
static __int128 range_analysis_test(unsigned long long seed) {
    __int128 result = 0;
    __int128 base = ((__int128)seed << 64) | seed;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = -((__int128)1 << 62); i < ((__int128)1 << 62); i += (1 << 40)) {
        /* Force comparisons where high words might differ */
        if (i < base) {
            result += i;
        } else if (i > base + HIGH_WORD_DIFF) {
            result -= i;
        }
        
        /* Mixed-precision comparison */
        if ((unsigned long long)i < seed) {
            result ^= i;
        }
    }
    
    return result;
}

/* Test overflow operations that require wide comparisons */
static int test_overflow_checks(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow_add = 0, overflow_mul = 0, overflow_sub = 0;
    
    /* Use builtins for overflow checking */
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_sub = __builtin_sub_overflow(a, b, &diff);
    overflow_mul = __builtin_mul_overflow(a, b, &prod);
    
    /* Comparisons that exercise high/low word logic */
    if (sum > (((__int128)MAX_UINT64 << 64) | MAX_UINT64)) {
        return -1;
    }
    if (diff < -(((__int128)MAX_UINT64 << 64) | MAX_UINT64)) {
        return 1;
    }
    
    return overflow_add + overflow_mul * 2 + overflow_sub * 4;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int switch_on_int128(__int128 value) {
    switch (value) {
        case (((__int128)0x1ULL << 64) | 0x1ULL):
            return 1;
        case (((__int128)0x1ULL << 64) | 0x2ULL):
            return 2;
        case (((__int128)0x2ULL << 64) | 0x1ULL):  /* High word differs */
            return 3;
        case -(((__int128)0x1ULL << 64) | 0x1ULL): /* Negative value */
            return 4;
        case (((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64):
            return 5;
        default:
            return 0;
    }
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 cross_boundary_bitops(__int128 a, __int128 b) {
    __int128 result = a;
    
    /* Shift operations that affect high word */
    result = (result << 65) | (result >> 63);
    
    /* Bitwise operations */
    result &= ~b;  /* Complement might affect high word */
    result |= ((__int128)b << 32);
    
    /* Mask operations that isolate high/low words */
    __int128 high_mask = ((__int128)MAX_UINT64 << 64);
    __int128 low_mask = MAX_UINT64;
    
    if ((result & high_mask) > (a & high_mask)) {
        result ^= low_mask;
    }
    
    return result;
}

/* Variadic function to force conversions */
static void print_int128_variadic(__int128 value) {
    /* Force conversion sequences */
    printf("High: %llx, Low: %llx\n", 
           (unsigned long long)(value >> 64),
           (unsigned long long)(value & MAX_UINT64));
    
    /* Mixed comparisons in printf arguments */
    printf("Compare with 64-bit: %d\n", 
           value > (unsigned long long)(value & MAX_UINT64));
}

/* Use builtins that may trigger double_int comparisons */
static int builtin_operations(__int128 a, __int128 b) {
    int result = 0;
    
    /* __builtin_expect with wide comparisons */
    if (__builtin_expect(a < b, 1)) {
        result |= 1;
    }
    
    if (__builtin_expect((unsigned __int128)a > (unsigned __int128)b, 0)) {
        result |= 2;
    }
    
    /* Count leading zeros on high word */
    if (a > 0) {
        unsigned long long high = a >> 64;
        result += __builtin_clzll(high);
    }
    
    return result;
}

/* Main test function with array operations */
int main(void) {
    /* Array of __int128 values that exercise different comparison paths */
    __int128 test_values[8] = {
        0,  /* Zero */
        ((__int128)1 << 64) | 1,  /* High = 1, Low = 1 */
        ((__int128)1 << 64) | 2,  /* High = 1, Low = 2 (low word differs) */
        ((__int128)2 << 64) | 1,  /* High = 2, Low = 1 (high word differs) */
        -(((__int128)1 << 64) | 1),  /* Negative with both words set */
        ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64,  /* High bit set in both */
        ((__int128)MAX_UINT64 << 64) | MAX_UINT64,  /* Max positive */
        -(((__int128)MAX_UINT64 << 64) | MAX_UINT64), /* Min negative-ish */
    };
    
    unsigned __int128 unsigned_values[4] = {
        0,
        ((unsigned __int128)1 << 64) | 1,
        ((unsigned __int128)MAX_UINT64 << 64) | MAX_UINT64,
        ((unsigned __int128)HIGH_BIT_64 << 64) | HIGH_BIT_64,
    };
    
    int checksum = 0;
    
    /* Test 1: Compare values where high words differ */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_values[i] < test_values[j]) checksum += 1;
            if (test_values[i] > test_values[j]) checksum += 2;
            if (test_values[i] == test_values[j]) checksum += 4;
            
            /* Unsigned comparisons */
            unsigned __int128 ui = (unsigned __int128)test_values[i];
            unsigned __int128 uj = (unsigned __int128)test_values[j];
            if (ui < uj) checksum += 8;
            if (ui > uj) checksum += 16;
        }
    }
    
    /* Test 2: Range analysis with loops */
    for (unsigned long long seed = 0; seed < 4; seed++) {
        __int128 range_result = range_analysis_test(seed);
        checksum += (int)(range_result & 0x7FFFFFFF);
    }
    
    /* Test 3: Overflow checks */
    for (int i = 0; i < 8; i += 2) {
        checksum += test_overflow_checks(test_values[i], test_values[i+1]);
    }
    
    /* Test 4: Switch statements */
    for (int i = 0; i < 8; i++) {
        checksum += switch_on_int128(test_values[i]);
    }
    
    /* Test 5: Bitwise operations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            __int128 bit_result = cross_boundary_bitops(test_values[i], test_values[j]);
            checksum += (int)(bit_result & 0xFF);
        }
    }
    
    /* Test 6: Builtin operations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += builtin_operations(test_values[i], test_values[j]);
        }
    }
    
    /* Test 7: Ternary operators with mixed types */
    for (int i = 0; i < 8; i++) {
        long long narrow = (long long)test_values[i];
        __int128 ternary_result = (i & 1) ? test_values[i] : narrow;
        checksum += (ternary_result > 0) ? 1 : -1;
    }
    
    /* Test 8: Boundary comparisons */
    __int128 max_signed = ((__int128)MAX_UINT64 << 63) - 1;
    __int128 min_signed = -max_signed - 1;
    unsigned __int128 max_unsigned = ~(unsigned __int128)0;
    
    if (test_values[6] < max_signed) checksum += 1000;
    if (test_values[7] > min_signed) checksum += 2000;
    if (unsigned_values[2] < max_unsigned) checksum += 3000;
    
    /* Print final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Variadic function calls */
    for (int i = 0; i < 8; i++) {
        print_int128_variadic(test_values[i]);
    }
    
    return 0;
}
