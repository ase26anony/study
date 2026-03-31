/* test-double-int.c - Designed to trigger GCC's internal double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define HIGH_VAL_64   0xFFFFFFFF00000000ULL

/* Large 128-bit constants for comparison */
static const __int128 NEG_LARGE = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;  /* High negative, low negative */
static const __int128 POS_LARGE = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
static const __int128 MIXED_HIGH_POS = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0FEDCBA987654321ULL;
static const __int128 MIXED_HIGH_NEG = ((__int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)0x8000000000000000ULL << 64), 
               "Compile-time 128-bit comparison 1");

_Static_assert(((__int128)0x123456789ABCDEF0ULL << 64) > 
               ((__int128)0x123456789ABCDEEEULL << 64),
               "Compile-time 128-bit comparison 2");

/* Function to create value ranges that span both words */
static __int128 create_range_variant(int selector) {
    switch (selector) {
        case 0: return ((__int128)0x1ULL << 64) | 0x0ULL;          /* High=1, Low=0 */
        case 1: return ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL;   /* High=1, Low=max32 */
        case 2: return ((__int128)0x0ULL << 64) | HIGH_BIT_64;     /* High=0, Low with high bit */
        case 3: return ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;     /* Negative high, zero low */
        case 4: return ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;/* Both words negative */
        default: return 0;
    }
}

/* Test high-word comparisons (lines 1285-1288) */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Comparisons where high words differ */
    __int128 a1 = ((__int128)0x2ULL << 64) | 0x0ULL;
    __int128 b1 = ((__int128)0x1ULL << 64) | MAX_64;  /* b1 has smaller high word */
    
    if (a1 > b1) checksum += 1;  /* Should take: a.high > b.high */
    if (b1 < a1) checksum += 2;  /* Should take: b.high < a.high */
    
    /* Negative high words comparison */
    __int128 a2 = ((__int128)0xFFFFFFFFFFFFFFFEULL << 64) | 0x0ULL;  /* -2 in high */
    __int128 b2 = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | MAX_64;  /* -1 in high */
    
    if (a2 > b2) checksum += 4;  /* Unsigned comparison: 0xFE... > 0xFF... */
    if (b2 < a2) checksum += 8;  /* Opposite direction */
    
    /* Equal high words, different low words (lines 1289-1293) */
    __int128 a3 = ((__int128)0x1ULL << 64) | 0x2ULL;
    __int128 b3 = ((__int128)0x1ULL << 64) | 0x1ULL;
    
    if (a3 > b3) checksum += 16;  /* High equal, a.low > b.low */
    if (b3 < a3) checksum += 32;  /* High equal, b.low < a.low */
    
    return checksum;
}

/* Test with unsigned __int128 comparisons */
static int test_unsigned_comparisons(void) {
    unsigned __int128 ua, ub;
    int checksum = 0;
    
    /* High word differs */
    ua = ((unsigned __int128)0x2ULL << 64) | 0x0ULL;
    ub = ((unsigned __int128)0x1ULL << 64) | MAX_64;
    
    if (ua > ub) checksum += 1;
    if (ub < ua) checksum += 2;
    
    /* High word equal, low differs */
    ua = ((unsigned __int128)0x1ULL << 64) | 0xFFFFFFFF00000000ULL;
    ub = ((unsigned __int128)0x1ULL << 64) | 0x00000000FFFFFFFFULL;
    
    if (ua > ub) checksum += 4;
    if (ub < ua) checksum += 8;
    
    /* Boundary cases */
    unsigned __int128 max128 = ~((unsigned __int128)0);
    unsigned __int128 almost_max = max128 - 1;
    
    if (max128 > almost_max) checksum += 16;
    if (almost_max < max128) checksum += 32;
    
    return checksum;
}

