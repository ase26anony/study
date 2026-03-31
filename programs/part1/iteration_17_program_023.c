/* test-double-int-comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define MID_RANGE_64  0x7FFFFFFFFFFFFFFFULL

/* Large 128-bit constants for comparison */
static const __int128 SIGNED_MAX = ((__int128)MAX_UINT64 << 64) | MAX_UINT64;
static const __int128 SIGNED_MIN = ((__int128)HIGH_BIT_64 << 64);
static const unsigned __int128 UNSIGNED_MAX = ((unsigned __int128)MAX_UINT64 << 64) | MAX_UINT64;

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)0x1ULL << 64) > 0, "128-bit shift should work");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit constant");

/* Test 1: High word comparisons (signed) */
static int test_high_word_comparisons(void) {
    __int128 a = ((__int128)HIGH_BIT_64 << 64) | 0x1ULL;  /* Very negative */
    __int128 b = ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL; /* Positive */
    __int128 c = ((__int128)0x2ULL << 64) | 0x1ULL;       /* More positive */
    __int128 d = ((__int128)0x1ULL << 64) | 0x2ULL;       /* Same high, different low */
    
    int result = 0;
    
    /* These should trigger high word comparisons */
    if (a < b) result += 1;  /* High words differ (negative vs positive) */
    if (b < c) result += 2;  /* High words differ (0x1 vs 0x2) */
    if (d < b) result += 4;  /* High words equal, low words differ */
    if (c > b) result += 8;  /* Reverse comparison */
    
    /* Boundary cases */
    if (SIGNED_MIN < SIGNED_MAX) result += 16;
    if (0 > SIGNED_MIN) result += 32;
    
    return result;
}

/* Test 2: High word comparisons (unsigned) */
static int test_unsigned_high_word_comparisons(void) {
    unsigned __int128 a = ((unsigned __int128)0x1ULL << 64) | 0x1ULL;
    unsigned __int128 b = ((unsigned __int128)0x2ULL << 64) | 0xFFFFFFFFULL;
    unsigned __int128 c = ((unsigned __int128)0x2ULL << 64) | 0x1ULL;
    unsigned __int128 d = UNSIGNED_MAX;
    
    int result = 0;
    
    if (a < b) result += 1;   /* High words differ */
    if (b < d) result += 2;   /* High words differ */
    if (a < c) result += 4;   /* High words equal, low words differ */
    if (d > b) result += 8;   /* Reverse comparison */
    
    /* Test with mixed high/low patterns */
    unsigned __int128 e = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x1ULL;
    unsigned __int128 f = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_UINT64;
    
    if (e > f) result += 16;  /* High word with MSB set vs without */
    
    return result;
}

/* Test 3: Range analysis with loops */
static int test_range_analysis(void) {
    int result = 0;
    
    /* Loop with __int128 induction variable crossing 64-bit boundary */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32); 
         i < ((__int128)0x7FFFFFFFFFFFFFFFULL << 32) + 1000; 
         i++) {
        if (i < ((__int128)0x7FFFFFFFFFFFFFFFULL << 32) + 500) {
            result += (int)(i & 0xFF);
        }
    }
    
    /* Another loop with unsigned __int128 */
    unsigned __int128 start = ((unsigned __int128)MAX_UINT64 << 32);
    for (unsigned __int128 j = start; j < start + 1000; j++) {
        if (j > start + 500) {
            result -= (int)(j & 0xFF);
        }
    }
    
    return result & 0xFF;  /* Keep result small */
}

/* Test 4: Overflow operations requiring comparisons */
static int test_overflow_comparisons(void) {
    int result = 0;
    __int128 a, b;
    int overflow;
    
    /* Test __builtin_add_overflow with 128-bit values */
    a = ((__int128)MAX_UINT64 << 62);
    b = ((__int128)MAX_UINT64 << 62);
    overflow = __builtin_add_overflow(a, b, &a);
    if (overflow) result += 1;
    
    /* Test __builtin_mul_overflow */
    a = ((__int128)0x2ULL << 62);
    b = ((__int128)0x3ULL << 62);
    overflow = __builtin_mul_overflow(a, b, &a);
    if (overflow) result += 2;
    
    /* Test comparisons after overflow checks */
    __int128 max_val = ((__int128)MAX_UINT64 << 63) | MAX_UINT64;
    __int128 min_val = ((__int128)HIGH_BIT_64 << 63);
    
    if (a < max_val) result += 4;
    if (min_val < a) result += 8;
    
    return result;
}

/* Test 5: Bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    int result = 0;
    
    __int128 x = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 y = ((__int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    
    /* Operations that mix high and low words */
    __int128 and_result = x & y;
    __int128 or_result = x | y;
    __int128 shift_left = x << 65;  /* Crosses 64-bit boundary */
    __int128 shift_right = x >> 65; /* Crosses 64-bit boundary */
    
    /* Comparisons of bitwise operation results */
    if (and_result < or_result) result += 1;
    if (shift_left > shift_right) result += 2;
    if ((x & HIGH_BIT_64) != 0) result += 4;
    
    /* Test with unsigned */
    unsigned __int128 ux = (unsigned __int128)x;
    unsigned __int128 uy = (unsigned __int128)y;
    unsigned __int128 ushift = ux >> 65;
    
    if (ushift < ux) result += 8;
    
    return result;
}

