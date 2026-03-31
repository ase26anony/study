/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * for 128-bit integer operations during constant folding and optimization.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)HIGH_BIT_64 << 64),
               "Comparison with high words differing");

/* Function to create value ranges that span both high and low words */
static __int128 create_range(int selector) {
    switch (selector) {
        case 0: return ((__int128)HIGH_BIT_64 << 64) | MID_128;
        case 1: return ((__int128)HIGH_BIT_64 << 64) | (MID_128 + 1);
        case 2: return ((__int128)(HIGH_BIT_64 - 1) << 64) | MAX_64;
        case 3: return -(((__int128)HIGH_BIT_64 << 64) | MID_128);
        case 4: return (__int128)MAX_64 * MAX_64;
        case 5: return (__int128)MAX_64 * MAX_64 + 1;
        default: return 0;
    }
}

/* Test comparisons where high words differ */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Positive values with different high words */
    __int128 a1 = ((__int128)0x1000000000000000ULL << 64) | 0x1ULL;
    __int128 b1 = ((__int128)0x2000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    checksum += (a1 < b1) ? 1 : 0;  /* High word a < high word b */
    checksum += (b1 > a1) ? 2 : 0;
    
    /* Negative values with different high words */
    __int128 a2 = -a1;
    __int128 b2 = -b1;
    checksum += (a2 > b2) ? 4 : 0;  /* Negative comparison */
    checksum += (b2 < a2) ? 8 : 0;
    
    /* Mixed signs */
    checksum += (a1 > b2) ? 16 : 0;
    checksum += (b1 < a2) ? 32 : 0;
    
    return checksum;
}

/* Test comparisons where high words are equal but low words differ */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    __int128 base_high = ((__int128)0x3000000000000000ULL << 64);
    __int128 a1 = base_high | 0x1ULL;
    __int128 b1 = base_high | 0x2ULL;
    __int128 c1 = base_high | 0xFFFFFFFFFFFFFFFFULL;
    
    checksum += (a1 < b1) ? 1 : 0;  /* Low word a < low word b */
    checksum += (b1 > a1) ? 2 : 0;
    checksum += (a1 < c1) ? 4 : 0;
    checksum += (c1 > b1) ? 8 : 0;
    
    /* Same with negative base */
    __int128 neg_base = -base_high;
    __int128 a2 = neg_base | 0x1ULL;
    __int128 b2 = neg_base | 0x2ULL;
    
    checksum += (a2 > b2) ? 16 : 0;  /* Negative: smaller magnitude is larger */
    checksum += (b2 < a2) ? 32 : 0;
    
    return checksum;
}

/* Test boundary values */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Maximum and minimum signed 128-bit values */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = ((__int128)HIGH_BIT_64 << 64);
    
    checksum += (max_signed > min_signed) ? 1 : 0;
    checksum += (min_signed < 0) ? 2 : 0;
    checksum += (max_signed > 0) ? 4 : 0;
    
    /* Unsigned comparisons */
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 mid_unsigned = ((unsigned __int128)0x8000000000000000ULL << 64);
    
    checksum += (max_unsigned > mid_unsigned) ? 8 : 0;
    checksum += (mid_unsigned < max_unsigned) ? 16 : 0;
    
    /* Cross-type comparisons */
    checksum += ((unsigned __int128)max_signed < max_unsigned) ? 32 : 0;
    
    return checksum;
}

/* Test overflow operations that require wide comparisons */
static int test_overflow_comparisons(void) {
    int checksum = 0;
    
    __int128 large = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64);
    __int128 one = 1;
    
    /* Overflow checking */
    __int128 overflow_result;
    int overflow_flag = __builtin_add_overflow(large, large, &overflow_result);
    checksum += overflow_flag ? 1 : 0;
    
    /* Multiplication overflow */
    __int128 mul_result;
    overflow_flag = __builtin_mul_overflow(large, one, &mul_result);
    checksum += overflow_flag ? 2 : 0;
    
    /* Compare after potential overflow */
    __int128 sum = large + one;
    checksum += (sum > large) ? 4 : 0;
    checksum += (sum < 0) ? 8 : 0;
    
    return checksum;
}

