/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT 0x123456789ABCDEF0ULL

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > ((__int128)(HIGH_BIT_64 - 1) << 64),
               "High word comparison test 1");
_Static_assert(((__int128)MAX_64 << 64) < (__int128)-1,
               "High word comparison test 2");

/* Function to create value ranges for VRP analysis */
__int128 create_range(int selector, unsigned long long seed) {
    __int128 result = 0;
    
    switch (selector) {
        case 0:
            /* High word positive, low word varies */
            result = ((__int128)seed << 64) | (seed ^ 0x5555555555555555ULL);
            break;
        case 1:
            /* High word negative, low word varies */
            result = ((__int128)(HIGH_BIT_64 | seed) << 64) | seed;
            break;
        case 2:
            /* High word zero, low word large */
            result = (__int128)seed;
            if (seed & HIGH_BIT_64) result = -result;
            break;
        case 3:
            /* Both words equal */
            result = ((__int128)seed << 64) | seed;
            break;
        case 4:
            /* Near INT128_MAX */
            result = ((__int128)MAX_64 << 64) | MAX_64;
            result -= seed;
            break;
        case 5:
            /* Near INT128_MIN */
            result = ((__int128)HIGH_BIT_64 << 64);
            result += seed;
            break;
        default:
            /* Mixed positive/negative */
            result = ((__int128)(seed & HIGH_BIT_64 ? HIGH_BIT_64 : 0) << 64) | seed;
    }
    
    return result;
}

/* Test high word comparisons */
int test_high_word_comparisons(void) {
    volatile int checksum = 0;
    
    /* Comparisons where high words differ (both positive) */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should be true */
    checksum += (a1 > b1) ? 2 : 0;  /* Should be false */
    
    /* Comparisons where high words differ (negative vs positive) */
    __int128 a2 = ((__int128)HIGH_BIT_64 << 64);  /* Negative */
    __int128 b2 = ((__int128)0x1ULL << 64);       /* Positive */
    checksum += (a2 < b2) ? 4 : 0;  /* Should be true (negative < positive) */
    checksum += (a2 > b2) ? 8 : 0;  /* Should be false */
    
    /* Comparisons where high words differ (both negative) */
    __int128 a3 = ((__int128)(HIGH_BIT_64 | 0x1ULL) << 64);
    __int128 b3 = ((__int128)(HIGH_BIT_64 | 0x2ULL) << 64);
    checksum += (a3 < b3) ? 16 : 0;  /* Should be true (more negative < less negative) */
    checksum += (a3 > b3) ? 32 : 0;  /* Should be false */
    
    return checksum;
}

/* Test low word comparisons (when high words are equal) */
int test_low_word_comparisons(void) {
    volatile int checksum = 0;
    
    /* Same high word, different low words */
    __int128 base_high = ((__int128)0x123456789ABCDEF0ULL << 64);
    __int128 a1 = base_high | 0x1111111111111111ULL;
    __int128 b1 = base_high | 0x2222222222222222ULL;
    checksum += (a1 < b1) ? 1 : 0;
    checksum += (a1 > b1) ? 2 : 0;
    
    /* Edge case: low word overflow comparison */
    __int128 a2 = base_high | MAX_64;
    __int128 b2 = base_high | 0x0ULL;
    /* a2.low > b2.low, but careful with signed comparison */
    checksum += ((unsigned __int128)a2 < (unsigned __int128)b2) ? 4 : 0;
    checksum += ((unsigned __int128)a2 > (unsigned __int128)b2) ? 8 : 0;
    
    /* Negative numbers with same high word */
    __int128 neg_high = ((__int128)(HIGH_BIT_64 | 0x12345678ULL) << 64);
    __int128 a3 = neg_high | 0x1111111111111111ULL;
    __int128 b3 = neg_high | 0x2222222222222222ULL;
    checksum += (a3 < b3) ? 16 : 0;
    checksum += (a3 > b3) ? 32 : 0;
    
    return checksum;
}

/* Test boundary values */
int test_boundary_comparisons(void) {
    volatile int checksum = 0;
    
    /* Define boundary values */
    __int128 int128_max = ((__int128)MAX_64 << 64) | MAX_64;
    __int128 int128_min = ((__int128)HIGH_BIT_64 << 64);
    __int128 zero = 0;
    __int128 one = 1;
    __int128 minus_one = -1;
    
    unsigned __int128 uint128_max = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    /* Signed comparisons at boundaries */
    checksum += (int128_min < int128_max) ? 1 : 0;
    checksum += (int128_min > int128_max) ? 2 : 0;
    checksum += (zero > int128_min) ? 4 : 0;
    checksum += (zero < int128_max) ? 8 : 0;
    
    /* Unsigned comparisons */
    checksum += ((unsigned __int128)zero < uint128_max) ? 16 : 0;
    checksum += ((unsigned __int128)minus_one < uint128_max) ? 32 : 0;
    checksum += ((unsigned __int128)minus_one > uint128_max) ? 64 : 0;
    
    /* Mixed signed/unsigned */
    checksum += ((__int128)uint128_max > int128_max) ? 128 : 0;
    
    return checksum;
}