/* Test 6: Mixed-precision comparisons */
static int test_mixed_precision(void) {
    int result = 0;
    
    __int128 a = ((__int128)0x1ULL << 64) | 0x1ULL;
    unsigned long long b = 0xFFFFFFFFFFFFFFFFULL;
    size_t c = SIZE_MAX;
    
    /* Compare __int128 with narrower types */
    if (a > b) result += 1;
    if (a < (__int128)c) result += 2;
    
    /* Ternary operator with mixed types */
    __int128 d = (b > 1000) ? a : (__int128)b;
    if (d == a) result += 4;
    
    /* Compare with zero in different forms */
    if (a > 0) result += 8;
    if (a > 0LL) result += 16;
    if (a > 0ULL) result += 32;
    
    return result;
}

/* Test 7: Compiler builtins with 128-bit values */
static int test_builtin_functions(void) {
    int result = 0;
    
    unsigned __int128 x = ((unsigned __int128)0x1ULL << 64) | 0x8000000000000000ULL;
    
    /* Use builtins that may trigger comparisons */
    if (__builtin_expect(x > ((unsigned __int128)0x1ULL << 63), 1)) {
        result += 1;
    }
    
    /* Test popcount on parts of 128-bit value */
    unsigned long long low = (unsigned long long)x;
    unsigned long long high = (unsigned long long)(x >> 64);
    
    if (__builtin_popcountll(low) > __builtin_popcountll(high)) {
        result += 2;
    }
    
    /* Manual byte swap that crosses 64-bit boundary */
    unsigned __int128 swapped = ((x & 0xFF) << 120) |
                               (((x >> 8) & 0xFF) << 112) |
                               (((x >> 16) & 0xFF) << 104) |
                               (((x >> 24) & 0xFF) << 96) |
                               (((x >> 32) & 0xFF) << 88) |
                               (((x >> 40) & 0xFF) << 80) |
                               (((x >> 48) & 0xFF) << 72) |
                               (((x >> 56) & 0xFF) << 64) |
                               (((x >> 64) & 0xFF) << 56) |
                               (((x >> 72) & 0xFF) << 48) |
                               (((x >> 80) & 0xFF) << 40) |
                               (((x >> 88) & 0xFF) << 32) |
                               (((x >> 96) & 0xFF) << 24) |
                               (((x >> 104) & 0xFF) << 16) |
                               (((x >> 112) & 0xFF) << 8) |
                               ((x >> 120) & 0xFF);
    
    if (swapped != x) result += 4;
    
    return result;
}

/* Test 8: Array operations with 128-bit values */
static int test_array_operations(void) {
    /* Array of __int128 values that exercise different comparison paths */
    __int128 arr[8] = {
        SIGNED_MIN,
        ((__int128)HIGH_BIT_64 << 63) | 0x1ULL,
        ((__int128)0x1ULL << 63),
        ((__int128)0x1ULL << 64) | 0x1ULL,
        ((__int128)0x2ULL << 64) | MAX_UINT64,
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_UINT64,
        SIGNED_MAX - 1,
        SIGNED_MAX
    };
    
    int result = 0;
    
    /* Compare each element with its neighbors */
    for (int i = 0; i < 7; i++) {
        if (arr[i] < arr[i + 1]) result += (1 << i);
        if (arr[i] > arr[7 - i]) result += (1 << (i + 8));
    }
    
    /* Find min and max in array */
    __int128 min_val = arr[0];
    __int128 max_val = arr[0];
    for (int i = 1; i < 8; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
        result += (int)((arr[i] >> 64) & 0xFF);  /* Use high word */
    }
    
    if (min_val == SIGNED_MIN) result += 256;
    if (max_val == SIGNED_MAX) result += 512;
    
    return result & 0x3FF;  /* Keep within 10 bits */
}

/* Test 9: Switch statement with __int128 case labels */
static int test_switch_statement(int input) {
    __int128 key = ((__int128)input << 64) | input;
    int result = 0;
    
    /* Switch that forces compiler to generate comparison tree */
    switch ((unsigned long long)(key >> 64)) {
        case 0:
            if (key < 0) result = 1;
            else result = 2;
            break;
        case 1:
            result = 3;
            break;
        case 0x8000000000000000ULL:
            result = 4;
            break;
        default:
            if (key > ((__int128)0x1ULL << 65)) result = 5;
            else result = 6;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    checksum += test_high_word_comparisons();
    printf("Test 1 result: %d\n", test_high_word_comparisons());
    
    checksum += test_unsigned_high_word_comparisons();
    printf("Test 2 result: %d\n", test_unsigned_high_word_comparisons());
    
    checksum += test_range_analysis();
    printf("Test 3 result: %d\n", test_range_analysis());
    
    checksum += test_overflow_comparisons();
    printf("Test 4 result: %d\n", test_overflow_comparisons());
    
    checksum += test_bitwise_operations();
    printf("Test 5 result: %d\n", test_bitwise_operations());
    
    checksum += test_mixed_precision();
    printf("Test 6 result: %d\n", test_mixed_precision());
    
    checksum += test_builtin_functions();
    printf("Test 7 result: %d\n", test_builtin_functions());
    
    checksum += test_array_operations();
    printf("Test 8 result: %d\n", test_array_operations());
    
    checksum += test_switch_statement(42);
    printf("Test 9 result: %d\n", test_switch_statement(42));
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