/* Test bitwise operations crossing 64-bit boundary */
static int test_bitwise_comparisons(void) {
    int checksum = 0;
    
    __int128 mask_low = 0xFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFULL << 64);
    __int128 value = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    /* Bitwise operations */
    __int128 masked_low = value & mask_low;
    __int128 masked_high = value & mask_high;
    __int128 shifted = value << 32;
    __int128 shifted_right = value >> 32;
    
    checksum += (masked_low < masked_high) ? 1 : 0;
    checksum += (shifted > value) ? 2 : 0;
    checksum += (shifted_right < value) ? 4 : 0;
    
    /* Cross-boundary shifts */
    __int128 cross_shift = (value << 64) | (value >> 64);
    checksum += (cross_shift != value) ? 8 : 0;
    
    return checksum;
}

/* Test range analysis with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -5; i <= 5; i++) {
        __int128 scaled = i * ((__int128)0x1000000000000000ULL);
        checksum += (scaled >= -((__int128)0x5000000000000000ULL) && 
                    scaled <= ((__int128)0x5000000000000000ULL)) ? 1 : 0;
    }
    
    /* Array operations to give optimizer work */
    __int128 arr[8] = {
        ((__int128)0x1ULL << 64),
        ((__int128)0x2ULL << 64),
        ((__int128)0x3ULL << 64),
        ((__int128)0x4ULL << 64),
        ((__int128)0x5ULL << 64),
        ((__int128)0x6ULL << 64),
        ((__int128)0x7ULL << 64),
        ((__int128)0x8ULL << 64)
    };
    
    for (int i = 0; i < 7; i++) {
        checksum += (arr[i] < arr[i + 1]) ? 2 : 0;
    }
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    __int128 wide_val = ((__int128)0x1000000000000000ULL << 64);
    long long narrow_val = 0x7FFFFFFFFFFFFFFFLL;
    unsigned long long ul_narrow = 0xFFFFFFFFFFFFFFFFULL;
    size_t size_val = (size_t)-1;
    
    /* Mixed comparisons */
    checksum += (wide_val > narrow_val) ? 1 : 0;
    checksum += (wide_val > ul_narrow) ? 2 : 0;
    checksum += ((unsigned __int128)wide_val > size_val) ? 4 : 0;
    
    /* Ternary with mixed types */
    __int128 ternary_result = (narrow_val > 0) ? wide_val : (__int128)narrow_val;
    checksum += (ternary_result == wide_val) ? 8 : 0;
    
    /* Implicit conversions in arithmetic */
    __int128 mixed_sum = wide_val + narrow_val + ul_narrow + size_val;
    checksum += (mixed_sum > wide_val) ? 16 : 0;
    
    return checksum;
}

/* Test compiler builtins with wide integers */
static int test_builtin_comparisons(void) {
    int checksum = 0;
    
    unsigned __int128 value = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                              0xFEDCBA9876543210ULL;
    
    /* Use builtins that may trigger internal comparisons */
    int leading_zero = __builtin_clzll((unsigned long long)(value >> 64));
    int trailing_zero = __builtin_ctzll((unsigned long long)value);
    int popcount = __builtin_popcountll((unsigned long long)(value >> 64)) +
                   __builtin_popcountll((unsigned long long)value);
    
    checksum += (leading_zero < 64) ? 1 : 0;
    checksum += (trailing_zero >= 0) ? 2 : 0;
    checksum += (popcount > 0) ? 4 : 0;
    
    /* __builtin_expect with wide comparison */
    if (__builtin_expect(value > ((unsigned __int128)0x8000000000000000ULL << 64), 1)) {
        checksum += 8;
    }
    
    return checksum;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_bitwise_comparisons();
    total_checksum += test_range_analysis();
    total_checksum += test_mixed_precision();
    total_checksum += test_builtin_comparisons();
    
    /* Force use of all created values to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        __int128 val = create_range(i);
        total_checksum += (val != 0) ? 1 : 0;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
