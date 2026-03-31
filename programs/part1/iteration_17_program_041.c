/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that will exercise high-word comparisons */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT 0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should create positive 128-bit value");
_Static_assert(((__int128)MAX_64 << 64) < (__int128)-1,
               "Comparison of high-word differing values");

/* Test 1: Comparisons where high words differ */
__attribute__((noinline))
int test_high_word_comparisons(void) {
    volatile __int128 a, b;
    int result = 0;
    
    /* Case 1: High words differ, both positive */
    a = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    b = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    result += (a < b) ? 1 : 0;  /* Should trigger high-word comparison */
    result += (a > b) ? 2 : 0;
    
    /* Case 2: High words differ, negative values */
    a = -(((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL);
    b = -(((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL);
    result += (a < b) ? 4 : 0;  /* Negative comparison */
    result += (a > b) ? 8 : 0;
    
    /* Case 3: High words equal, low words differ */
    a = ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    b = ((__int128)0x1ULL << 64) | 0x0ULL;
    result += (a < b) ? 16 : 0;  /* Should trigger low-word comparison */
    result += (a > b) ? 32 : 0;
    
    return result;
}

/* Test 2: Range analysis with loops */
__attribute__((noinline))
int test_range_analysis(void) {
    __int128 sum = 0;
    __int128 i;
    
    /* Loop that crosses 64-bit boundary */
    for (i = ((__int128)MAX_64 - 10); i < ((__int128)MAX_64 + 10); i++) {
        sum += i;
        
        /* Force range analysis on comparisons */
        if (i < 0) sum += 1;
        if (i > ((__int128)MAX_64 << 1)) sum += 2;
        
        /* Mixed precision comparison */
        if (i < (long long)MAX_64) sum += 4;
    }
    
    /* Another loop with unsigned __int128 */
    unsigned __int128 u = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    for (unsigned __int128 j = u - 5; j < u + 5; j++) {
        /* Compare near UINT128_MAX boundary */
        if (j > (u - 3)) sum += 8;
        if (j < (u + 3)) sum += 16;
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* Test 3: Overflow operations requiring wide comparisons */
__attribute__((noinline))
int test_overflow_checks(void) {
    int result = 0;
    __int128 x, y, z;
    
    /* Test __builtin_add_overflow with 128-bit values */
    x = ((__int128)MAX_64 << 63);  /* Large positive value */
    y = ((__int128)MAX_64 << 63);
    
    if (__builtin_add_overflow(x, y, &z)) {
        result += 1;  /* Overflow occurred */
    }
    
    /* Test multiplication near boundaries */
    x = ((__int128)0x7FFFFFFFFFFFFFFFLL);
    y = ((__int128)0x7FFFFFFFFFFFFFFFLL);
    
    if (__builtin_mul_overflow(x, y, &z)) {
        result += 2;
    }
    
    /* Test signed overflow in comparisons */
    x = ((__int128)1 << 126);  /* Near INT128_MAX */
    y = x + x;
    
    /* These comparisons should exercise the high-word path */
    result += (x > 0) ? 4 : 0;
    result += (y < 0) ? 8 : 0;  /* May overflow to negative */
    
    return result;
}

/* Test 4: Bitwise operations crossing 64-bit boundary */
__attribute__((noinline))
int test_bitwise_operations(void) {
    __int128 a, b;
    int result = 0;
    
    /* Create values that span both high and low words */
    a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    b = ((__int128)0x0FEDCBA987654321ULL << 64) | 0x0123456789ABCDEFULL;
    
    /* Bitwise operations */
    __int128 and_result = a & b;
    __int128 or_result = a | b;
    __int128 shift_result = a << 32;  /* Crosses 64-bit boundary */
    __int128 shift_right = a >> 96;   /* Moves high word to low word */
    
    /* Comparisons after bitwise operations */
    result += (and_result < or_result) ? 1 : 0;
    result += (shift_result > a) ? 2 : 0;
    result += (shift_right < b) ? 4 : 0;
    
    /* Test with unsigned __int128 */
    unsigned __int128 ua = (unsigned __int128)a;
    unsigned __int128 ub = (unsigned __int128)b;
    
    result += (ua < ub) ? 8 : 0;
    result += (ua > ub) ? 16 : 0;
    
    return result;
}

/* Test 5: Switch statement with __int128 case labels */
__attribute__((noinline))
int test_switch_statement(__int128 value) {
    int result = 0;
    
    /* Switch on __int128 - forces compiler to generate comparison tree */
    switch (value) {
        case ((__int128)0x1ULL << 64):
            result = 1;
            break;
        case ((__int128)0x2ULL << 64):
            result = 2;
            break;
        case ((__int128)0x3ULL << 64) | 0xFFFFFFFFULL:
            result = 3;
            break;
        case -((__int128)0x1ULL << 64):
            result = 4;
            break;
        default:
            /* Compare against boundary values */
            if (value < ((__int128)0x1ULL << 64)) {
                result = 5;
            } else if (value > ((__int128)0x3ULL << 64)) {
                result = 6;
            } else {
                result = 7;
            }
    }
    
    return result;
}

/* Test 6: Array operations with __int128 */
__attribute__((noinline))
int test_array_operations(void) {
    /* Array of __int128 values that exercise different comparison paths */
    __int128 arr[8] = {
        ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Low word max */
        ((__int128)0x1ULL << 64) | 0x0ULL,                 /* High word 1 */
        ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Both words */
        ((__int128)0x2ULL << 64) | 0x0ULL,                 /* High word 2 */
        -(((__int128)0x1ULL << 64) | 0x0ULL),              /* Negative */
        -(((__int128)0x2ULL << 64) | 0x0ULL),              /* More negative */
        ((__int128)MAX_64 << 64) | MAX_64,                 /* Large positive */
        -(((__int128)MAX_64 << 64) | MAX_64)               /* Large negative */
    };
    
    int result = 0;
    
    /* Compare all pairs to exercise comparison logic */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (arr[i] < arr[j]) result += 1;
            if (arr[i] > arr[j]) result += 2;
            if (arr[i] == arr[j]) result += 4;
            
            /* Mixed type comparison */
            if (arr[i] < (long long)arr[j]) result += 8;
            if (arr[i] > (unsigned long long)arr[j]) result += 16;
        }
    }
    
    return result & 0xFF;
}

/* Test 7: Ternary operator with mixed types */
__attribute__((noinline))
int test_ternary_operations(void) {
    int result = 0;
    __int128 a, b;
    
    /* Ternary with __int128 and narrower types */
    a = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    b = 0x123456789ABCDEF0ULL;  /* 64-bit value */
    
    /* Force conversions and comparisons */
    __int128 c = (a > b) ? a : b;  /* b promoted to __int128 */
    result += (c == a) ? 1 : 0;
    
    /* Ternary with unsigned __int128 */
    unsigned __int128 ua = (unsigned __int128)a;
    unsigned __int128 ub = (unsigned __int128)-1;  /* UINT128_MAX */
    
    unsigned __int128 uc = (ua < ub) ? ua : ub;
    result += (uc == ua) ? 2 : 0;
    
    /* Nested ternary with comparisons */
    __int128 d = (a < 0) ? -a : (a > ((__int128)MAX_64 << 64)) ? a >> 1 : a;
    result += (d > 0) ? 4 : 0;
    
    return result;
}

/* Test 8: Builtin functions with __int128 */
__attribute__((noinline))
int test_builtin_functions(void) {
    int result = 0;
    unsigned __int128 x;
    
    /* Create value for population count */
    x = ((unsigned __int128)0x5555555555555555ULL << 64) | 0x5555555555555555ULL;
    
    /* Use builtins that may trigger internal comparisons */
    int popcnt = __builtin_popcountll((unsigned long long)(x >> 64)) +
                 __builtin_popcountll((unsigned long long)x);
    result += popcnt & 0xF;
    
    /* Test __builtin_expect with __int128 comparison */
    __int128 a = ((__int128)0x1ULL << 64);
    __int128 b = ((__int128)0x2ULL << 64);
    
    if (__builtin_expect(a < b, 1)) {
        result += 16;
    }
    
    if (__builtin_expect(a > b, 0)) {
        result += 32;
    }
    
    /* Test clz on high word */
    if (x != 0) {
        int clz_high = __builtin_clzll((unsigned long long)(x >> 64));
        int clz_low = __builtin_clzll((unsigned long long)x);
        result += (clz_high < clz_low) ? 64 : 0;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total_result = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests to exercise different code paths */
    total_result += test_high_word_comparisons();
    total_result += test_range_analysis();
    total_result += test_overflow_checks();
    total_result += test_bitwise_operations();
    
    /* Test switch with different values */
    total_result += test_switch_statement(((__int128)0x2ULL << 64));
    total_result += test_switch_statement(((__int128)0x1ULL << 64) | 0x1234);
    total_result += test_switch_statement(-((__int128)0x1ULL << 64));
    
    total_result += test_array_operations();
    total_result += test_ternary_operations();
    total_result += test_builtin_functions();
    
    /* Final mixed-precision comparison to ensure no dead code elimination */
    volatile __int128 final_check = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    if (final_check > 0) {
        total_result += 1;
    }
    
    printf("Total checksum: %d\n", total_result);
    printf("Test completed. Compile with:\n");
    printf("  gcc -O3 -fstrict-overflow -Wstrict-overflow=5 test.c\n");
    printf("  gcc -O2 -fdump-tree-all -fdump-rtl-all test.c\n");
    
    return total_result != 0 ? 0 : 1;
}
