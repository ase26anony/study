/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT 0x123456789ABCDEF0ULL

/* Test 1: Constant folding with __int128 comparisons */
static void test_constant_folding(void) {
    /* Force compile-time evaluation with static assertions */
    const __int128 a1 = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;  /* High and low both have MSB set */
    const __int128 a2 = ((__int128)HIGH_BIT_64 << 64) | (HIGH_BIT_64 - 1);
    const __int128 a3 = ((__int128)(HIGH_BIT_64 - 1) << 64) | HIGH_BIT_64;
    const __int128 a4 = ((__int128)(HIGH_BIT_64 - 1) << 64) | (HIGH_BIT_64 - 1);
    
    /* These comparisons should trigger high-word comparisons */
    _Static_assert(a1 < 0, "Negative __int128 constant");
    _Static_assert(a2 < a1, "High equal, low differs");
    _Static_assert(a3 > a4, "High differs, low equal");
    
    /* Mixed positive/negative comparisons */
    const __int128 pos_large = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    const __int128 neg_large = ((__int128)HIGH_BIT_64 << 64) | 0ULL;
    _Static_assert(pos_large > neg_large, "Positive > Negative");
}

/* Test 2: Range analysis with __int128 loops */
static unsigned long long test_range_analysis(void) {
    unsigned long long checksum = 0;
    
    /* Loop with __int128 induction variable crossing 64-bit boundary */
    for (__int128 i = ((__int128)HIGH_BIT_64 << 32); 
         i < ((__int128)HIGH_BIT_64 << 32) + 1000; 
         i++) {
        /* Force range analysis on comparisons */
        if (i > ((__int128)HIGH_BIT_64 << 32) + 500) {
            checksum += (unsigned long long)(i & 0xFFFFFFFF);
        }
    }
    
    /* Another loop with negative values */
    for (__int128 j = -((__int128)HIGH_BIT_64 << 32);
         j < -((__int128)HIGH_BIT_64 << 32) + 1000;
         j++) {
        if (j < -((__int128)HIGH_BIT_64 << 32) + 500) {
            checksum += (unsigned long long)(-j & 0xFFFFFFFF);
        }
    }
    
    return checksum;
}

/* Test 3: Overflow checking with builtins */
static unsigned long long test_overflow_checks(void) {
    unsigned long long checksum = 0;
    
    /* Test overflow addition */
    __int128 x = ((__int128)MAX_64 << 64) | MAX_64;
    __int128 y = 1;
    __int128 result;
    
    if (__builtin_add_overflow(x, y, &result)) {
        checksum += 1;  /* Should overflow */
    }
    
    /* Test overflow multiplication */
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 b = 2;
    
    if (__builtin_mul_overflow(a, b, &result)) {
        checksum += 2;  /* May overflow depending on constant folding */
    }
    
    /* Test unsigned overflow */
    unsigned __int128 ux = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 uy = 1;
    unsigned __int128 uresult;
    
    if (__builtin_add_overflow(ux, uy, &uresult)) {
        checksum += 4;  /* Should overflow */
    }
    
    return checksum;
}

/* Test 4: Bitwise operations crossing 64-bit boundary */
static unsigned long long test_bitwise_operations(void) {
    unsigned long long checksum = 0;
    
    /* Create __int128 values with specific bit patterns */
    __int128 pattern1 = ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL;
    __int128 pattern2 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    
    /* Bitwise operations that cross word boundaries */
    __int128 and_result = pattern1 & pattern2;
    __int128 or_result = pattern1 | pattern2;
    __int128 xor_result = pattern1 ^ pattern2;
    
    /* Shifts that move bits across the 64-bit boundary */
    __int128 left_shift = pattern1 << 65;  /* Crosses boundary */
    __int128 right_shift = pattern2 >> 65; /* Crosses boundary */
    
    /* Comparisons of shifted values */
    if (left_shift > 0) checksum += 1;
    if (right_shift < pattern2) checksum += 2;
    if ((and_result == 0) && (or_result == ~(__int128)0)) checksum += 4;
    
    return checksum;
}

