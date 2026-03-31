/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * for 128-bit integer operations during compilation.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64), 
               "128-bit comparison with high word difference");

/* Function to create value ranges that span both words */
static __int128 create_wide_range(int selector) {
    switch (selector) {
        case 0: return (__int128)0;
        case 1: return (__int128)1;
        case 2: return (__int128)-1;
        case 3: return ((__int128)HIGH_BIT_64 << 64) | MID_128;  /* High word = 0x8000..., low = MID */
        case 4: return ((__int128)HIGH_BIT_64 << 64) - 1;        /* Just below INT128_MIN */
        case 5: return ~((__int128)0) >> 1;                      /* INT128_MAX */
        case 6: return (__int128)MID_128 << 64;                  /* High word = MID, low = 0 */
        case 7: return -((__int128)MID_128 << 64);               /* Negative with high word */
        default: return 0;
    }
}

/* Test high-word comparisons (lines 1285-1288) */
static int test_high_word_comparisons(void) {
    volatile int checksum = 0;
    
    /* Comparisons where high words differ (signed) */
    __int128 a = ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL;  /* high=1, low=0xFFFFFFFF */
    __int128 b = ((__int128)0x2ULL << 64) | 0xFFFFFFFFULL;  /* high=2, low=0xFFFFFFFF */
    
    checksum += (a < b) ? 1 : 0;    /* Should be true - high word comparison */
    checksum += (b > a) ? 2 : 0;    /* Should be true */
    
    /* Negative values with different high words */
    __int128 c = -a;
    __int128 d = -b;
    
    checksum += (c > d) ? 4 : 0;    /* Should be true (less negative > more negative) */
    checksum += (d < c) ? 8 : 0;    /* Should be true */
    
    /* Comparisons where high words are equal but low words differ (lines 1289-1293) */
    __int128 e = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 f = ((__int128)0x1ULL << 64) | 0x2ULL;
    
    checksum += (e < f) ? 16 : 0;   /* Should be true - low word comparison */
    checksum += (f > e) ? 32 : 0;   /* Should be true */
    
    /* Edge case: low word at maximum */
    __int128 g = ((__int128)0x1ULL << 64) | MAX_64;
    __int128 h = ((__int128)0x2ULL << 64) | 0x0ULL;
    
    checksum += (g < h) ? 64 : 0;   /* Should be true */
    
    return checksum;
}

/* Test unsigned comparisons */
static int test_unsigned_comparisons(void) {
    volatile int checksum = 0;
    
    unsigned __int128 ua = ((unsigned __int128)0x1ULL << 64) | 0xFFFFFFFFULL;
    unsigned __int128 ub = ((unsigned __int128)0x2ULL << 64) | 0xFFFFFFFFULL;
    
    checksum += (ua < ub) ? 1 : 0;
    checksum += (ub > ua) ? 2 : 0;
    
    /* Test with high bit set in high word */
    unsigned __int128 uc = ((unsigned __int128)HIGH_BIT_64 << 64) | 0x0ULL;
    unsigned __int128 ud = ((unsigned __int128)(HIGH_BIT_64 >> 1) << 64) | MAX_64;
    
    checksum += (uc > ud) ? 4 : 0;  /* Should be true */
    
    return checksum;
}

/* Test range analysis with loops */
static int test_range_analysis(void) {
    volatile int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -((__int128)10 << 62); 
         i < ((__int128)10 << 62); 
         i += (__int128)1 << 60) {
        checksum += (i > 0) ? 1 : 0;
    }
    
    /* Overflow checking with builtins */
    __int128 x = ((__int128)MAX_64 << 60);
    __int128 y = ((__int128)MAX_64 << 60);
    __int128 result;
    
    if (__builtin_add_overflow(x, y, &result)) {
        checksum += 2;  /* Overflow should occur */
    }
    
    /* Multiplication overflow check */
    __int128 m = ((__int128)MAX_64 << 32);
    if (__builtin_mul_overflow(m, m, &result)) {
        checksum += 4;  /* Overflow should occur */
    }
    
    return checksum;
}

/* Test mixed-precision operations */
static int test_mixed_precision(void) {
    volatile int checksum = 0;
    
    __int128 wide = ((__int128)0x1ULL << 64) | 0x12345678ULL;
    long long narrow = 0x12345678ULL;
    unsigned long long unarrow = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Compare __int128 with narrower types */
    checksum += (wide > narrow) ? 1 : 0;
    checksum += (wide < unarrow) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (narrow > 0) ? wide : (__int128)narrow;
    checksum += (ternary_result == wide) ? 4 : 0;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 shifted = wide << 32;
    checksum += ((shifted >> 96) == 0x1ULL) ? 8 : 0;
    
    /* Use builtins that may trigger double_int comparisons */
    unsigned __int128 bits = ((unsigned __int128)0x1ULL << 64) | 0x1ULL;
    int clz = __builtin_clzll((unsigned long long)(bits >> 64));
    checksum += (clz == 63) ? 16 : 0;
    
    return checksum;
}

/* Test with arrays to give optimizer substantial work */
static int test_array_operations(void) {
    volatile int checksum = 0;
    
    /* Array of __int128 values that exercise different comparison paths */
    __int128 arr[8] = {
        0,
        ((__int128)0x1ULL << 64) | 0x1ULL,
        -((__int128)0x1ULL << 64) | 0x1ULL,
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)HIGH_BIT_64 << 64) - 1,
        ~((__int128)0) >> 1,  /* INT128_MAX */
        ~((__int128)0),       /* -1 in two's complement */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64
    };
    
    /* Perform comparisons between array elements */
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 8; j++) {
            checksum += (arr[i] < arr[j]) ? 1 : 0;
            checksum += (arr[i] > arr[j]) ? 2 : 0;
            checksum += (arr[i] == arr[j]) ? 4 : 0;
        }
    }
    
    /* Test boundary values */
    __int128 min_val = ~((__int128)0) << 127;  /* Approximate INT128_MIN */
    __int128 max_val = ~min_val;               /* Approximate INT128_MAX */
    
    checksum += (min_val < max_val) ? 256 : 0;
    checksum += (max_val > min_val) ? 512 : 0;
    
    return checksum;
}

/* Main function that accumulates all test results */
int main(void) {
    volatile int total_checksum = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    /* Execute all tests to trigger various comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_unsigned_comparisons();
    total_checksum += test_range_analysis();
    total_checksum += test_mixed_precision();
    total_checksum += test_array_operations();
    
    /* Force evaluation of __int128 comparisons in main */
    __int128 final_a = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    __int128 final_b = ((__int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL;
    
    total_checksum += (final_a < final_b) ? 1024 : 0;
    total_checksum += (final_a > final_b) ? 2048 : 0;
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect((final_a != final_b), 1)) {
        total_checksum += 4096;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Prevent dead code elimination */
    volatile __int128 sink = final_a + final_b + total_checksum;
    (void)sink;
    
    return total_checksum != 0 ? 0 : 1;
}
