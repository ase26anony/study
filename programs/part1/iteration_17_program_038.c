/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross the 64-bit boundary */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_64        0x7FFFFFFFFFFFFFFFULL

/* Signed 128-bit constants */
static const __int128 INT128_MAX = ((__int128)MAX_64 << 64) | MAX_64;
static const __int128 INT128_MIN = ((__int128)HIGH_BIT_64 << 64);
static const __int128 MID_POS_128 = ((__int128)MID_64 << 64) | MID_64;
static const __int128 MID_NEG_128 = ((__int128)HIGH_BIT_64 << 64) | MID_64;

/* Unsigned 128-bit constants */
static const unsigned __int128 UINT128_MAX = 
    ((unsigned __int128)MAX_64 << 64) | MAX_64;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x1 << 64) > 0, "High word positive comparison");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "High word negative comparison");
_Static_assert(((__int128)0x1 << 64) != 0, "High word non-zero comparison");

/* Function to create value ranges that span both high and low words */
static __int128 create_range_value(int selector) {
    switch (selector) {
        case 0: return 0;
        case 1: return ((__int128)0x1 << 64);  /* Only high word set */
        case 2: return ((__int128)0x1 << 64) | 0x1;  /* Both words set */
        case 3: return ((__int128)HIGH_BIT_64 << 64);  /* Negative, high word only */
        case 4: return ((__int128)HIGH_BIT_64 << 64) | 0x1;  /* Negative, both words */
        case 5: return MID_POS_128;
        case 6: return MID_NEG_128;
        default: return INT128_MAX;
    }
}

/* Test high word comparisons (both positive and negative) */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Comparisons where only high words differ */
    __int128 a1 = ((__int128)0x1 << 64);
    __int128 b1 = ((__int128)0x2 << 64);
    checksum += (a1 < b1) ? 1 : 0;  /* Should trigger high word comparison */
    checksum += (b1 > a1) ? 2 : 0;
    
    /* Negative high word comparisons */
    __int128 a2 = ((__int128)HIGH_BIT_64 << 64);
    __int128 b2 = ((__int128)(HIGH_BIT_64 | 0x1) << 64);
    checksum += (a2 < b2) ? 4 : 0;  /* Both negative, different high words */
    checksum += (b2 > a2) ? 8 : 0;
    
    /* Mixed sign high word comparisons */
    __int128 pos_high = ((__int128)0x1 << 64);
    __int128 neg_high = ((__int128)HIGH_BIT_64 << 64);
    checksum += (pos_high > neg_high) ? 16 : 0;
    checksum += (neg_high < pos_high) ? 32 : 0;
    
    return checksum;
}

/* Test low word comparisons when high words are equal */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    /* Same high word, different low words */
    __int128 base_high = ((__int128)0x12345678 << 64);
    __int128 a1 = base_high | 0x1;
    __int128 b1 = base_high | 0x2;
    checksum += (a1 < b1) ? 1 : 0;  /* Should trigger low word comparison */
    checksum += (b1 > a1) ? 2 : 0;
    
    /* Same negative high word, different low words */
    __int128 neg_high = ((__int128)HIGH_BIT_64 << 64);
    __int128 a2 = neg_high | 0x1;
    __int128 b2 = neg_high | 0x2;
    checksum += (a2 < b2) ? 4 : 0;
    checksum += (b2 > a2) ? 8 : 0;
    
    /* Edge case: low word overflow in comparison */
    __int128 a3 = base_high | MAX_64;
    __int128 b3 = base_high;
    checksum += (a3 > b3) ? 16 : 0;
    checksum += (b3 < a3) ? 32 : 0;
    
    return checksum;
}

/* Test boundary values */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* MAX vs MIN */
    checksum += (INT128_MAX > INT128_MIN) ? 1 : 0;
    checksum += (INT128_MIN < INT128_MAX) ? 2 : 0;
    
    /* MAX vs near-MAX */
    __int128 near_max = INT128_MAX - 1;
    checksum += (INT128_MAX > near_max) ? 4 : 0;
    checksum += (near_max < INT128_MAX) ? 8 : 0;
    
    /* MIN vs near-MIN */
    __int128 near_min = INT128_MIN + 1;
    checksum += (INT128_MIN < near_min) ? 16 : 0;
    checksum += (near_min > INT128_MIN) ? 32 : 0;
    
    /* Unsigned boundary comparisons */
    unsigned __int128 u_max = UINT128_MAX;
    unsigned __int128 u_near_max = UINT128_MAX - 1;
    checksum += (u_max > u_near_max) ? 64 : 0;
    checksum += (u_near_max < u_max) ? 128 : 0;
    
    return checksum;
}

