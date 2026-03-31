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
#define MID_128_HIGH  0x123456789ABCDEF0ULL
#define MID_128_LOW   0xFEDCBA9876543210ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64 << 64) > (__int128)MAX_64,
               "128-bit comparison with high word difference");

/* Test function that forces range analysis on __int128 */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundaries */
    for (__int128 i = start; i < end; i += (HIGH_BIT_64 >> 61)) {
        /* Force comparisons where high words may differ */
        if (i < (__int128)0) {
            sum -= i;
        } else if (i > (__int128)(HIGH_BIT_64 << 32)) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Overflow checks requiring wide comparisons */
        __int128 overflow_test;
        if (__builtin_add_overflow(sum, i, &overflow_test)) {
            sum = i;
        }
    }
    return sum;
}

/* Function to test mixed-precision comparisons */
static int compare_mixed_types(__int128 a, unsigned __int128 b) {
    int result = 0;
    
    /* Direct comparisons that may use double_int logic */
    if (a < (__int128)b) result |= 1;
    if ((unsigned __int128)a > b) result |= 2;
    if (a == (__int128)b) result |= 4;
    
    /* Ternary with mixed types */
    __int128 c = (a > 0) ? a : (__int128)b;
    if (c > a) result |= 8;
    
    return result;
}

/* Test bitwise operations crossing 64-bit boundary */
static unsigned __int128 test_bitwise_ops(unsigned __int128 x) {
    /* Operations that affect both high and low words */
    unsigned __int128 y = x << 65;  /* Cross 64-bit boundary */
    unsigned __int128 z = y >> 33;
    unsigned __int128 w = x & ((unsigned __int128)MAX_64 << 64);
    
    /* Comparisons after bitwise ops */
    if (y > z) w |= 1;
    if ((x & HIGH_BIT_64) != 0) w |= 2;
    
    return w;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int test_switch(__int128 value) {
    switch (value) {
        case ((__int128)0x1000000000000000ULL << 64):
            return 1;
        case ((__int128)(-1) * ((__int128)0x1000000000000000ULL << 64)):
            return 2;
        case ((__int128)MID_128_HIGH << 64 | MID_128_LOW):
            return 3;
        case 0:
            return 4;
        default:
            return 5;
    }
}

/* Array operations to give optimizer substantial work */
static __int128 process_array(__int128 arr[], int size) {
    __int128 checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Compare array elements - may trigger double_int comparisons */
        if (i > 0 && arr[i] > arr[i-1]) {
            checksum += arr[i];
        } else if (i > 0 && arr[i] < arr[i-1]) {
            checksum -= arr[i];
        }
        
        /* Compare with boundaries */
        if (arr[i] > ((__int128)MAX_64 << 32)) {
            checksum |= 1;
        }
        if (arr[i] < -((__int128)MAX_64 << 32)) {
            checksum |= 2;
        }
    }
    
    return checksum;
}

/* Use builtins that may involve wide integer comparisons */
static int test_builtins(__int128 x) {
    int result = 0;
    
    /* __builtin_expect with __int128 comparison */
    if (__builtin_expect(x > 0, 1)) {
        result++;
    }
    
    /* Count leading zeros on high word */
    unsigned __int128 ux = (unsigned __int128)x;
    if (ux != 0) {
        int clz_high = __builtin_clzll(ux >> 64);
        int clz_low = __builtin_clzll(ux & MAX_64);
        result += clz_high + clz_low;
    }
    
    return result;
}

int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Values where only high words differ */
    __int128 a1 = (__int128)HIGH_BIT_64 << 64;  /* High word = 0x8000000000000000 */
    __int128 b1 = (__int128)(HIGH_BIT_64 - 1) << 64;
    checksum += (a1 > b1) ? 1 : -1;
    checksum += (a1 < b1) ? 2 : -2;
    
    /* Test 2: High words equal, low words differ */
    __int128 a2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x7FFFFFFFFFFFFFFFULL;
    checksum += (a2 > b2) ? 4 : -4;
    checksum += (a2 < b2) ? 8 : -8;
    
    /* Test 3: Boundary values */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 min_signed = ((__int128)HIGH_BIT_64 << 64);
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    checksum += (max_signed > min_signed) ? 16 : -16;
    checksum += ((unsigned __int128)max_signed < max_unsigned) ? 32 : -32;
    
    /* Test 4: Mixed signed/unsigned comparisons */
    checksum += compare_mixed_types(a1, (unsigned __int128)b1);
    checksum += compare_mixed_types(-a1, (unsigned __int128)b1);
    
    /* Test 5: Array operations */
    __int128 arr[8] = {
        (__int128)0,
        (__int128)1 << 66,
        (__int128)(-1) << 66,
        ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        max_signed,
        min_signed,
        (__int128)max_unsigned / 2
    };
    
    checksum += process_array(arr, 8);
    
    /* Test 6: Range analysis triggers */
    __int128 range_result = process_range(min_signed + 1000, min_signed + 10000);
    checksum += range_result;
    
    /* Test 7: Bitwise operations */
    unsigned __int128 bitwise_result = test_bitwise_ops(
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0x0FEDCBA987654321ULL
    );
    checksum += (__int128)bitwise_result;
    
    /* Test 8: Switch statement */
    checksum += test_switch(((__int128)MID_128_HIGH << 64 | MID_128_LOW));
    checksum += test_switch(0);
    
    /* Test 9: Builtin functions */
    checksum += test_builtins(a1);
    checksum += test_builtins(b1);
    checksum += test_builtins(max_signed);
    
    /* Test 10: Arithmetic with overflow */
    __int128 x = max_signed / 2;
    __int128 y = max_signed / 2 + 1;
    __int128 sum, diff, prod;
    
    if (__builtin_add_overflow(x, y, &sum)) {
        checksum |= 0x100;
    }
    if (__builtin_mul_overflow(x, y, &prod)) {
        checksum |= 0x200;
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum (low 64 bits): %lld\n", (long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (high 64 bits): %lld\n", (long long)(checksum >> 64));
    
    /* Force printf to handle __int128 */
    unsigned __int128 print_test = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0x0FEDCBA987654321ULL;
    printf("Test value: 0x%016llx%016llx\n", 
           (unsigned long long)(print_test >> 64),
           (unsigned long long)(print_test & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
