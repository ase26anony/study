/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define MID_RANGE_64  0x7FFFFFFFFFFFFFFFULL

/* Signed 128-bit constants */
static const __int128 INT128_MAX = ((__int128)MAX_UINT64 << 64) | MAX_UINT64;
static const __int128 INT128_MIN = (~INT128_MAX);
static const __int128 HIGH_WORD_DIFF = ((__int128)1 << 64);
static const __int128 NEG_HIGH_WORD = ((__int128)(-1L) << 64);

/* Unsigned 128-bit constants */
static const unsigned __int128 UINT128_MAX = 
    ((unsigned __int128)MAX_UINT64 << 64) | MAX_UINT64;

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)-1 << 64) < 0, 
               "Negative shift should be negative");

/* Function to create value ranges that span both words */
static __int128 create_range(int selector) {
    switch(selector) {
        case 0: return 0;
        case 1: return HIGH_BIT_64;  /* Only low word has high bit */
        case 2: return HIGH_BIT_64 << 1;
        case 3: return HIGH_BIT_64 << 32;
        case 4: return HIGH_BIT_64 << 64;  /* High word has bit */
        case 5: return (HIGH_BIT_64 << 64) | HIGH_BIT_64;
        case 6: return INT128_MAX;
        case 7: return INT128_MIN;
        default: return selector;
    }
}

/* Test high-word comparisons (lines 1285-1288) */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Comparisons where high words differ (signed) */
    __int128 a1 = HIGH_BIT_64 << 64;      /* High word = 0x8000000000000000 */
    __int128 b1 = HIGH_BIT_64 << 63;      /* High word = 0x4000000000000000 */
    checksum += (a1 > b1) ? 1 : -1;       /* Should trigger high > high */
    checksum += (b1 < a1) ? 1 : -1;       /* Should trigger high < high */
    
    /* Comparisons where high words differ (unsigned interpretation) */
    unsigned __int128 ua1 = (unsigned __int128)a1;
    unsigned __int128 ub1 = (unsigned __int128)b1;
    checksum += (ua1 > ub1) ? 2 : -2;     /* Unsigned high word comparison */
    
    /* Negative high words */
    __int128 neg_a = NEG_HIGH_WORD;
    __int128 neg_b = NEG_HIGH_WORD | 1;
    checksum += (neg_a < neg_b) ? 3 : -3; /* Both negative, high words equal? */
    
    return checksum;
}

/* Test low-word comparisons (lines 1289-1293) */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    /* Same high word, different low words */
    __int128 a2 = HIGH_WORD_DIFF | 100;
    __int128 b2 = HIGH_WORD_DIFF | 200;
    checksum += (a2 < b2) ? 4 : -4;       /* Should trigger low < low */
    checksum += (b2 > a2) ? 4 : -4;       /* Should trigger low > low */
    
    /* Edge case: low word overflow in comparisons */
    __int128 max_low = HIGH_WORD_DIFF | MAX_UINT64;
    __int128 min_low = HIGH_WORD_DIFF | 0;
    checksum += (max_low > min_low) ? 5 : -5;
    
    /* Mixed signed/unsigned with same high word */
    unsigned __int128 ua2 = (unsigned __int128)a2;
    unsigned __int128 ub2 = (unsigned __int128)b2;
    checksum += (ua2 < ub2) ? 6 : -6;
    
    return checksum;
}

/* Test range analysis with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = INT128_MIN + 1000; i < INT128_MIN + 2000; i += 100) {
        /* Force range analysis on negative values */
        if (i > INT128_MIN + 1500) {
            checksum += 1;
        }
    }
    
    /* Loop crossing 64-bit boundary */
    for (unsigned __int128 j = UINT128_MAX - 1000; j < UINT128_MAX; j += 100) {
        /* This should analyze ranges in high word */
        if (j > UINT128_MAX - 500) {
            checksum += 2;
        }
    }
    
    return checksum;
}

