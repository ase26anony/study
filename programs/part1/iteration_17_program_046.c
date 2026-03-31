/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x7FFFFFFFFFFFFFFFULL

/* Static assertions to force compile-time comparisons */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "Shifted max should be larger");

/* Function prototypes using __int128 extensively */
static __int128 add_with_overflow(__int128 a, __int128 b, int *overflow);
static unsigned __int128 rotate_left(unsigned __int128 x, int shift);
static int compare_arrays(__int128 *a, __int128 *b, size_t n);

/* Range analysis test with __int128 induction */
void range_analysis_test(void) {
    /* Loop with __int128 induction variable near 64-bit boundary */
    for (__int128 i = HIGH_BIT_64 - 100; i < HIGH_BIT_64 + 100; i++) {
        /* Force comparisons in loop condition */
        if (i < (__int128)HIGH_BIT_64) {
            /* This path exercises high-word comparison when i is negative */
            volatile __int128 temp = i * 2;
            (void)temp;
        }
    }
    
    /* Another loop crossing zero boundary */
    for (__int128 j = -HIGH_BIT_64; j < HIGH_BIT_64; j += HIGH_BIT_64 / 100) {
        /* Mixed comparisons */
        if (j > 0 && j < (__int128)(HIGH_BIT_64 >> 1)) {
            volatile __int128 temp = j;
            (void)temp;
        }
    }
}

/* Constant folding boundary tests */
void constant_folding_tests(void) {
    /* Large constants that require high-word comparisons */
    const __int128 A = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;
    const __int128 B = ((__int128)HIGH_BIT_64 << 64) | (HIGH_BIT_64 - 1);
    const __int128 C = ((__int128)(HIGH_BIT_64 - 1) << 64) | HIGH_BIT_64;
    const __int128 D = -((__int128)HIGH_BIT_64 << 64);
    
    /* Force compile-time comparisons */
    switch (sizeof(__int128)) {
        case 16: {
            /* These comparisons should be folded at compile time */
            int cmp1 = (A > B) ? 1 : ((A < B) ? -1 : 0);
            int cmp2 = (A > C) ? 1 : ((A < C) ? -1 : 0);
            int cmp3 = (D < C) ? 1 : ((D > C) ? -1 : 0);
            volatile int v1 = cmp1, v2 = cmp2, v3 = cmp3;
            (void)v1; (void)v2; (void)v3;
            break;
        }
    }
    
    /* Bitwise operations crossing 64-bit boundary */
    unsigned __int128 mask = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 shifted = mask >> 32;
    unsigned __int128 masked = shifted & ((unsigned __int128)0xFFFF << 96);
    
    volatile unsigned __int128 vmasked = masked;
    (void)vmasked;
}

/* Mixed precision operations */
void mixed_precision_tests(void) {
    /* Compare __int128 with narrower types */
    __int128 large = ((__int128)HIGH_BIT_64 << 64);
    long long medium = HIGH_BIT_64;
    size_t ssize = SIZE_MAX;
    
    /* These should trigger double_int comparisons */
    int cmp1 = (large > (__int128)medium) ? 1 : 0;
    int cmp2 = ((unsigned __int128)large > (unsigned __int128)ssize) ? 1 : 0;
    
    /* Ternary with mixed types */
    __int128 result = (cmp1 > cmp2) ? 
                     ((__int128)medium << 64) : 
                     (__int128)ssize;
    
    /* Variadic function with __int128 conversion */
    printf("Mixed precision: cmp1=%d, cmp2=%d\n", cmp1, cmp2);
    printf("Result high word: 0x%016llx\n", 
           (unsigned long long)(result >> 64));
    
    volatile __int128 vresult = result;
    (void)vresult;
}