/* Test with overflow operations */
int test_overflow_comparisons(void) {
    volatile int checksum = 0;
    
    __int128 x = ((__int128)MAX_64 << 64) | (MAX_64 - 1000);
    __int128 y = 1001;
    
    /* These should trigger overflow analysis */
    __int128 sum, diff, prod;
    int overflow_add, overflow_mul;
    
    overflow_add = __builtin_add_overflow(x, y, &sum);
    overflow_mul = __builtin_mul_overflow(x, 2, &prod);
    
    checksum += overflow_add ? 1 : 0;
    checksum += overflow_mul ? 2 : 0;
    
    /* Compare results */
    checksum += (sum > x) ? 4 : 0;
    checksum += (prod < x) ? 8 : 0;  /* Should be true due to overflow */
    
    /* Test with unsigned */
    unsigned __int128 ux = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 uy = 1;
    unsigned __int128 usum;
    
    int uoverflow = __builtin_add_overflow(ux, uy, &usum);
    checksum += uoverflow ? 16 : 0;
    checksum += (usum < ux) ? 32 : 0;  /* Should be true due to wrap-around */
    
    return checksum;
}

/* Test with arrays and loops for VRP */
int test_array_vrp_comparisons(void) {
    volatile int checksum = 0;
    
    /* Array of __int128 values spanning different ranges */
    __int128 arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = create_range(i, LARGE_CONSTANT + i * 0x1000ULL);
    }
    
    /* Perform comparisons that should trigger VRP */
    for (int i = 0; i < 7; i++) {
        /* These comparisons create value ranges */
        if (arr[i] < arr[i + 1]) {
            checksum += 1 << i;
        }
        
        /* Force high-word comparisons */
        if ((i & 1) == 0) {
            __int128 shifted = arr[i] << 2;
            if (shifted > arr[i]) {
                checksum += 1 << (i + 8);
            }
        }
    }
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 mask = ((__int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    for (int i = 0; i < 8; i++) {
        __int128 masked = arr[i] & mask;
        __int128 shifted = arr[i] >> 3;
        
        if (masked < shifted) {
            checksum += 1 << (i + 16);
        }
    }
    
    return checksum;
}

/* Test mixed-precision operations */
int test_mixed_precision_comparisons(void) {
    volatile int checksum = 0;
    
    __int128 large = ((__int128)0x123456789ABCDEF0ULL << 64);
    long long medium = 0x123456789ABCDEF0LL;
    unsigned long long umedium = 0xFEDCBA9876543210ULL;
    size_t ssize = (size_t)-1;
    
    /* Mixed comparisons */
    checksum += (large > medium) ? 1 : 0;
    checksum += (large < (__int128)umedium) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (medium > 0) ? large : (__int128)medium;
    checksum += (ternary_result == large) ? 4 : 0;
    
    /* Comparison with size_t (platform dependent) */
    checksum += ((unsigned __int128)large > ssize) ? 8 : 0;
    
    /* Implicit conversions in arithmetic */
    __int128 mixed_sum = large + medium;
    checksum += (mixed_sum > large) ? 16 : 0;
    
    /* Bitwise with mixed types */
    __int128 bitwise_and = large & medium;
    checksum += (bitwise_and < large) ? 32 : 0;
    
    return checksum;
}

/* Test compiler builtins */
int test_builtin_comparisons(void) {
    volatile int checksum = 0;
    
    unsigned __int128 x = ((unsigned __int128)0x8000000000000001ULL << 64) | 0x0000000000000001ULL;
    
    /* Count leading zeros - requires high word check */
    int clz_high = __builtin_clzll((unsigned long long)(x >> 64));
    int clz_low = __builtin_clzll((unsigned long long)x);
    checksum += (clz_high < clz_low) ? 1 : 0;
    
    /* Population count */
    int popcnt = __builtin_popcountll((unsigned long long)(x >> 64)) +
                 __builtin_popcountll((unsigned long long)x);
    checksum += (popcnt > 0) ? 2 : 0;
    
    /* Byte swap simulation */
    unsigned __int128 y = 0;
    for (int i = 0; i < 16; i++) {
        y |= ((x >> (8 * i)) & 0xFF) << (8 * (15 - i));
    }
    checksum += (y != x) ? 4 : 0;
    
    /* __builtin_expect with wide comparisons */
    __int128 a = ((__int128)0x1ULL << 64);
    __int128 b = ((__int128)0x2ULL << 64);
    
    if (__builtin_expect(a < b, 1)) {
        checksum += 8;
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all test suites */
    total_checksum += test_high_word_comparisons();
    printf("High word comparisons: %d\n", total_checksum);
    
    total_checksum += test_low_word_comparisons();
    printf("Low word comparisons: %d\n", total_checksum);
    
    total_checksum += test_boundary_comparisons();
    printf("Boundary comparisons: %d\n", total_checksum);
    
    total_checksum += test_overflow_comparisons();
    printf("Overflow comparisons: %d\n", total_checksum);
    
    total_checksum += test_array_vrp_comparisons();
    printf("Array VRP comparisons: %d\n", total_checksum);
    
    total_checksum += test_mixed_precision_comparisons();
    printf("Mixed precision comparisons: %d\n", total_checksum);
    
    total_checksum += test_builtin_comparisons();
    printf("Builtin comparisons: %d\n", total_checksum);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Force use of all values to prevent dead code elimination */
    volatile __int128 force_use = 0;
    for (int i = 0; i < 8; i++) {
        force_use += create_range(i, i * 0x1000ULL);
    }
    
    return total_checksum != 0 ? 0 : 1;
}
