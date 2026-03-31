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
#define LARGE_CONST_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)
#define NEG_LARGE_CONST_128 ((__int128)0xFEDCBA9876543210ULL << 64 | 0x123456789ABCDEF0ULL)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)1 << 64) > 0, "128-bit shift must work");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit constant");

/* Function to create value ranges that span both high and low words */
__int128 create_range(int selector) {
    switch(selector) {
        case 0: return (__int128)0;
        case 1: return (__int128)1;
        case 2: return (__int128)-1;
        case 3: return (__int128)HIGH_BIT_64;  /* Exactly at 64-bit boundary */
        case 4: return (__int128)HIGH_BIT_64 - 1;
        case 5: return (__int128)HIGH_BIT_64 + 1;
        case 6: return (__int128)MAX_64;
        case 7: return (__int128)MAX_64 << 64;
        default: return (__int128)selector * ((__int128)1 << 96);
    }
}

/* Test comparisons where high words differ */
int test_high_word_comparisons(void) {
    volatile int checksum = 0;
    
    /* Comparisons with different high words (positive) */
    __int128 a1 = (__int128)0x1ULL << 64;      /* high = 1, low = 0 */
    __int128 b1 = (__int128)0x2ULL << 64;      /* high = 2, low = 0 */
    checksum += (a1 < b1) ? 1 : 0;            /* Should be true */
    checksum += (b1 > a1) ? 2 : 0;            /* Should be true */
    
    /* Comparisons with different high words (negative) */
    __int128 a2 = (__int128)(-1) * ((__int128)0x1ULL << 64);
    __int128 b2 = (__int128)(-2) * ((__int128)0x1ULL << 64);
    checksum += (a2 > b2) ? 4 : 0;            /* -1 > -2 (true) */
    checksum += (b2 < a2) ? 8 : 0;            /* -2 < -1 (true) */
    
    /* Mixed positive/negative high words */
    __int128 pos_high = (__int128)0x1ULL << 64;
    __int128 neg_high = (__int128)(-1) * ((__int128)0x1ULL << 64);
    checksum += (pos_high > neg_high) ? 16 : 0;  /* Positive > negative */
    
    return checksum;
}

/* Test comparisons where high words are equal but low words differ */
int test_low_word_comparisons(void) {
    volatile int checksum = 0;
    
    /* Same high word, different low words */
    __int128 base_high = (__int128)0x12345678ULL << 64;
    __int128 a1 = base_high | 0x1111111111111111ULL;
    __int128 b1 = base_high | 0x2222222222222222ULL;
    
    checksum += (a1 < b1) ? 1 : 0;
    checksum += (b1 > a1) ? 2 : 0;
    checksum += (a1 == a1) ? 4 : 0;
    
    /* Test with negative high word */
    __int128 neg_base = (__int128)(-0x12345678ULL) << 64;
    __int128 a2 = neg_base | 0x1111111111111111ULL;
    __int128 b2 = neg_base | 0x2222222222222222ULL;
    
    checksum += (a2 > b2) ? 8 : 0;  /* With negative high, smaller low is larger */
    checksum += (b2 < a2) ? 16 : 0;
    
    return checksum;
}

/* Test boundary values */
int test_boundary_comparisons(void) {
    volatile int checksum = 0;
    
    /* Get approximate boundaries for __int128 */
    __int128 max_pos = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 min_neg = ((__int128)0x8000000000000000ULL << 64);
    __int128 near_max = max_pos - 1;
    __int128 near_min = min_neg + 1;
    
    checksum += (max_pos > near_max) ? 1 : 0;
    checksum += (min_neg < near_min) ? 2 : 0;
    checksum += (max_pos > min_neg) ? 4 : 0;
    
    /* Test unsigned __int128 boundaries */
    unsigned __int128 u_max = ~(unsigned __int128)0;
    unsigned __int128 u_near_max = u_max - 1;
    checksum += (u_max > u_near_max) ? 8 : 0;
    checksum += (u_max > (unsigned __int128)0) ? 16 : 0;
    
    return checksum;
}

