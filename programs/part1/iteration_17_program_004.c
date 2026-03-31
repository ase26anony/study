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
#define MID_64        0x7FFFFFFFFFFFFFFFULL

/* Large 128-bit constants for constant folding */
static const __int128 SIGNED_MAX = ((__int128)MAX_64 << 64) | MAX_64;
static const __int128 SIGNED_MIN = ((__int128)HIGH_BIT_64 << 64);
static const unsigned __int128 UNSIGNED_MAX = ((unsigned __int128)MAX_64 << 64) | MAX_64;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x1 << 120) > (((__int128)0x1 << 119) * 2 - 1), 
               "128-bit constant folding comparison 1");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, 
               "128-bit negative constant detection");
_Static_assert(((unsigned __int128)MAX_64 << 64) > MAX_64, 
               "128-bit unsigned high-word comparison");

/* Function to create value ranges that span both words */
static __int128 create_range(__int128 base, int offset) {
    /* Operations that might trigger range analysis on both high/low words */
    __int128 result = base;
    
    /* Force potential overflow in high word */
    if (offset > 0) {
        result += ((__int128)offset << 32);
    } else {
        result -= ((__int128)(-offset) << 32);
    }
    
    /* Bitwise operations crossing 64-bit boundary */
    result = (result & ~((__int128)0xFF << 72)) | ((__int128)0xAA << 72);
    
    return result;
}

/* Function using builtins that may trigger double_int comparisons */
static int compare_with_builtins(__int128 a, __int128 b) {
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(a < b, 0)) {
        return -1;
    }
    if (__builtin_expect(a > b, 1)) {
        return 1;
    }
    
    /* Overflow checking that requires wide comparisons */
    __int128 sum;
    if (__builtin_add_overflow(a, b, &sum)) {
        /* Overflow occurred - compare overflow boundaries */
        if (a > 0 && b > 0) {
            return (SIGNED_MAX - a) < b ? -1 : 1;
        }
    }
    
    return 0;
}

/* Test case 1: High word differs, low word equal */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Case 1a: Positive values, high word differs */
    __int128 a1 = ((__int128)0x1 << 64) | 0x123456789ABCDEF0ULL;
    __int128 b1 = ((__int128)0x2 << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should be true */
    checksum += (a1 > b1) ? 2 : 0;  /* Should be false */
    
    /* Case 1b: Negative values, high word differs */
    __int128 a2 = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    __int128 b2 = ((__int128)(HIGH_BIT_64 | 0x1) << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a2 < b2) ? 4 : 0;  /* Should be true (more negative < less negative) */
    checksum += (a2 > b2) ? 8 : 0;  /* Should be false */
    
    /* Case 1c: Mixed signs */
    __int128 a3 = ((__int128)0x1 << 64) | 0x123456789ABCDEF0ULL;  /* Positive */
    __int128 b3 = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;  /* Negative */
    checksum += (a3 < b3) ? 16 : 0;  /* Should be false (positive > negative) */
    checksum += (a3 > b3) ? 32 : 0;  /* Should be true */
    
    return checksum;
}

/* Test case 2: High word equal, low word differs */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    /* Same high word, different low words */
    __int128 base_high = ((__int128)0x12345678 << 64);
    
    __int128 a1 = base_high | 0x0000000000000001ULL;
    __int128 b1 = base_high | 0xFFFFFFFFFFFFFFFFULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should be true */
    checksum += (a1 > b1) ? 2 : 0;  /* Should be false */
    
    /* With negative high word */
    __int128 neg_high = ((__int128)HIGH_BIT_64 << 64);
    
    __int128 a2 = neg_high | 0x0000000000000001ULL;
    __int128 b2 = neg_high | 0xFFFFFFFFFFFFFFFFULL;
    checksum += (a2 < b2) ? 4 : 0;  /* Should be true (more negative < less negative) */
    checksum += (a2 > b2) ? 8 : 0;  /* Should be false */
    
    return checksum;
}

/* Test case 3: Boundary value comparisons */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Signed boundaries */
    checksum += (SIGNED_MIN < SIGNED_MAX) ? 1 : 0;
    checksum += (SIGNED_MIN > 0) ? 2 : 0;
    checksum += (SIGNED_MAX > 0) ? 4 : 0;
    
    /* Near overflow boundaries */
    __int128 near_max = SIGNED_MAX - 1000;
    __int128 at_max = SIGNED_MAX;
    checksum += (near_max < at_max) ? 8 : 0;
    checksum += (near_max > at_max) ? 16 : 0;
    
    /* Unsigned comparisons */
    unsigned __int128 u1 = ((unsigned __int128)0x1 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 u2 = ((unsigned __int128)0x2 << 64) | 0x0000000000000000ULL;
    checksum += (u1 < u2) ? 32 : 0;
    checksum += (u1 > u2) ? 64 : 0;
    
    return checksum;
}

