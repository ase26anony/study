/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * for 128-bit integer operations during constant folding and optimization.
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

/* Test function that exercises __int128 range analysis */
static __int128 range_analysis_test(unsigned long long seed) {
    __int128 result = 0;
    __int128 base = ((__int128)seed << 64) | seed;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -((__int128)1 << 70); i < ((__int128)1 << 70); i += (1ULL << 60)) {
        if (i < base) {
            result += i;
        } else {
            result -= i;
        }
    }
    return result;
}

/* Test overflow operations with __int128 */
static int test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow_add, overflow_mul;
    
    /* Use builtins for overflow checking */
    overflow_add = __builtin_add_overflow(a, b, &sum);
    __builtin_mul_overflow(a, b, &prod);
    
    /* Force comparisons that may use double_int */
    if (sum > a && sum > b && !overflow_add) {
        diff = sum - a;
        if (diff == b) {
            return 1;
        }
    }
    
    /* Compare with mixed precision */
    if (prod > (long long)prod) {
        return 2;
    }
    
    return 0;
}

/* Exercise bitwise operations across 64-bit boundary */
static unsigned __int128 bitwise_test(unsigned __int128 x) {
    /* Operations that cross the 64-bit boundary */
    unsigned __int128 y = (x << 64) | (x >> 64);
    unsigned __int128 z = y & ~((unsigned __int128)MAX_64 << 64);
    unsigned __int128 w = z | ((unsigned __int128)0xFFFFFFFF << 96);
    
    /* Comparisons between results */
    if (y > z && z < w && w > x) {
        return y ^ z ^ w;
    }
    return x;
}

/* Switch statement with __int128 cases (compile-time constants) */
static int switch_test(__int128 value) {
    /* GCC may generate comparison trees for these cases */
    switch ((unsigned __int128)value & 0xFF) {
        case ((unsigned __int128)0x8000000000000000ULL >> 56):
            return 1;
        case ((unsigned __int128)MAX_64 >> 56):
            return 2;
        case ((unsigned __int128)0x123456789ABCDEF0ULL >> 56):
            return 3;
        default:
            return 0;
    }
}

/* Variadic function to force conversions */
static void print_128(__int128 val) {
    /* Force conversion sequences */
    printf("High: %lld, Low: %lld\n", 
           (long long)(val >> 64), 
           (long long)(val & MAX_64));
}

/* Main test function with comprehensive __int128 operations */
int main(void) {
    unsigned long long checksum = 0;
    
    /* Array of __int128 values for optimizer to work on */
    __int128 test_values[8] = {
        ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64,  /* Negative value */
        ((__int128)MAX_64 << 64) | MAX_64,            /* Large positive */
        ((__int128)MID_128 << 64) | MID_128,          /* Mid-range */
        -((__int128)1 << 120),                        /* Very negative */
        ((__int128)1 << 120) - 1,                     /* Near max positive */
        0,
        -1,
        ((__int128)0x12345678 << 96) | ((__int128)0x9ABCDEF0 << 32) | 0x2468ACE0
    };
    
    /* Test 1: Comparisons with high word differences */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cmp;
            if (test_values[i] < test_values[j]) {
                cmp = -1;
            } else if (test_values[i] > test_values[j]) {
                cmp = 1;
            } else {
                cmp = 0;
            }
            checksum += (unsigned long long)cmp + i + j;
            
            /* Use __builtin_expect to influence branch prediction */
            if (__builtin_expect(test_values[i] != test_values[j], 1)) {
                checksum ^= (unsigned long long)(test_values[i] >> 64);
            }
        }
    }
    
    /* Test 2: Range analysis with loops */
    for (unsigned long long seed = 0; seed < 4; seed++) {
        __int128 result = range_analysis_test(seed);
        checksum += (unsigned long long)(result >> 64) + (unsigned long long)result;
    }
    
    /* Test 3: Overflow operations */
    for (int i = 0; i < 7; i += 2) {
        checksum += test_overflow_ops(test_values[i], test_values[i+1]);
    }
    
    /* Test 4: Bitwise operations */
    unsigned __int128 bit_result = 0;
    for (int i = 0; i < 8; i++) {
        bit_result ^= bitwise_test((unsigned __int128)test_values[i]);
    }
    checksum += (unsigned long long)(bit_result >> 64) + (unsigned long long)bit_result;
    
    /* Test 5: Mixed precision comparisons */
    for (int i = 0; i < 8; i++) {
        /* Compare __int128 with narrower types */
        if (test_values[i] > (long long)test_values[i]) {
            checksum += i * 3;
        }
        if ((unsigned __int128)test_values[i] > (unsigned long long)test_values[i]) {
            checksum += i * 7;
        }
        
        /* Ternary with mixed types */
        __int128 ternary_result = (i & 1) ? test_values[i] : (__int128)(i * 1000);
        checksum += (unsigned long long)(ternary_result & 0xFF);
        
        /* Switch test */
        checksum += switch_test(test_values[i]);
    }
    
    /* Test 6: Built-in functions */
    for (int i = 0; i < 8; i++) {
        /* Count leading zeros in high and low parts */
        int clz_high = __builtin_clzll(test_values[i] >> 64);
        int clz_low = __builtin_clzll((unsigned long long)test_values[i]);
        checksum += clz_high + clz_low;
        
        /* Population count */
        int popcnt = __builtin_popcountll(test_values[i] >> 64) +
                    __builtin_popcountll((unsigned long long)test_values[i]);
        checksum += popcnt;
    }
    
    /* Test 7: Boundary value comparisons */
    __int128 max_signed = ((__int128)1 << 127) - 1;
    __int128 min_signed = -((__int128)1 << 127);
    unsigned __int128 max_unsigned = ~((unsigned __int128)0);
    
    /* Exercise all comparison operators at boundaries */
    checksum += (max_signed > min_signed) ? 1 : 0;
    checksum += (max_signed < max_unsigned) ? 2 : 0;
    checksum += (min_signed < 0) ? 4 : 0;
    checksum += ((unsigned __int128)max_signed > max_unsigned / 2) ? 8 : 0;
    
    /* Force constant folding with arithmetic */
    __int128 folded = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    folded = folded * 3 / 2 + 1;
    checksum += (unsigned long long)(folded >> 64) ^ (unsigned long long)folded;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %llu\n", checksum);
    
    /* Print some values to force variadic conversions */
    print_128(test_values[0]);
    print_128(max_signed);
    print_128(min_signed);
    
    return (int)(checksum & 0xFF);
}
