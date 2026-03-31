/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross the 64-bit boundary */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x7FFFFFFFFFFFFFFFULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "128-bit comparison with high word difference");

/* Test function that forces range analysis on __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable - forces VRP analysis */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Operations that may overflow */
        __int128 temp = i * i;
        
        /* Force comparisons at different boundaries */
        if (temp < 0) {
            sum += -temp;
        } else if (temp > ((__int128)HIGH_BIT_64 << 64)) {
            sum += temp >> 2;
        } else {
            sum += temp;
        }
        
        /* Mixed precision comparison */
        if (temp > LLONG_MAX) {
            sum -= 1;
        }
    }
    return sum;
}

/* Function using builtin overflow checks with __int128 */
static int check_overflow_operations(void) {
    __int128 a = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    __int128 b = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 result;
    int overflow;
    
    /* These builtins may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    overflow |= __builtin_mul_overflow(a, 2, &result);
    overflow |= __builtin_sub_overflow(b, a, &result);
    
    return overflow;
}

/* Switch statement with __int128 case labels */
static int test_switch(__int128 value) {
    /* Force compiler to generate comparison trees for switch */
    switch ((unsigned __int128)value) {
        case ((unsigned __int128)0x1ULL << 127):
            return 1;
        case ((unsigned __int128)MAX_64 << 64):
            return 2;
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            return 3;
        default:
            /* Compare high words specifically */
            if (((unsigned __int128)value >> 64) > (unsigned __int128)MAX_64 / 2) {
                return 4;
            }
            return 0;
    }
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 test_bitwise_ops(__int128 x, __int128 y) {
    __int128 result = 0;
    
    /* Operations that require handling both high and low words */
    result = (x & y) | ((x ^ y) << 64);
    result = (result >> 32) | (result << 32);
    result = (result & (((__int128)MAX_64 << 64) | MAX_64));
    
    /* Use builtins that may trigger wide comparisons */
    if (__builtin_expect((x > y) && (x < 0), 0)) {
        result = ~result;
    }
    
    return result;
}

/* Variadic function to force conversions */
static void print_128(__int128 value) {
    /* This forces conversion sequences */
    printf("High: 0x%016llX, Low: 0x%016llX\n", 
           (unsigned long long)((unsigned __int128)value >> 64),
           (unsigned long long)(value & MAX_64));
}

/* Main test function with comprehensive __int128 comparisons */
int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Compare values where only high words differ */
    __int128 test_cases[8] = {
        /* High word differences (signed) */
        ((__int128)1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)2ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        
        /* High word differences (unsigned) */
        ((__int128)HIGH_BIT_64 << 64) | 0x0ULL,
        ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x0ULL,
        
        /* Equal high words, different low words */
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL,
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL,
        
        /* Boundary values */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64,  /* Near INT128_MAX */
        ((__int128)HIGH_BIT_64 << 64) | 0x0ULL,            /* INT128_MIN */
    };
    
    /* Exercise all comparison paths */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_cases[i] < test_cases[j]) checksum += 1;
            if (test_cases[i] > test_cases[j]) checksum += 2;
            if (test_cases[i] <= test_cases[j]) checksum += 3;
            if (test_cases[i] >= test_cases[j]) checksum += 4;
            if (test_cases[i] == test_cases[j]) checksum += 5;
            if (test_cases[i] != test_cases[j]) checksum += 6;
        }
    }
    
    /* Test 2: Range analysis with loops */
    checksum += process_range(-((__int128)HIGH_BIT_64 << 63), 
                              ((__int128)HIGH_BIT_64 << 63));
    
    /* Test 3: Overflow checks */
    checksum += check_overflow_operations();
    
    /* Test 4: Switch statement with __int128 */
    for (int i = 0; i < 8; i++) {
        checksum += test_switch(test_cases[i]);
    }
    
    /* Test 5: Bitwise operations */
    __int128 bitwise_result = 0;
    for (int i = 0; i < 8; i += 2) {
        bitwise_result = test_bitwise_ops(test_cases[i], test_cases[i+1]);
        checksum += bitwise_result & 0xFF;
    }
    
    /* Test 6: Mixed precision comparisons */
    for (int i = 0; i < 8; i++) {
        /* Compare __int128 with 64-bit types */
        if (test_cases[i] > LLONG_MAX) checksum += 100;
        if (test_cases[i] < LLONG_MIN) checksum += 200;
        
        /* Ternary operator with mixed types */
        __int128 temp = (test_cases[i] > 0) ? test_cases[i] : (__int128)i;
        checksum += temp & 0xFF;
    }
    
    /* Test 7: Builtin functions */
    for (int i = 0; i < 8; i++) {
        unsigned __int128 uval = (unsigned __int128)test_cases[i];
        
        /* These may trigger internal wide comparisons */
        int clz = __builtin_clzll((unsigned long long)(uval >> 64));
        int ctz = __builtin_ctzll((unsigned long long)(uval & MAX_64));
        checksum += clz + ctz;
    }
    
    /* Print results to prevent dead code elimination */
    print_128(checksum);
    printf("Final checksum (low 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum & MAX_64));
    
    /* Additional static assertions to force constant folding */
    _Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) > 
                   ((__int128)0x3FFFFFFFFFFFFFFFULL << 64),
                   "Constant folding comparison 1");
    
    _Static_assert(((__int128)HIGH_BIT_64 << 64) < 0,
                   "Constant folding comparison 2");
    
    return 0;
}