/* Test overflow operations that require comparisons */
static int test_overflow_comparisons(void) {
    int checksum = 0;
    __int128 result;
    int overflow;
    
    /* Overflow addition near boundaries */
    __int128 near_max = INT128_MAX - 100;
    overflow = __builtin_add_overflow(near_max, 200, &result);
    checksum += overflow ? 7 : -7;  /* Should overflow */
    
    __int128 near_min = INT128_MIN + 100;
    overflow = __builtin_sub_overflow(near_min, 200, &result);
    checksum += overflow ? 8 : -8;  /* Should underflow */
    
    /* Multiplication that crosses word boundaries */
    __int128 large = HIGH_BIT_64;
    overflow = __builtin_mul_overflow(large, large, &result);
    checksum += overflow ? 9 : -9;  /* 2^63 * 2^63 = 2^126, fits in 128-bit */
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 wide_val = HIGH_WORD_DIFF | 500;
    long long narrow_val = 500;
    
    checksum += (wide_val > narrow_val) ? 10 : -10;
    checksum += (narrow_val < wide_val) ? 10 : -10;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (wide_val > 0) ? wide_val : (__int128)narrow_val;
    checksum += (ternary_result == wide_val) ? 11 : -11;
    
    /* Array indexing with __int128 calculations */
    __int128 array[8] = {0};
    for (int i = 0; i < 8; i++) {
        array[i] = create_range(i);
    }
    
    /* Comparisons between array elements */
    for (int i = 0; i < 7; i++) {
        checksum += (array[i] < array[i + 1]) ? 1 : 0;
    }
    
    return checksum;
}

/* Test bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 bits = 1;
    
    /* Shift across 64-bit boundary */
    __int128 shifted = bits << 65;  /* High word = 2, low word = 0 */
    __int128 mask = ((__int128)0x3 << 64) | 0x1;
    
    checksum += ((shifted & mask) == (bits << 65)) ? 12 : -12;
    
    /* Right shift of negative number */
    __int128 neg_shift = -1;
    __int128 right_shifted = neg_shift >> 65;
    checksum += (right_shifted == -1) ? 13 : -13;  /* Arithmetic shift */
    
    /* Byte swap simulation */
    unsigned __int128 to_swap = ((unsigned __int128)0x0123456789ABCDEFULL << 64) |
                                0xFEDCBA9876543210ULL;
    /* Manual byte swap */
    unsigned __int128 swapped = 0;
    for (int i = 0; i < 16; i++) {
        unsigned char byte = (to_swap >> (8 * i)) & 0xFF;
        swapped |= ((unsigned __int128)byte << (8 * (15 - i)));
    }
    checksum += (swapped != to_swap) ? 14 : -14;
    
    return checksum;
}

/* Test switch with __int128 cases (forces comparison tree) */
static int test_switch_comparisons(__int128 value) {
    switch (value) {
        case ((__int128)0ULL):
            return 1;
        case ((__int128)HIGH_BIT_64):
            return 2;
        case ((__int128)HIGH_BIT_64 << 64):
            return 3;
        case ((__int128)HIGH_BIT_64 << 64 | HIGH_BIT_64):
            return 4;
        case INT128_MAX:
            return 5;
        case INT128_MIN:
            return 6;
        default:
            return 0;
    }
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_range_analysis();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_mixed_precision();
    total_checksum += test_bitwise_operations();
    
    /* Test switch with various values */
    total_checksum += test_switch_comparisons(0);
    total_checksum += test_switch_comparisons(HIGH_BIT_64);
    total_checksum += test_switch_comparisons(HIGH_BIT_64 << 64);
    total_checksum += test_switch_comparisons(INT128_MAX);
    
    /* Use __builtin_expect with wide comparisons */
    __int128 x = HIGH_WORD_DIFF | 42;
    __int128 y = HIGH_WORD_DIFF | 100;
    if (__builtin_expect(x < y, 1)) {
        total_checksum += 20;
    }
    
    /* Force printf conversion (may trigger internal conversions) */
    printf("Intermediate checksum: %d\n", total_checksum);
    
    /* Final array operations to prevent dead code elimination */
    __int128 final_array[8];
    for (int i = 0; i < 8; i++) {
        final_array[i] = create_range(i);
        /* Compare array elements to force double_int comparisons */
        for (int j = 0; j < i; j++) {
            if (final_array[j] < final_array[i]) {
                total_checksum += 1;
            }
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
