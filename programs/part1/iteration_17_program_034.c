/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128_HIGH  0x123456789ABCDEF0ULL
#define MID_128_LOW   0xFEDCBA9876543210ULL

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > ((__int128)MAX_64), 
               "High word comparison test 1");
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)HIGH_BIT_64 << 64),
               "High word comparison test 2");

/* Test function for range analysis with __int128 */
static __int128 range_analysis_test(unsigned long long seed) {
    __int128 result = 0;
    __int128 base = ((__int128)seed << 64) | seed;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -((__int128)1 << 70); i < ((__int128)1 << 70); i += (1ULL << 40)) {
        if (i < 0) {
            result += i & base;
        } else {
            result |= i ^ base;
        }
    }
    
    return result;
}

/* Test overflow operations */
static int test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow_add, overflow_mul;
    
    /* Use builtins for overflow checking */
    overflow_add = __builtin_add_overflow(a, b, &sum);
    __builtin_mul_overflow(a, b, &prod);
    
    /* These comparisons may trigger double_int logic */
    if (sum > 0 && prod < 0) return 1;
    if (a > b && sum < diff) return 2;
    
    return overflow_add ? 3 : 0;
}

/* Mixed-precision comparisons */
static long long mixed_precision_test(__int128 a, unsigned long long b) {
    /* Compare __int128 with narrower types */
    if (a == (__int128)b) return 1;
    if (a < (__int128)b) return 2;
    if (a > (__int128)b) return 3;
    
    /* Ternary with mixed types */
    __int128 result = (b > 1000) ? a : (__int128)(b * b);
    
    /* Bitwise operations crossing 64-bit boundary */
    result = result & (((__int128)MAX_64 << 64) | MAX_64);
    result = result | ((__int128)HIGH_BIT_64 << 63);
    
    return (long long)(result >> 64);
}

/* Switch with __int128 cases (compile-time constants) */
static int switch_test(__int128 val) {
    /* GCC may generate comparison trees for these cases */
    switch ((unsigned __int128)val & 0xFF) {
        case 0: return 0;
        case ((unsigned __int128)HIGH_BIT_64 >> 56): return 1;
        case ((unsigned __int128)MAX_64 >> 56): return 2;
        default: return 3;
    }
}

/* Array operations for optimizer */
static __int128 process_array(const __int128 arr[], int size) {
    __int128 acc = 0;
    
    for (int i = 0; i < size; i++) {
        /* Comparisons that exercise high/low words */
        if (arr[i] > (((__int128)i << 120) | i)) {
            acc += arr[i];
        } else if (arr[i] < -(((__int128)i << 120) | i)) {
            acc -= arr[i];
        } else {
            acc ^= arr[i];
        }
        
        /* Bit counting operations */
        unsigned long long low = (unsigned long long)arr[i];
        unsigned long long high = (unsigned long long)(arr[i] >> 64);
        acc += __builtin_popcountll(low);
        acc += __builtin_popcountll(high) << 64;
    }
    
    return acc;
}

int main(void) {
    /* Initialize test array with values that exercise comparison paths */
    __int128 test_array[8];
    
    /* 1. Values where high words differ (positive) */
    test_array[0] = ((__int128)1 << 64) | 1;          /* High = 1, Low = 1 */
    test_array[1] = ((__int128)2 << 64) | 0xFFFFFFFFFFFFFFFFULL; /* High = 2, Low = max */
    
    /* 2. Values where high words differ (negative) */
    test_array[2] = -((__int128)1 << 64);             /* High = -1, Low = 0 */
    test_array[3] = -((__int128)2 << 64) | 1;         /* High = -2, Low = 1 */
    
    /* 3. Values where high words equal, low words differ */
    test_array[4] = ((__int128)0x12345678 << 64) | 1;
    test_array[5] = ((__int128)0x12345678 << 64) | 2;
    
    /* 4. Boundary values */
    test_array[6] = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64; /* Near INT128_MAX */
    test_array[7] = ((__int128)0x8000000000000000ULL << 64);          /* Near INT128_MIN */
    
    /* Accumulator for checksum */
    unsigned __int128 checksum = 0;
    
    /* Test 1: Direct comparisons between array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_array[i] < test_array[j]) checksum += 1;
            if (test_array[i] > test_array[j]) checksum += 2;
            if (test_array[i] == test_array[j]) checksum += 3;
            if (test_array[i] != test_array[j]) checksum += 5;
            
            /* Use __builtin_expect to influence branch prediction */
            if (__builtin_expect(test_array[i] >= 0, 1)) {
                checksum += test_array[i];
            }
        }
    }
    
    /* Test 2: Range analysis */
    __int128 range_result = range_analysis_test(0xDEADBEEF);
    checksum += (unsigned __int128)range_result;
    
    /* Test 3: Overflow operations */
    for (int i = 0; i < 8; i++) {
        checksum += test_overflow_ops(test_array[i], test_array[(i + 1) % 8]);
    }
    
    /* Test 4: Mixed precision */
    for (int i = 0; i < 8; i++) {
        checksum += mixed_precision_test(test_array[i], 0x123456789ABCDEF0ULL);
    }
    
    /* Test 5: Array processing */
    __int128 array_result = process_array(test_array, 8);
    checksum += (unsigned __int128)array_result;
    
    /* Test 6: Switch with constant comparisons */
    for (int i = 0; i < 8; i++) {
        checksum += switch_test(test_array[i]);
    }
    
    /* Test 7: Bitwise operations crossing boundaries */
    for (int i = 0; i < 8; i++) {
        __int128 shifted = test_array[i] << 32;
        __int128 masked = shifted & (((__int128)MAX_64 << 64) | MAX_64);
        __int128 swapped = ((masked & 0xFFFF) << 112) | 
                          ((masked & 0xFFFF0000) << 80) |
                          ((masked >> 80) & 0xFFFF0000) |
                          ((masked >> 112) & 0xFFFF);
        
        if (swapped > masked) checksum += 1;
        if (swapped < masked) checksum += 2;
    }
    
    /* Test 8: Variadic function with __int128 (triggers conversions) */
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits): %llu\n", 
           (unsigned long long)checksum);
    
    /* Force use of all results to prevent dead code elimination */
    volatile __int128 sink = range_result + array_result;
    (void)sink;
    
    return 0;
}
