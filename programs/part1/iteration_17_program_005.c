/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define MID_RANGE_64  0x7FFFFFFFFFFFFFFFULL

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > ((__int128)MID_RANGE_64 << 64), 
               "High word comparison test 1");
_Static_assert(((__int128)MAX_UINT64) < ((__int128)MAX_UINT64 << 64), 
               "High word comparison test 2");

/* Test function that forces range analysis on __int128 */
static __int128 process_int128(__int128 a, __int128 b, int mode) {
    __int128 result = 0;
    
    /* Different comparison patterns to exercise all paths */
    if (mode == 0) {
        /* Compare where high words differ (both positive) */
        if (a > b) result = a - b;
        else if (a < b) result = b - a;
        else result = 1;
    } else if (mode == 1) {
        /* Compare where high words differ (negative values) */
        if (a < b) result = a * 2;
        else result = b * 2;
    } else if (mode == 2) {
        /* Compare where high words equal, low words differ */
        result = (a == b) ? 0 : ((a > b) ? 1 : -1);
    }
    
    return result;
}

/* Function to test overflow operations with __int128 */
static int test_overflow_checks(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow = 0;
    
    /* Use builtins that may trigger internal comparisons */
    overflow |= __builtin_add_overflow(a, b, &sum);
    overflow |= __builtin_sub_overflow(a, b, &diff);
    overflow |= __builtin_mul_overflow(a, b, &prod);
    
    /* Comparisons that exercise the high/low word logic */
    if (sum > a && sum > b) overflow |= 0x1;
    if (diff < a && diff < b) overflow |= 0x2;
    if (prod / a != b && a != 0) overflow |= 0x4;
    
    return overflow;
}

