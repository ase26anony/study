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
#define MID_128_HIGH  0x123456789ABCDEF0ULL
#define MID_128_LOW   0xFEDCBA9876543210ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "128-bit comparison with high word difference");

/* Test function that exercises __int128 range analysis */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit induction variable - forces VRP analysis */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Mixed-precision operations */
        unsigned long long narrow = (unsigned long long)i;
        __int128 wide = (__int128)narrow * (__int128)narrow;
        
        /* Comparisons that may trigger high-word checks */
        if (wide > i) {
            sum += 1;
        } else if (wide < i) {
            sum -= 1;
        }
        
        /* Bitwise operations crossing 64-bit boundary */
        __int128 shifted = i << 33;
        if ((shifted & ((__int128)HIGH_BIT_64 << 64)) != 0) {
            sum |= 1;
        }
    }
    return sum;
}

/* Function to test overflow detection with wide integers */
static int test_overflow_checks(void) {
    int overflow_count = 0;
    
    /* Test values that will exercise high-word comparisons */
    __int128 large_vals[] = {
        ((__int128)HIGH_BIT_64 << 64) | 0x1,          /* Negative large */
        ((__int128)MAX_64 << 64) | MAX_64,            /* Very large positive */
        ((__int128)MID_128_HIGH << 64) | MID_128_LOW, /* Mid-range */
        0,                                            /* Zero */
        -((__int128)HIGH_BIT_64 << 64),               /* Negative boundary */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64, /* Near max */
        ((__int128)0x1ULL << 64) | 0x1,               /* Small high word */
        ((__int128)0xFFFFFFFFULL << 64) | 0x0         /* Medium high word */
    };
    
    /* Test comparisons between all pairs */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Direct comparisons - should trigger double_int::cmp */
            if (large_vals[i] < large_vals[j]) {
                overflow_count += 1;
            }
            if (large_vals[i] > large_vals[j]) {
                overflow_count += 2;
            }
            if (large_vals[i] == large_vals[j]) {
                overflow_count += 4;
            }
            
            /* Arithmetic with overflow checking */
            __int128 result;
            if (__builtin_add_overflow(large_vals[i], large_vals[j], &result)) {
                overflow_count += 8;
            }
            
            /* Mixed-type comparisons */
            unsigned long long narrow = (unsigned long long)large_vals[i];
            if (large_vals[i] > (__int128)narrow) {
                overflow_count += 16;
            }
        }
    }
    
    return overflow_count;
}

/* Function using switch with __int128 cases (compile-time constants) */
static int test_switch(__int128 val) {
    /* GCC should generate comparison trees for these cases */
    switch (val) {
        case ((__int128)0x1ULL << 64):
            return 1;
        case ((__int128)HIGH_BIT_64 << 64):
            return 2;
        case ((__int128)MAX_64 << 64):
            return 3;
        case 0:
            return 4;
        case -((__int128)HIGH_BIT_64 << 64):
            return 5;
        default:
            /* Force comparison with default */
            if (val > ((__int128)MAX_64 << 63)) return 6;
            if (val < -((__int128)MAX_64 << 63)) return 7;
            return 0;
    }
}

/* Test bitwise operations and builtins */
static unsigned test_bitwise_ops(void) {
    unsigned __int128 uval1 = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 uval2 = ((unsigned __int128)0x5555555555555555ULL << 64) | 
                              0xAAAAAAAAAAAAAAAALL;
    
    /* Bitwise operations that cross word boundaries */
    unsigned __int128 and_result = uval1 & uval2;
    unsigned __int128 or_result = uval1 | uval2;
    unsigned __int128 xor_result = uval1 ^ uval2;
    unsigned __int128 shift_result = uval1 >> 65;  /* Crosses 64-bit boundary */
    
    /* Comparisons of results - should trigger unsigned high-word checks */
    unsigned checksum = 0;
    if (and_result < or_result) checksum += 1;
    if (xor_result > shift_result) checksum += 2;
    if ((and_result | or_result) == uval1) checksum += 4;
    
    /* Use builtins that may trigger wide comparisons */
    if (__builtin_expect((uval1 > uval2), 1)) {
        checksum += 8;
    }
    
    /* Population count across 128 bits */
    unsigned long long pop1 = __builtin_popcountll((unsigned long long)(uval1 >> 64));
    unsigned long long pop2 = __builtin_popcountll((unsigned long long)uval1);
    if ((pop1 + pop2) > 64) checksum += 16;
    
    return checksum;
}

/* Variadic function to force conversions */
static void test_variadic(__int128 val1, __int128 val2) {
    /* Force conversions through variadic arguments */
    printf("Comparison result: %d\n", val1 < val2);
    printf("High parts: %llx vs %llx\n", 
           (unsigned long long)(val1 >> 64),
           (unsigned long long)(val2 >> 64));
}

int main(void) {
    unsigned total_checksum = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    /* Test 1: Range analysis with loops */
    __int128 range_result = process_range(-1000, 1000);
    total_checksum += (unsigned)range_result;
    
    /* Test 2: Overflow and direct comparisons */
    int overflow_result = test_overflow_checks();
    total_checksum += overflow_result;
    
    /* Test 3: Switch statement with __int128 cases */
    __int128 test_vals[] = {
        ((__int128)0x1ULL << 64),
        ((__int128)HIGH_BIT_64 << 64),
        0,
        -((__int128)HIGH_BIT_64 << 64)
    };
    
    for (int i = 0; i < 4; i++) {
        total_checksum += test_switch(test_vals[i]);
    }
    
    /* Test 4: Bitwise operations and builtins */
    total_checksum += test_bitwise_ops();
    
    /* Test 5: Ternary operations with mixed types */
    __int128 large_val = ((__int128)MAX_64 << 64) | MAX_64;
    long long narrow_val = LLONG_MAX;
    
    /* Ternary with different types - forces conversions and comparisons */
    __int128 ternary_result = (large_val > (__int128)narrow_val) ? 
                              large_val : (__int128)narrow_val;
    total_checksum += (unsigned)(ternary_result & 0xFF);
    
    /* Test 6: Boundary comparisons */
    unsigned __int128 umax = ~((unsigned __int128)0);
    __int128 smax = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 smin = -smax - 1;
    
    if (umax > (unsigned __int128)smax) total_checksum += 0x100;
    if ((unsigned __int128)smax < umax) total_checksum += 0x200;
    if (smin < smax) total_checksum += 0x400;
    
    /* Test 7: Variadic conversions */
    test_variadic(((__int128)0x1ULL << 64) | 0x1, 
                  ((__int128)0x1ULL << 64) | 0x2);
    
    /* Final checksum to prevent dead code elimination */
    printf("Total checksum: 0x%08x\n", total_checksum);
    
    return (total_checksum == 0) ? 1 : 0;
}
