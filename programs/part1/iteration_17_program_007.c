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
#define NEG_LARGE_CONST_128 (-LARGE_CONST_128)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFLL << 64) > 0, 
               "High word positive comparison needed");
_Static_assert(((__int128)0x8000000000000000LL << 64) < 0,
               "High word negative comparison needed");

/* Function to create value ranges that span both words */
static __int128 create_range_boundary(int selector) {
    switch (selector) {
        case 0: return (__int128)0x1ULL << 63;  /* Only high bit in low word */
        case 1: return (__int128)0x1ULL << 64;  /* Bit in high word only */
        case 2: return (__int128)0xFFFFFFFFULL << 32;  /* Crosses word boundary */
        case 3: return -((__int128)0x1ULL << 63);
        case 4: return -((__int128)0x1ULL << 64);
        default: return 0;
    }
}

/* Test high word comparisons (both positive and negative) */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Test 1: High words differ, low words equal */
    __int128 a1 = (__int128)0x1ULL << 64;
    __int128 b1 = (__int128)0x2ULL << 64;
    checksum += (a1 < b1) ? 1 : 0;  /* Should trigger high word comparison */
    checksum += (b1 > a1) ? 2 : 0;
    
    /* Test 2: High words differ (negative range) */
    __int128 a2 = -((__int128)0x2ULL << 64);
    __int128 b2 = -((__int128)0x1ULL << 64);
    checksum += (a2 < b2) ? 4 : 0;  /* Negative comparison */
    checksum += (b2 > a2) ? 8 : 0;
    
    /* Test 3: High words equal, low words differ */
    __int128 base = (__int128)0x1ULL << 64;
    __int128 a3 = base | 0x1ULL;
    __int128 b3 = base | 0x2ULL;
    checksum += (a3 < b3) ? 16 : 0;  /* Should trigger low word comparison */
    checksum += (b3 > a3) ? 32 : 0;
    
    return checksum;
}

/* Test boundary values and overflow */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Maximum and minimum 128-bit values */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = -max_signed - 1;
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    /* Compare at boundaries */
    checksum += (max_signed > 0) ? 64 : 0;
    checksum += (min_signed < 0) ? 128 : 0;
    checksum += ((unsigned __int128)max_unsigned > max_signed) ? 256 : 0;
    
    /* Test near overflow boundaries */
    __int128 near_max = max_signed - 100;
    __int128 near_min = min_signed + 100;
    checksum += (near_max < max_signed) ? 512 : 0;
    checksum += (near_min > min_signed) ? 1024 : 0;
    
    return checksum;
}

/* Test with arithmetic operations that may overflow */
static int test_overflow_comparisons(void) {
    int checksum = 0;
    
    __int128 large = (__int128)0x7FFFFFFFFFFFFFFFULL << 32;
    __int128 small = 0xFFFFFFFFULL;
    
    /* These operations may trigger overflow analysis */
    __int128 sum = large + small;
    __int128 diff = large - small;
    __int128 prod = large * 2;
    
    /* Comparisons after arithmetic */
    checksum += (sum > large) ? 2048 : 0;
    checksum += (diff < large) ? 4096 : 0;
    checksum += (prod > large) ? 8192 : 0;
    
    /* Use builtin overflow checks */
    __int128 of_result;
    int overflow = __builtin_add_overflow(large, large, &of_result);
    checksum += overflow ? 16384 : 0;
    
    overflow = __builtin_mul_overflow(large, 2, &of_result);
    checksum += overflow ? 32768 : 0;
    
    return checksum;
}

/* Test mixed-precision comparisons */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    __int128 wide_val = (__int128)0x1ULL << 64;
    long long narrow_val = 0x7FFFFFFFFFFFFFFFLL;
    unsigned long long unsigned_narrow = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Mixed type comparisons */
    checksum += (wide_val > narrow_val) ? 65536 : 0;
    checksum += (wide_val > (__int128)unsigned_narrow) ? 131072 : 0;
    checksum += ((unsigned __int128)wide_val > unsigned_narrow) ? 262144 : 0;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (narrow_val > 0) ? wide_val : narrow_val;
    checksum += (ternary_result == wide_val) ? 524288 : 0;
    
    return checksum;
}

/* Test with arrays to give optimizer substantial work */
static int test_array_comparisons(void) {
    int checksum = 0;
    
    /* Array of values that exercise different comparison paths */
    __int128 values[8] = {
        0,
        (__int128)0x1ULL << 63,
        (__int128)0x1ULL << 64,
        (__int128)0x3ULL << 64,
        -((__int128)0x1ULL << 63),
        -((__int128)0x1ULL << 64),
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x1ULL,
        -(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x1ULL)
    };
    
    /* Perform comparisons that should trigger all paths */
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (values[i] < values[j]) checksum += 1;
            if (values[i] > values[j]) checksum += 2;
            if (values[i] == values[j]) checksum += 4;
            if (values[i] != values[j]) checksum += 8;
        }
    }
    
    return checksum;
}

/* Test bitwise operations that cross word boundaries */
static int test_bitwise_comparisons(void) {
    int checksum = 0;
    
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = (__int128)0xFFFFFFFFFFFFFFFFULL << 64;
    __int128 mask_cross = ((__int128)0xFFFFFFFFULL << 32) | 0xFFFFFFFFULL;
    
    __int128 val1 = (__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL;
    __int128 val2 = ~val1;
    
    /* Bitwise operations followed by comparisons */
    __int128 and_result = val1 & mask_cross;
    __int128 or_result = val1 | mask_high;
    __int128 shift_result = val1 << 32;
    __int128 shift_right = val1 >> 32;
    
    checksum += (and_result < val1) ? 1048576 : 0;
    checksum += (or_result > val1) ? 2097152 : 0;
    checksum += (shift_result > val1) ? 4194304 : 0;
    checksum += (shift_right < val1) ? 8388608 : 0;
    
    return checksum;
}

/* Test with compiler builtins */
static int test_builtin_comparisons(void) {
    int checksum = 0;
    
    unsigned __int128 uval = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) |
                             0xFEDCBA9876543210ULL;
    
    /* Use builtins that may trigger internal comparisons */
    int leading_zeros = __builtin_clzll((unsigned long long)(uval >> 64));
    int trailing_zeros = __builtin_ctzll((unsigned long long)uval);
    int popcount = __builtin_popcountll((unsigned long long)(uval >> 64)) +
                   __builtin_popcountll((unsigned long long)uval);
    
    checksum += leading_zeros;
    checksum += trailing_zeros * 256;
    checksum += popcount * 65536;
    
    /* __builtin_expect with wide comparisons */
    __int128 a = (__int128)0x1ULL << 64;
    __int128 b = (__int128)0x2ULL << 64;
    
    if (__builtin_expect(a < b, 1)) checksum += 16777216;
    if (__builtin_expect(b > a, 1)) checksum += 33554432;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    /* Execute all tests to trigger various comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_comparisons();
    total_checksum += test_mixed_precision();
    total_checksum += test_array_comparisons();
    total_checksum += test_bitwise_comparisons();
    total_checksum += test_builtin_comparisons();
    
    /* Force evaluation of all comparisons by using the result */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Additional compile-time forced evaluations */
    #if defined(__GNUC__) && __GNUC__ >= 4
    /* Force constant folding with __builtin_constant_p */
    if (__builtin_constant_p(((__int128)0x1ULL << 64) > 0)) {
        printf("Constant folding triggered for high-word comparison\n");
    }
    #endif
    
    return total_checksum != 0 ? 0 : 1;
}