/* Mixed precision operations */
static __int128 mixed_precision_ops(unsigned long long ull, size_t sz, __int128 i128) {
    /* Force conversions and comparisons between different types */
    __int128 result = 0;
    
    if ((__int128)ull > i128) result += 1;
    if ((size_t)i128 > sz) result += 2;
    
    /* Ternary with mixed types */
    result += (i128 > 0) ? (__int128)ull : (__int128)sz;
    
    return result;
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 cross_boundary_bitops(__int128 val) {
    __int128 result = val;
    
    /* Shift operations that move bits across the 64-bit boundary */
    result = (result << 65) | (result >> 63);
    
    /* Bitwise operations */
    result &= ~((__int128)HIGH_BIT_64 << 64);
    result |= ((__int128)MAX_UINT64 << 32);
    
    /* Mask that spans both words */
    __int128 mask = ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL;
    result ^= mask;
    
    return result;
}

/* Loop with __int128 induction variable */
static __int128 int128_loop(__int128 start, __int128 end, __int128 step) {
    __int128 accum = 0;
    
    /* Loop that may trigger range analysis */
    for (__int128 i = start; i < end; i += step) {
        /* Comparisons in loop condition exercise the target code */
        if (i > start + (step * 10)) {
            accum += i;
        } else if (i < start - (step * 5)) {
            accum -= i;
        } else {
            accum ^= i;
        }
        
        /* Additional comparison to ensure all paths are taken */
        if ((i & ((__int128)1 << 70)) != 0) {
            accum >>= 1;
        }
    }
    
    return accum;
}

/* Switch with __int128 cases (compile-time constants) */
static int int128_switch(__int128 key) {
    int result = 0;
    
    /* GCC must generate comparison trees for these cases */
    switch ((unsigned __int128)key) {
        case ((unsigned __int128)0ULL):
            result = 1;
            break;
        case ((unsigned __int128)MAX_UINT64):
            result = 2;
            break;
        case ((unsigned __int128)MAX_UINT64 << 64):
            result = 3;
            break;
        case ((unsigned __int128)MAX_UINT64 << 64 | MAX_UINT64):
            result = 4;
            break;
        case ((unsigned __int128)HIGH_BIT_64 << 64):
            result = 5;
            break;
        default:
            /* Force comparison with boundary values */
            if (key == ((__int128)HIGH_BIT_64 << 64)) result = 6;
            else if (key == -((__int128)HIGH_BIT_64 << 64)) result = 7;
            else result = 8;
    }
    
    return result;
}

/* Use compiler builtins with __int128 */
static int builtin_int128_ops(__int128 val) {
    int result = 0;
    
    /* Count leading zeros - may trigger internal conversions */
    if (val > 0) {
        unsigned long long low = (unsigned long long)val;
        unsigned long long high = (unsigned long long)(val >> 64);
        result += __builtin_clzll(high);
        result += __builtin_ctzll(low);
        result += __builtin_popcountll(low ^ high);
    }
    
    /* Branch prediction with __int128 comparison */
    if (__builtin_expect(val != 0, 1)) {
        result ^= 0xFF;
    }
    
    return result;
}

int main(void) {
    __int128 checksum = 0;
    
    /* Array of test values that exercise different comparison scenarios */
    __int128 test_values[8] = {
        0,  /* Zero */
        ((__int128)1 << 63),  /* Just below 64-bit boundary */
        ((__int128)1 << 64),  /* Exactly crossing 64-bit boundary */
        ((__int128)HIGH_BIT_64 << 64),  /* High word has high bit set */
        -((__int128)HIGH_BIT_64 << 64), /* Large negative */
        ((__int128)MAX_UINT64 << 64) | MAX_UINT64,  /* Max positive */
        ~((__int128)MAX_UINT64 << 64),  /* High word all 1s except MSB */
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
    };
    
    /* Test 1: Direct comparisons between array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_values[i] < test_values[j]) checksum += 1;
            if (test_values[i] > test_values[j]) checksum += 2;
            if (test_values[i] == test_values[j]) checksum += 3;
            if (test_values[i] != test_values[j]) checksum += 4;
        }
    }
    
    /* Test 2: Process with different comparison modes */
    for (int i = 0; i < 8; i += 2) {
        checksum += process_int128(test_values[i], test_values[i+1], i % 3);
    }
    
    /* Test 3: Overflow checks */
    for (int i = 0; i < 8; i++) {
        checksum += test_overflow_checks(test_values[i], test_values[(i+1)%8]);
    }
    
    /* Test 4: Mixed precision operations */
    for (int i = 0; i < 8; i++) {
        checksum += mixed_precision_ops(
            (unsigned long long)test_values[i],
            (size_t)test_values[i],
            test_values[i]
        );
    }
    
    /* Test 5: Bitwise operations */
    for (int i = 0; i < 8; i++) {
        checksum += cross_boundary_bitops(test_values[i]);
    }
    
    /* Test 6: Loop with __int128 */
    checksum += int128_loop(test_values[0], test_values[4], test_values[1]);
    
    /* Test 7: Switch statement */
    for (int i = 0; i < 8; i++) {
        checksum += int128_switch(test_values[i]);
    }
    
    /* Test 8: Builtin operations */
    for (int i = 0; i < 8; i++) {
        checksum += builtin_int128_ops(test_values[i]);
    }
    
    /* Test 9: Arithmetic with overflow simulation */
    for (int i = 0; i < 8; i++) {
        __int128 temp = test_values[i];
        for (int j = 0; j < 4; j++) {
            temp = temp * 3 + 1;
            /* Force comparison after each step */
            if (temp > test_values[i]) checksum += temp;
            else if (temp < test_values[i]) checksum -= temp;
        }
    }
    
    /* Test 10: Boundary value comparisons */
    __int128 boundaries[4] = {
        0,
        ((__int128)1 << 127) - 1,  /* INT128_MAX approx */
        (__int128)1 << 127,        /* INT128_MIN approx */
        ~(__int128)0               /* UINT128_MAX */
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Unsigned comparisons */
            if ((unsigned __int128)boundaries[i] < (unsigned __int128)boundaries[j])
                checksum += 1;
            if ((unsigned __int128)boundaries[i] > (unsigned __int128)boundaries[j])
                checksum += 2;
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    /* Split 128-bit checksum into two 64-bit parts for printing */
    unsigned long long low = (unsigned long long)checksum;
    unsigned long long high = (unsigned long long)(checksum >> 64);
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    
    /* Additional printf to force conversions */
    printf("Test values: ");
    for (int i = 0; i < 8; i++) {
        /* Force conversion to narrower type */
        printf("%lld ", (long long)test_values[i]);
    }
    printf("\n");
    
    return 0;
}
