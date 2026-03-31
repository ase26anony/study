/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONST_128 ((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64)
#define NEG_LARGE_CONST_128 ((__int128)(-1) << 120)

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFLL << 64) > 
               ((__int128)0x3FFFFFFFFFFFFFFFLL << 64), 
               "High word comparison should be positive");

_Static_assert(((__int128)0x8000000000000000ULL << 64) < 
               ((__int128)0x4000000000000000ULL << 64), 
               "High word comparison with sign extension");

/* Test function that exercises range analysis on __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = start; i < end; i += (HIGH_BIT_64 >> 61)) {
        /* Force comparisons that may use double_int::cmp */
        if (i < (__int128)0) {
            sum += i;
        } else if (i > (__int128)(HIGH_BIT_64 << 32)) {
            sum -= i;
        } else {
            sum ^= i;
        }
        
        /* Overflow checks that require wide comparisons */
        __int128 overflow_test;
        if (__builtin_add_overflow(sum, i, &overflow_test)) {
            sum = i;
        }
    }
    return sum;
}

/* Function with mixed-precision operations */
static unsigned __int128 mixed_precision_compare(unsigned long long a, __int128 b) {
    /* Compare __int128 with narrower types */
    if (b > (__int128)a) {
        return (unsigned __int128)(b - a);
    } else if ((unsigned __int128)a > (unsigned __int128)b) {
        return (unsigned __int128)(a - b);
    }
    
    /* Ternary with mixed types */
    return (a & 1) ? (unsigned __int128)b : (unsigned __int128)(b >> 1);
}

/* Test switch with __int128 case labels (compile-time constants) */
static int test_switch(__int128 value) {
    switch ((unsigned __int128)value >> 120) {
        case 0: return 1;
        case 1: return 2;
        case ((unsigned __int128)HIGH_BIT_64 >> 56): return 3;
        default: return 0;
    }
}

/* Array operations to give optimizer substantial work */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Compare values where only high words differ */
        if (arr[i] < LARGE_CONST_128) {
            checksum += arr[i];
        }
        
        /* Compare values where high words are equal but low words differ */
        __int128 masked = arr[i] & ((__int128)MAX_64 << 64);
        if (masked == ((__int128)0x123456789ABCDEF0ULL << 64)) {
            if (arr[i] < (masked | 0xFFFFFFFFULL)) {
                checksum -= arr[i];
            }
        }
        
        /* Bitwise operations crossing 64-bit boundary */
        __int128 shifted = (arr[i] << 5) | (arr[i] >> 123);
        checksum ^= shifted;
    }
    
    return checksum;
}

/* Use compiler built-ins with wide integers */
static int count_leading_zeros(__int128 value) {
    /* Operations that may trigger internal comparisons */
    if (value == 0) return 128;
    
    unsigned __int128 uval = (unsigned __int128)value;
    if (uval & ((unsigned __int128)MAX_64 << 64)) {
        return __builtin_clzll((unsigned long long)(uval >> 64));
    } else {
        return 64 + __builtin_clzll((unsigned long long)uval);
    }
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Compare values where only high words differ */
    __int128 val1 = (__int128)0x123456789ABCDEF0ULL << 64;
    __int128 val2 = (__int128)0x123456789ABCDEF1ULL << 64;
    
    if (val1 < val2) checksum += 1;
    if (val2 > val1) checksum += 2;
    
    /* Test 2: Compare values where high words are equal but low words differ */
    __int128 val3 = val1 | 0xFFFFFFFFULL;
    __int128 val4 = val1 | 0xFFFFFFFEULL;
    
    if (val3 > val4) checksum += 4;
    if (val4 < val3) checksum += 8;
    
    /* Test 3: Boundary comparisons */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = ((__int128)0x8000000000000000ULL << 64);
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    if (max_signed > min_signed) checksum += 16;
    if ((unsigned __int128)max_unsigned > (unsigned __int128)max_signed) checksum += 32;
    
    /* Test 4: Arithmetic with overflow checking */
    __int128 overflow_test;
    if (!__builtin_add_overflow(max_signed, max_signed, &overflow_test)) {
        checksum += 64;
    }
    
    /* Test 5: Range analysis triggers */
    checksum += process_range(min_signed >> 4, max_signed >> 4);
    
    /* Test 6: Mixed precision operations */
    checksum += mixed_precision_compare(ULLONG_MAX, max_signed);
    
    /* Test 7: Array operations (at least 8 elements) */
    __int128 arr[8] = {
        (__int128)0x1000000000000000ULL << 64,
        (__int128)0x2000000000000000ULL << 64,
        (__int128)0x3000000000000000ULL << 64,
        (__int128)0x4000000000000000ULL << 64,
        (__int128)0x5000000000000000ULL << 64,
        (__int128)0x6000000000000000ULL << 64,
        (__int128)0x7000000000000000ULL << 64,
        (__int128)0x8000000000000000ULL << 64
    };
    
    checksum += process_array(arr, 8);
    
    /* Test 8: Built-in functions */
    checksum += count_leading_zeros(max_signed);
    checksum += test_switch(NEG_LARGE_CONST_128);
    
    /* Use __builtin_expect with __int128 comparisons */
    if (__builtin_expect(val1 < val2, 1)) {
        checksum += 128;
    }
    
    /* Force printf conversion sequences */
    printf("Checksum (low 64 bits): %llu\n", (unsigned long long)checksum);
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)((unsigned __int128)checksum >> 64));
    
    /* Additional compile-time forced comparisons */
#if ((__int128)0x8000000000000000ULL << 64) < 0
    printf("Negative large constant detected\n");
#endif
    
    return 0;
}