/* Test 5: Array operations with __int128 */
static unsigned long long test_array_operations(void) {
    /* Array of __int128 values that exercise different comparison paths */
    __int128 arr[8] = {
        ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64,           /* Most negative */
        -((__int128)HIGH_BIT_64 << 63),                        /* Large negative */
        -1,                                                    /* -1 */
        0,                                                     /* Zero */
        1,                                                     /* Small positive */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 32),               /* Medium positive */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64,      /* Large positive */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | (MAX_64 - 1) /* Slightly smaller */
    };
    
    unsigned long long checksum = 0;
    
    /* Compare each element with every other element */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (arr[i] < arr[j]) checksum += 1;
            if (arr[i] > arr[j]) checksum += 2;
            if (arr[i] == arr[j]) checksum += 4;
            if (arr[i] != arr[j]) checksum += 8;
        }
    }
    
    /* Sort-like operation */
    for (int i = 0; i < 7; i++) {
        if (arr[i] > arr[i + 1]) {
            __int128 temp = arr[i];
            arr[i] = arr[i + 1];
            arr[i + 1] = temp;
            checksum += 16;
        }
    }
    
    return checksum;
}

/* Test 6: Mixed-type comparisons and conversions */
static unsigned long long test_mixed_type_operations(void) {
    unsigned long long checksum = 0;
    
    /* Compare __int128 with narrower types */
    __int128 wide_val = ((__int128)MAX_64 << 64) | MAX_64;
    long long narrow_val = MAX_64;
    size_t size_val = (size_t)-1;
    
    if (wide_val > narrow_val) checksum += 1;
    if (wide_val > size_val) checksum += 2;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (wide_val > 0) ? wide_val : (__int128)narrow_val;
    if (ternary_result == wide_val) checksum += 4;
    
    /* Implicit conversions in arithmetic */
    __int128 sum = wide_val + narrow_val + size_val;
    if (sum > wide_val) checksum += 8;
    
    /* Use __builtin_expect with __int128 comparison */
    if (__builtin_expect(wide_val < 0, 0)) {
        checksum += 16;
    }
    
    return checksum;
}

/* Test 7: Switch statement with __int128 case labels */
static unsigned long long test_switch_statement(void) {
    unsigned long long checksum = 0;
    
    /* GCC may generate comparison trees for switch on __int128 */
    __int128 switch_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    /* Note: Switch on __int128 may not be supported in all C standards,
     * but we use it to potentially trigger internal comparisons */
    if (switch_val == ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL) {
        checksum += 1;
    } else if (switch_val == ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL) {
        checksum += 2;
    } else if (switch_val < 0) {
        checksum += 4;
    } else {
        checksum += 8;
    }
    
    return checksum;
}

/* Test 8: Builtin functions with __int128 */
static unsigned long long test_builtin_functions(void) {
    unsigned long long checksum = 0;
    
    unsigned __int128 val = ((unsigned __int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    
    /* Count leading/trailing zeros on parts of __int128 */
    unsigned long long high_part = (unsigned long long)(val >> 64);
    unsigned long long low_part = (unsigned long long)val;
    
    checksum += __builtin_clzll(high_part);
    checksum += __builtin_ctzll(low_part);
    checksum += __builtin_popcountll(high_part);
    checksum += __builtin_popcountll(low_part);
    
    /* Manual byte swap for 128-bit */
    unsigned __int128 swapped = ((unsigned __int128)__builtin_bswap64(low_part) << 64) | 
                                 __builtin_bswap64(high_part);
    
    if (swapped != val) checksum += 0x1000;
    
    return checksum;
}

int main(void) {
    unsigned long long total_checksum = 0;
    
    /* Force constant folding evaluation */
    test_constant_folding();
    
    /* Execute all tests and accumulate checksum */
    total_checksum += test_range_analysis();
    total_checksum += test_overflow_checks();
    total_checksum += test_bitwise_operations();
    total_checksum += test_array_operations();
    total_checksum += test_mixed_type_operations();
    total_checksum += test_switch_statement();
    total_checksum += test_builtin_functions();
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %llu\n", total_checksum);
    
    /* Additional printf with __int128 argument (may trigger conversions) */
    __int128 final_val = ((__int128)total_checksum << 64) | total_checksum;
    printf("Final 128-bit value high: %llx, low: %llx\n",
           (unsigned long long)(final_val >> 64),
           (unsigned long long)(final_val & 0xFFFFFFFFFFFFFFFFULL));
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
