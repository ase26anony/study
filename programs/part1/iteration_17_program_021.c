/* test-double-int.c */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Force constant folding with __int128 comparisons */
static const __int128 NEG_BIG = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;  /* 0x80000000000000008000000000000000 */
static const __int128 POS_BIG = ((__int128)MAX_64 << 64) | MAX_64;           /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
static const __int128 MID_POS = ((__int128)MID_128 << 64) | MID_128;
static const unsigned __int128 UMAX = ((unsigned __int128)MAX_64 << 64) | MAX_64;

/* Test 1: Constant folding comparisons that exercise high-word logic */
static int test_constant_folding(void) {
    int result = 0;
    
    /* These should force compile-time comparisons */
    if (NEG_BIG < -NEG_BIG) result |= 1;
    if (POS_BIG > MID_POS) result |= 2;
    if ((__int128)0 > NEG_BIG) result |= 4;
    if (UMAX > (unsigned __int128)POS_BIG) result |= 8;
    
    /* Cross-type comparisons */
    if ((__int128)LONG_MAX < MID_POS) result |= 16;
    if ((unsigned __int128)ULONG_MAX < UMAX) result |= 32;
    
    return result;
}

/* Test 2: Range analysis with loops and __int128 induction */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundary */
    for (__int128 i = HIGH_BIT_64 - 10; i < HIGH_BIT_64 + 10; i++) {
        if (i < HIGH_BIT_64) checksum++;
        else checksum--;
        
        /* Force range analysis with conditional */
        __int128 j = i * 2;
        if (j > HIGH_BIT_64 << 1) checksum ^= (int)j;
    }
    
    /* Another loop crossing zero boundary */
    for (__int128 i = -5; i <= 5; i++) {
        __int128 val = i * HIGH_BIT_64;
        if (val < 0) checksum += 1;
        else if (val > 0) checksum += 2;
        else checksum += 3;
    }
    
    return checksum;
}

/* Test 3: Overflow checking with builtins */
static int test_overflow_checks(void) {
    int result = 0;
    __int128 a, b;
    int overflow;
    
    /* Test near overflow boundaries */
    a = ((__int128)MAX_64 << 32) | MAX_64;
    b = a;
    
    overflow = __builtin_add_overflow(a, b, &a);
    if (overflow) result |= 1;
    
    overflow = __builtin_mul_overflow(a, (__int128)2, &b);
    if (overflow) result |= 2;
    
    /* Test with negative values */
    a = NEG_BIG;
    overflow = __builtin_sub_overflow(a, (__int128)1, &b);
    if (overflow) result |= 4;
    
    return result;
}

/* Test 4: Bitwise operations crossing 64-bit boundary */
static int test_bitwise_ops(void) {
    unsigned __int128 mask1 = ((unsigned __int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    unsigned __int128 mask2 = ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL;
    unsigned __int128 val = UMAX;
    int result = 0;
    
    /* Shifts crossing word boundary */
    val = val >> 65;
    if (val < (UMAX >> 1)) result |= 1;
    
    val = mask1 << 70;
    if (val > mask1) result |= 2;
    
    /* Bitwise operations */
    val = mask1 & mask2;
    if (val != 0) result |= 4;
    
    val = mask1 | mask2;
    if (val == UMAX) result |= 8;
    
    return result;
}

/* Test 5: Switch statement with __int128 case labels */
static int test_switch(__int128 value) {
    switch (value) {
        case ((__int128)0x1000000000000000ULL << 64):
            return 1;
        case ((__int128)0x2000000000000000ULL << 64) | 0x1:
            return 2;
        case NEG_BIG:
            return 3;
        case MID_POS:
            return 4;
        default:
            return (value > 0) ? 5 : 6;
    }
}

/* Test 6: Mixed precision and conversions */
static int test_mixed_precision(void) {
    int result = 0;
    
    /* Ternary with mixed types */
    __int128 x = (sizeof(void*) == 8) ? 
                 ((__int128)LONG_MAX << 32) : 
                 ((__int128)INT_MAX << 32);
    
    /* Comparisons with different types */
    if (x > LLONG_MAX) result |= 1;
    if (x < (__int128)ULLONG_MAX) result |= 2;
    
    /* Array operations with __int128 */
    __int128 arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = ((__int128)i << (i * 8)) | i;
        if (i > 0 && arr[i] > arr[i-1]) result ^= (1 << i);
    }
    
    return result;
}

/* Test 7: Builtin functions with wide integers */
static int test_builtins(void) {
    unsigned __int128 x = UMAX;
    int result = 0;
    
    /* Count leading zeros - may trigger internal comparisons */
    if (__builtin_clzll((unsigned long long)(x >> 64)) == 0) result |= 1;
    
    /* Byte swap simulation */
    unsigned __int128 y = ((x & 0xFF) << 120) |
                         (((x >> 8) & 0xFF) << 112) |
                         (((x >> 16) & 0xFF) << 104) |
                         (((x >> 24) & 0xFF) << 96) |
                         (((x >> 32) & 0xFF) << 88) |
                         (((x >> 40) & 0xFF) << 80) |
                         (((x >> 48) & 0xFF) << 72) |
                         (((x >> 56) & 0xFF) << 64) |
                         (((x >> 64) & 0xFF) << 56) |
                         (((x >> 72) & 0xFF) << 48) |
                         (((x >> 80) & 0xFF) << 40) |
                         (((x >> 88) & 0xFF) << 32) |
                         (((x >> 96) & 0xFF) << 24) |
                         (((x >> 104) & 0xFF) << 16) |
                         (((x >> 112) & 0xFF) << 8) |
                         ((x >> 120) & 0xFF);
    
    if (y < x) result |= 2;
    
    /* Use __builtin_expect with __int128 comparison */
    __int128 a = MID_POS;
    __int128 b = MID_POS + 1;
    if (__builtin_expect(a < b, 1)) result |= 4;
    
    return result;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    printf("Testing double_int comparison paths...\n");
    
    /* Run all tests to exercise different comparison scenarios */
    total += test_constant_folding();
    printf("Constant folding test: %d\n", total);
    
    total += test_range_analysis();
    printf("Range analysis test: %d\n", total);
    
    total += test_overflow_checks();
    printf("Overflow check test: %d\n", total);
    
    total += test_bitwise_ops();
    printf("Bitwise ops test: %d\n", total);
    
    /* Test switch with different values */
    total += test_switch(((__int128)0x1000000000000000ULL << 64));
    total += test_switch(NEG_BIG);
    total += test_switch(MID_POS);
    total += test_switch(0);
    printf("Switch test: %d\n", total);
    
    total += test_mixed_precision();
    printf("Mixed precision test: %d\n", total);
    
    total += test_builtins();
    printf("Builtins test: %d\n", total);
    
    /* Final checksum to prevent dead code elimination */
    volatile int final_result = total;
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}

/* Force compile-time evaluation with static assertions */
_Static_assert(((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 
               (unsigned __int128)0xFFFFFFFFFFFFFFFFULL, 
               "128-bit comparison should work");

_Static_assert(((__int128)0x8000000000000000ULL << 64) < 
               (__int128)0x7FFFFFFFFFFFFFFFULL << 64, 
               "Signed 128-bit comparison should work");

/* Additional compile-time tests using #if */
#if ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 0xFFFFFFFFFFFFFFFFULL
/* This should be true */
#endif

#if ((__int128)0x8000000000000000ULL << 64) < 0
/* This should be true for signed comparison */
#endif