/* Built-in function tests */
void builtin_tests(void) {
    unsigned __int128 x = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) |
                          0xFEDCBA9876543210ULL;
    
    /* Operations that may use double_int internally */
    int leading_zero = __builtin_clzll((unsigned long long)(x >> 64));
    int trailing_zero = __builtin_ctzll((unsigned long long)x);
    int popcount = __builtin_popcountll((unsigned long long)(x >> 64)) +
                   __builtin_popcountll((unsigned long long)x);
    
    /* __builtin_expect with __int128 comparison */
    __int128 a = ((__int128)HIGH_BIT_64 << 64);
    __int128 b = a - 1;
    
    if (__builtin_expect(a > b, 1)) {
        volatile int temp = leading_zero + trailing_zero + popcount;
        (void)temp;
    }
}

/* Overflow checking with __int128 */
static __int128 add_with_overflow(__int128 a, __int128 b, int *overflow) {
    __int128 result;
    *overflow = __builtin_add_overflow(a, b, &result);
    return result;
}

static unsigned __int128 rotate_left(unsigned __int128 x, int shift) {
    shift &= 127;
    return (x << shift) | (x >> (128 - shift));
}

static int compare_arrays(__int128 *a, __int128 *b, size_t n) {
    int checksum = 0;
    for (size_t i = 0; i < n; i++) {
        if (a[i] < b[i]) checksum -= 1;
        else if (a[i] > b[i]) checksum += 1;
        /* else equal, no change */
    }
    return checksum;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Array comparisons with values exercising high/low words */
    __int128 arr1[8], arr2[8];
    
    /* Initialize with values that differ in high words */
    for (int i = 0; i < 8; i++) {
        arr1[i] = ((__int128)(HIGH_BIT_64 + i) << 64) | i;
        arr2[i] = ((__int128)(HIGH_BIT_64 + i - 1) << 64) | (i + 1000);
    }
    
    checksum += compare_arrays(arr1, arr2, 8);
    
    /* Test 2: Values with equal high words but different low words */
    __int128 arr3[4], arr4[4];
    const __int128 common_high = (__int128)HIGH_BIT_64 << 63;
    
    for (int i = 0; i < 4; i++) {
        arr3[i] = common_high | (i * HIGH_BIT_64 / 4);
        arr4[i] = common_high | ((i + 1) * HIGH_BIT_64 / 4);
    }
    
    checksum += compare_arrays(arr3, arr4, 4);
    
    /* Test 3: Boundary values */
    __int128 boundaries[6];
    boundaries[0] = ((__int128)1 << 127) - 1;  /* INT128_MAX-like */
    boundaries[1] = -((__int128)1 << 127);     /* INT128_MIN-like */
    boundaries[2] = 0;
    boundaries[3] = (__int128)MAX_64;
    boundaries[4] = (__int128)MAX_64 + 1;
    boundaries[5] = -((__int128)MAX_64 + 1);
    
    /* Self-comparisons to exercise all paths */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (boundaries[i] < boundaries[j]) checksum--;
            else if (boundaries[i] > boundaries[j]) checksum++;
        }
    }
    
    /* Test 4: Overflow operations */
    int overflow;
    __int128 sum = add_with_overflow(boundaries[0], boundaries[3], &overflow);
    if (overflow) checksum += 1000;
    
    /* Test 5: Mixed signed/unsigned comparisons */
    unsigned __int128 u1 = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 u2 = u1 - 1;
    
    if (u1 > u2) checksum += 1;
    if ((__int128)u1 > (__int128)u2) checksum += 2;  /* Different comparison! */
    
    /* Run the other test suites */
    range_analysis_test();
    constant_folding_tests();
    mixed_precision_tests();
    builtin_tests();
    
    /* Final output to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    printf("Test completed - compile with:\n");
    printf("  gcc -O3 -fstrict-overflow -Wstrict-overflow=5 test.c\n");
    printf("  gcc -O2 -fdump-tree-all -fdump-rtl-all test.c\n");
    
    return checksum != 0 ? 0 : 1;
}