/* Test case 4: Mixed precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 a = ((__int128)0x1 << 64) | 0x123456789ABCDEF0ULL;
    long long b = 0x123456789ABCDEF0LL;
    
    /* These should trigger conversions and comparisons */
    checksum += (a > b) ? 1 : 0;
    checksum += (a < (__int128)b) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 result = (a > 0) ? a : (__int128)b;
    checksum += (result == a) ? 4 : 0;
    
    /* Compare with size_t (architecture dependent) */
    size_t s = SIZE_MAX;
    __int128 c = s;
    checksum += (c == (__int128)s) ? 8 : 0;
    checksum += (c < ((__int128)s + 1)) ? 16 : 0;
    
    return checksum;
}

/* Test case 5: Array operations for optimizer */
static int test_array_operations(void) {
    int checksum = 0;
    
    /* Array of __int128 values spanning different high/low combinations */
    __int128 arr[8] = {
        SIGNED_MIN,
        SIGNED_MIN + 1,
        -((__int128)0x1 << 64),
        0,
        ((__int128)0x1 << 64) - 1,
        ((__int128)0x1 << 64),
        SIGNED_MAX - 1,
        SIGNED_MAX
    };
    
    /* Perform comparisons that exercise all combinations */
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (arr[i] < arr[j]) {
                checksum += 1;
            }
            if (arr[i] > arr[j]) {
                checksum += 2;
            }
            if (arr[i] == arr[j]) {
                checksum += 4;
            }
        }
    }
    
    /* Loop with __int128 induction variable */
    for (__int128 i = SIGNED_MIN + 1000; i < SIGNED_MIN + 1100; i++) {
        checksum += (i < 0) ? 1 : 0;
    }
    
    return checksum;
}

/* Test case 6: Bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    /* Shift operations that affect high word */
    __int128 left_shift = a << 4;
    __int128 right_shift = a >> 4;
    
    checksum += (left_shift > a) ? 1 : 0;
    checksum += (right_shift < a) ? 2 : 0;
    
    /* Bitwise AND/OR crossing boundary */
    __int128 mask = ((__int128)0xFFFF << 48) | 0xFFFF;
    __int128 masked = a & mask;
    checksum += (masked < a) ? 4 : 0;
    
    /* Cross-word bit manipulation */
    __int128 swapped = ((a & 0xFFFFFFFF00000000ULL) << 32) | 
                      ((a >> 32) & 0xFFFFFFFF00000000ULL) |
                      (a & 0x00000000FFFFFFFFULL);
    checksum += (swapped != a) ? 8 : 0;
    
    return checksum;
}

/* Test case 7: Overflow checking with builtins */
static int test_overflow_checks(void) {
    int checksum = 0;
    
    /* Test addition overflow */
    __int128 x = SIGNED_MAX - 100;
    __int128 y = 200;
    __int128 sum;
    
    if (__builtin_add_overflow(x, y, &sum)) {
        checksum += 1;  /* Should trigger */
    }
    
    /* Test multiplication overflow */
    __int128 a = ((__int128)0x1 << 62);
    __int128 b = 8;
    __int128 prod;
    
    if (__builtin_mul_overflow(a, b, &prod)) {
        checksum += 2;  /* Should trigger */
    }
    
    /* Compare overflow boundaries */
    __int128 safe_max = SIGNED_MAX / 2;
    checksum += (safe_max * 2 < SIGNED_MAX) ? 4 : 0;
    checksum += (safe_max * 3 > SIGNED_MAX) ? 8 : 0;
    
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all test cases */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_mixed_precision();
    total_checksum += test_array_operations();
    total_checksum += test_bitwise_operations();
    total_checksum += test_overflow_checks();
    
    /* Additional compile-time forced comparisons */
#if ((__int128)0x8000000000000000ULL << 64) < 0
    total_checksum += 0x100;
#endif
    
#if ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 0xFFFFFFFFFFFFFFFFULL
    total_checksum += 0x200;
#endif
    
    /* Switch statement with __int128 cases (compile-time constants) */
    __int128 switch_val = ((__int128)0x1 << 64);
    switch (switch_val) {
        case ((__int128)0x1 << 64):
            total_checksum += 0x400;
            break;
        case ((__int128)0x2 << 64):
            total_checksum += 0x800;
            break;
        default:
            total_checksum += 0x1000;
    }
    
    printf("Total checksum: 0x%08X\n", total_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