/* Test range analysis with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundary */
    for (__int128 i = ((__int128)0xFFFFFFFFFFFFFFF0ULL << 64); 
         i < ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) + 16; 
         i++) {
        /* Force comparison in loop condition */
        if (i < 0) checksum += (int)(i & 0xFF);
    }
    
    /* Overflow checking with builtins */
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 y = 1;
    __int128 result;
    
    if (__builtin_add_overflow(x, y, &result)) {
        checksum += 1000;  /* Should overflow */
    }
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 a = ((__int128)0x1ULL << 64) | 0x0ULL;
    unsigned long long b = 0xFFFFFFFFFFFFFFFFULL;
    
    if (a > b) checksum += 1;  /* Should be true */
    if (b < a) checksum += 2;  /* Should be true */
    
    /* Ternary operator with mixed types */
    __int128 val1 = (sizeof(size_t) == 8) ? 
                   ((__int128)0x1ULL << 64) : 0x1ULL;
    __int128 val2 = (sizeof(long long) == 8) ? 
                   ((__int128)0x1ULL << 63) : 0x1ULL;
    
    if (val1 > val2) checksum += 4;
    
    /* Array operations with __int128 */
    __int128 arr[8] = {
        0,
        ((__int128)0x1ULL << 64),
        ((__int128)0x1ULL << 63),
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)HIGH_BIT_64 << 63),
        ((__int128)MAX_64 << 64) | MAX_64,
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64,
        ((__int128)0x8000000000000000ULL << 64) | 0x0ULL
    };
    
    for (int i = 0; i < 7; i++) {
        if (arr[i] < arr[i + 1]) checksum += (1 << (i + 3));
    }
    
    return checksum;
}

/* Test bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0FEDCBA987654321ULL;
    __int128 b = ((__int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL;
    
    /* Bitwise operations that affect both words */
    __int128 and_result = a & b;
    __int128 or_result = a | b;
    __int128 shift_result = a << 65;  /* Crosses word boundary */
    __int128 shift_right = a >> 65;
    
    /* Comparisons after bitwise ops */
    if (and_result < or_result) checksum += 1;
    if (shift_result > shift_right) checksum += 2;
    
    /* Use __builtin_expect with 128-bit comparison */
    if (__builtin_expect((a & 0xFFFFFFFFULL) == 0x87654321UL, 0)) {
        checksum += 4;
    }
    
    return checksum;
}

/* Test switch statement with __int128 case labels */
static int test_switch_int128(void) {
    __int128 value = ((__int128)0x1ULL << 64) | 0x0ULL;
    int result = 0;
    
    switch (value) {
        case ((__int128)0x0ULL << 64) | 0x0ULL:
            result = 1;
            break;
        case ((__int128)0x1ULL << 64) | 0x0ULL:
            result = 2;  /* This should match */
            break;
        case ((__int128)0x1ULL << 64) | 0x1ULL:
            result = 3;
            break;
        case ((__int128)0x2ULL << 64) | 0x0ULL:
            result = 4;
            break;
        default:
            result = 5;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests */
    total_checksum += test_high_word_comparisons();
    printf("High word comparisons: %d\n", total_checksum);
    
    total_checksum += test_unsigned_comparisons();
    printf("Unsigned comparisons: %d\n", total_checksum);
    
    total_checksum += test_range_analysis();
    printf("Range analysis: %d\n", total_checksum);
    
    total_checksum += test_mixed_precision();
    printf("Mixed precision: %d\n", total_checksum);
    
    total_checksum += test_bitwise_operations();
    printf("Bitwise operations: %d\n", total_checksum);
    
    total_checksum += test_switch_int128();
    printf("Switch with int128: %d\n", total_checksum);
    
    /* Force use of all test values to prevent dead code elimination */
    volatile __int128 force_use = NEG_LARGE + POS_LARGE + MIXED_HIGH_POS + MIXED_HIGH_NEG;
    (void)force_use;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Expected checksum with 64-bit long long: 1209\n");
    
    return total_checksum == 1209 ? 0 : 1;
}