/* Test arithmetic with overflow checking */
static int test_overflow_comparisons(void) {
    int checksum = 0;
    
    __int128 a = INT128_MAX / 2;
    __int128 b = INT128_MAX / 2 + 1;
    
    /* These may trigger overflow analysis comparisons */
    __int128 sum;
    if (__builtin_add_overflow(a, b, &sum)) {
        checksum += 1;  /* Overflow occurred */
    }
    
    __int128 product;
    if (__builtin_mul_overflow(a, b, &product)) {
        checksum += 2;  /* Overflow occurred */
    }
    
    /* Test comparisons after arithmetic */
    __int128 x = INT128_MAX - 100;
    __int128 y = INT128_MAX - 50;
    __int128 diff = y - x;
    checksum += (diff > 0) ? 4 : 0;
    checksum += (diff < 100) ? 8 : 0;
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 a = ((__int128)0x1 << 64);
    unsigned long long b = 0x1;
    checksum += (a > b) ? 1 : 0;
    checksum += (b < a) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    long long selector = 1;
    __int128 result = selector ? ((__int128)0x1 << 64) : (__int128)100;
    checksum += (result > 0) ? 4 : 0;
    
    /* Array indexing with __int128 */
    __int128 arr[8] = {
        0,
        ((__int128)0x1 << 64),
        ((__int128)0x2 << 64),
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)(HIGH_BIT_64 | 0x1) << 64),
        INT128_MAX,
        INT128_MIN,
        MID_POS_128
    };
    
    /* Force comparisons between array elements */
    for (int i = 0; i < 7; i++) {
        checksum += (arr[i] < arr[i + 1]) ? (1 << (i + 3)) : 0;
        checksum += (arr[i + 1] > arr[i]) ? (1 << (i + 10)) : 0;
    }
    
    return checksum;
}

/* Test bitwise operations that cross 64-bit boundary */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 a = ((__int128)0x12345678 << 64) | 0x9ABCDEF0;
    __int128 b = ((__int128)0xFEDCBA98 << 64) | 0x76543210;
    
    /* Bitwise operations */
    __int128 and_result = a & b;
    __int128 or_result = a | b;
    __int128 xor_result = a ^ b;
    
    /* Comparisons after bitwise ops */
    checksum += (and_result < or_result) ? 1 : 0;
    checksum += (xor_result > and_result) ? 2 : 0;
    
    /* Shifts that cross word boundary */
    __int128 left_shift = a << 65;  /* Crosses 64-bit boundary */
    __int128 right_shift = b >> 65;
    
    checksum += (left_shift > right_shift) ? 4 : 0;
    checksum += (right_shift < left_shift) ? 8 : 0;
    
    /* Use builtins that may trigger comparisons */
    unsigned __int128 ua = (unsigned __int128)a;
    int popcount = __builtin_popcountll((unsigned long long)(ua >> 64)) +
                   __builtin_popcountll((unsigned long long)ua);
    checksum += (popcount > 0) ? 16 : 0;
    
    return checksum;
}

/* Test loops with __int128 induction variables */
static int test_loop_comparisons(void) {
    int checksum = 0;
    
    /* Loop near 64-bit boundary */
    for (__int128 i = ((__int128)MAX_64 - 10); i < ((__int128)MAX_64 + 10); i++) {
        checksum += (i > 0) ? 1 : 0;
        checksum += (i < ((__int128)MAX_64 << 1)) ? 2 : 0;
    }
    
    /* Reverse loop with negative values */
    for (__int128 i = -10; i <= 10; i++) {
        __int128 val = ((__int128)HIGH_BIT_64 << 64) + i;
        checksum += (val < 0) ? 4 : 0;
        checksum += (val > ((__int128)HIGH_BIT_64 << 64) - 100) ? 8 : 0;
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing double_int comparison logic...\n");
    
    /* Run all test suites */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_mixed_precision();
    total_checksum += test_bitwise_operations();
    total_checksum += test_loop_comparisons();
    
    /* Force evaluation of switch with __int128 cases */
    for (int i = 0; i < 8; i++) {
        __int128 val = create_range_value(i);
        total_checksum += (val >= 0) ? i : -i;
    }
    
    /* Use __builtin_expect with __int128 comparisons */
    __int128 x = INT128_MAX;
    __int128 y = INT128_MIN;
    if (__builtin_expect(x > y, 1)) {
        total_checksum += 1024;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