/* Test arithmetic with overflow checking */
int test_overflow_comparisons(void) {
    volatile int checksum = 0;
    
    __int128 large = (__int128)0x7FFFFFFFFFFFFFFFULL << 60;
    __int128 add_result;
    int overflow;
    
    /* Use builtins that may trigger double_int comparisons */
    overflow = __builtin_add_overflow(large, large, &add_result);
    checksum += overflow ? 1 : 0;
    
    __int128 small = (__int128)0x1ULL << 60;
    overflow = __builtin_add_overflow(small, small, &add_result);
    checksum += overflow ? 0 : 2;
    
    /* Multiplication overflow */
    __int128 mul_result;
    overflow = __builtin_mul_overflow(large, (__int128)2, &mul_result);
    checksum += overflow ? 4 : 0;
    
    return checksum;
}

/* Test mixed-precision operations */
int test_mixed_precision(void) {
    volatile int checksum = 0;
    
    __int128 wide_val = (__int128)0x123456789ABCDEF0ULL << 64;
    unsigned long long narrow = 0x123456789ABCDEF0ULL;
    
    /* Compare 128-bit with 64-bit */
    checksum += (wide_val > (__int128)narrow) ? 1 : 0;
    checksum += (wide_val == (wide_val & ~(__int128)narrow)) ? 0 : 2;
    
    /* Ternary with mixed types */
    __int128 result = (narrow > 100) ? wide_val : (__int128)narrow;
    checksum += (result == wide_val) ? 4 : 0;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 masked = wide_val & ((__int128)0xFFFFFFFFULL << 32);
    checksum += (masked != 0) ? 8 : 0;
    
    /* Shifts that move bits across the 64-bit boundary */
    __int128 shifted = (__int128)0x1ULL << 72;  /* Crosses 64-bit boundary */
    checksum += (shifted > (__int128)0xFFFFFFFFULL) ? 16 : 0;
    
    return checksum;
}

/* Test with arrays to give optimizer substantial work */
int test_array_comparisons(void) {
    volatile int checksum = 0;
    
    /* Array of __int128 values spanning different ranges */
    __int128 arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = create_range(i);
    }
    
    /* Perform various comparisons on array elements */
    for (int i = 0; i < 7; i++) {
        checksum += (arr[i] < arr[i+1]) ? (1 << i) : 0;
        checksum += (arr[i] != arr[i+1]) ? (1 << (i+4)) : 0;
    }
    
    /* Compare first and last (widest range) */
    checksum += (arr[0] < arr[7]) ? 256 : 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(arr[3] > arr[4], 1)) {
        checksum += 512;
    }
    
    return checksum;
}

/* Test compiler builtins that may use double_int */
int test_builtin_operations(void) {
    volatile int checksum = 0;
    
    unsigned __int128 uval = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                             0xFEDCBA9876543210ULL;
    
    /* Count leading zeros on high and low parts */
    unsigned long long high_part = (unsigned long long)(uval >> 64);
    unsigned long long low_part = (unsigned long long)uval;
    
    int clz_high = __builtin_clzll(high_part);
    int clz_low = __builtin_clzll(low_part);
    
    checksum += clz_high;
    checksum += clz_low * 2;
    
    /* Population count */
    int popcount = __builtin_popcountll(high_part) + __builtin_popcountll(low_part);
    checksum += popcount * 4;
    
    /* Byte swap simulation (manual) */
    unsigned __int128 swapped = 0;
    for (int i = 0; i < sizeof(uval); i++) {
        unsigned char byte = ((unsigned char *)&uval)[i];
        ((unsigned char *)&swapped)[sizeof(uval)-1-i] = byte;
    }
    checksum += (swapped != uval) ? 128 : 0;
    
    return checksum;
}

int main(void) {
    volatile int total_checksum = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    /* Execute all tests to trigger various comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_mixed_precision();
    total_checksum += test_array_comparisons();
    total_checksum += test_builtin_operations();
    
    /* Force evaluation of comparisons in constant expressions */
    #if ((__int128)0x8000000000000000ULL << 64) < 0
    total_checksum += 1024;
    #endif
    
    #if ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 0xFFFFFFFFFFFFFFFFULL
    total_checksum += 2048;
    #endif
    
    printf("Total checksum: %d\n", total_checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    /* Use results to prevent dead code elimination */
    if (total_checksum == 0) {
        printf("WARNING: All tests were eliminated!\n");
    }
    
    return total_checksum != 0 ? 0 : 1;
}
