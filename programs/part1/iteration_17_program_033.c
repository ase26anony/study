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
#define LARGE_CONSTANT 0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "128-bit comparison with high word difference");

/* Function to force range analysis on __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    /* Loop with 128-bit induction variable crossing 64-bit boundary */
    for (__int128 i = start; i < end; i += (HIGH_BIT_64 >> 61)) {
        /* Force comparisons in loop condition */
        if (i < (end / 2)) {
            sum += i;
        } else {
            sum -= i;
        }
        
        /* Additional comparisons to exercise high/low word logic */
        if (i > (start * 3)) {
            sum |= i;
        }
        
        /* Compare with 64-bit boundary values */
        if (i < HIGH_BIT_64) {
            sum &= ~i;
        }
    }
    return sum;
}

/* Function using built-in overflow checks with __int128 */
static int check_overflow_operations(void) {
    __int128 a = ((__int128)HIGH_BIT_64 << 32);
    __int128 b = ((__int128)MAX_64 << 16);
    __int128 result;
    int overflow;
    
    /* Force overflow checks requiring wide comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (!overflow && result > a) {
        /* Comparison between values with different high words */
        return (result < b) ? 1 : 0;
    }
    
    overflow = __builtin_mul_overflow(a, 2, &result);
    if (!overflow && result == (a << 1)) {
        /* Shift operation crossing 64-bit boundary */
        return (result > HIGH_BIT_64) ? 2 : 0;
    }
    
    return 0;
}

/* Mixed-precision comparisons */
static unsigned long long mixed_comparisons(__int128 wide_val) {
    unsigned long long checksum = 0;
    
    /* Compare __int128 with various 64-bit types */
    if (wide_val > LLONG_MAX) checksum |= 0x1;
    if (wide_val < LLONG_MIN) checksum |= 0x2;
    if ((unsigned __int128)wide_val > ULLONG_MAX) checksum |= 0x4;
    if ((unsigned __int128)wide_val < 0xFFFFFFFFULL) checksum |= 0x8;
    
    /* Ternary operator with mixed types */
    long long narrow = (wide_val > 0) ? (long long)(wide_val & MAX_64) : -1;
    checksum += (unsigned long long)narrow;
    
    return checksum;
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 cross_boundary_bitops(__int128 x, __int128 y) {
    __int128 result = 0;
    
    /* Operations that affect both high and low words */
    result = (x << 32) | (y >> 96);
    result &= ~((__int128)MAX_64 << 64);
    result |= ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    
    /* Comparisons after bit operations */
    if ((result & ((__int128)MAX_64 << 64)) > ((__int128)HIGH_BIT_64 << 64)) {
        result ^= ((__int128)MAX_64 << 64);
    }
    
    return result;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int wide_switch(__int128 value) {
    /* GCC may generate comparison trees for these cases */
    switch ((unsigned long long)(value >> 64)) {
        case 0x0:
            return 1;
        case 0x1:
            return 2;
        case 0x8000000000000000ULL:
            return 3;
        case 0xFFFFFFFFFFFFFFFFULL:
            return 4;
        default:
            return 0;
    }
}

/* Array operations with __int128 */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple comparisons in array processing */
        if (i > 0 && arr[i] > arr[i-1]) {
            checksum += arr[i] - arr[i-1];
        } else if (i > 0 && arr[i] < arr[i-1]) {
            checksum -= arr[i-1] - arr[i];
        }
        
        /* Compare with array boundaries */
        if (arr[i] > arr[0]) {
            checksum |= arr[i];
        }
        
        /* Force high-word comparisons */
        if ((arr[i] >> 64) > (checksum >> 64)) {
            checksum ^= arr[i];
        }
    }
    
    return checksum;
}

/* Use built-in functions with __int128 */
static unsigned int builtin_operations(__int128 x) {
    unsigned int result = 0;
    
    /* Count leading zeros in high word */
    if (x > 0) {
        unsigned long long high = (unsigned long long)((unsigned __int128)x >> 64);
        result += __builtin_clzll(high);
    }
    
    /* Population count on both halves */
    unsigned long long low = (unsigned long long)((unsigned __int128)x & MAX_64);
    result += __builtin_popcountll(low);
    
    /* Byte swap simulation */
    __int128 swapped = ((__int128)__builtin_bswap64(low) << 64) | 
                       __builtin_bswap64((unsigned long long)(x >> 64));
    
    /* Comparison after byte swap */
    if (swapped < x) {
        result |= 0x80000000;
    }
    
    return result;
}

int main(void) {
    __int128 checksum = 0;
    unsigned long long final_result = 0;
    
    /* Test 1: Values where only high words differ */
    __int128 val1 = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 val2 = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    
    if (val1 < val2) checksum += 1;  /* Should trigger high-word comparison */
    if (val2 > val1) checksum += 2;
    
    /* Test 2: High words equal, low words differ */
    __int128 val3 = ((__int128)0x1ULL << 64) | 0x0ULL;
    __int128 val4 = ((__int128)0x1ULL << 64) | 0x1ULL;
    
    if (val3 < val4) checksum += 4;  /* Should trigger low-word comparison */
    if (val4 > val3) checksum += 8;
    
    /* Test 3: Boundary values */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL;
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    if (min_signed < max_signed) checksum += 16;
    if ((unsigned __int128)max_signed < max_unsigned) checksum += 32;
    
    /* Test 4: Range analysis with loops */
    __int128 range_result = process_range(0, ((__int128)0x10ULL << 64));
    checksum ^= range_result;
    
    /* Test 5: Overflow checks */
    final_result += check_overflow_operations();
    
    /* Test 6: Mixed precision */
    final_result += mixed_comparisons(val1);
    
    /* Test 7: Bitwise operations */
    __int128 bit_result = cross_boundary_bitops(val1, val2);
    checksum += bit_result;
    
    /* Test 8: Switch statement */
    final_result += wide_switch(min_signed);
    
    /* Test 9: Array operations (8 elements as requested) */
    __int128 arr[8] = {
        0,
        ((__int128)0x1ULL << 63),
        ((__int128)0x1ULL << 64),
        ((__int128)0x8000000000000000ULL << 64),
        ((__int128)MAX_64 << 64) | MAX_64,
        -((__int128)0x1ULL << 64),
        ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL,
        ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL
    };
    
    __int128 array_result = process_array(arr, 8);
    checksum += array_result;
    
    /* Test 10: Built-in functions */
    final_result += builtin_operations(checksum);
    
    /* Combine all results to prevent dead code elimination */
    final_result += (unsigned long long)(checksum & MAX_64);
    final_result += (unsigned long long)((checksum >> 64) & MAX_64);
    
    /* Use __builtin_expect with __int128 comparison */
    if (__builtin_expect(checksum != 0, 1)) {
        final_result |= 0x100000000ULL;
    }
    
    printf("Result: 0x%016llX\n", final_result);
    
    /* Variadic function with __int128 conversion */
    printf("Low part: %lld\n", (long long)(checksum & MAX_64));
    
    return (final_result != 0) ? 0 : 1;
}
