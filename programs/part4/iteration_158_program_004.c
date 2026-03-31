/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force compiler to use wide integer operations */
typedef unsigned __int128 u128;
typedef __int128 s128;

/* Volatile sink to prevent optimization */
volatile int sink = 0;

/* Function that compares wide integers and returns comparison result */
static int compare_u128(u128 a, u128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static int compare_s128(s128 a, s128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Test case 1: High part comparisons (unsigned) */
static void test_high_part_comparisons(void) {
    /* Case 1a: High part less, low part arbitrary */
    u128 a1 = ((u128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    u128 b1 = ((u128)0xFEDCBA9876543210ULL << 64) | 0x0000000000000000ULL;
    sink += compare_u128(a1, b1);  /* Should return -1 */
    
    /* Case 1b: High part greater, low part arbitrary */
    u128 a2 = ((u128)0xFEDCBA9876543210ULL << 64) | 0x0000000000000000ULL;
    u128 b2 = ((u128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    sink += compare_u128(a2, b2);  /* Should return 1 */
    
    /* Case 1c: High part equal, low part less */
    u128 a3 = ((u128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL;
    u128 b3 = ((u128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    sink += compare_u128(a3, b3);  /* Should return -1 */
    
    /* Case 1d: High part equal, low part greater */
    u128 a4 = ((u128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    u128 b4 = ((u128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL;
    sink += compare_u128(a4, b4);  /* Should return 1 */
    
    /* Case 1e: Complete equality */
    u128 a5 = ((u128)0x123456789ABCDEF0ULL << 64) | 0x5555555555555555ULL;
    u128 b5 = ((u128)0x123456789ABCDEF0ULL << 64) | 0x5555555555555555ULL;
    sink += compare_u128(a5, b5);  /* Should return 0 */
}

/* Test case 2: Mixed signed/unsigned comparisons */
static void test_mixed_signed_unsigned(void) {
    /* Signed negative vs unsigned large */
    s128 s1 = -1;  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF (two's complement) */
    u128 u1 = 0xFFFFFFFFFFFFFFFFULL;  /* Only low 64 bits set */
    
    /* This should trigger unsigned comparison of high parts */
    if (s1 < u1) {
        sink += 100;  /* Likely taken due to unsigned comparison */
    }
    
    /* Explicit comparison with casting */
    sink += compare_s128(s1, (s128)u1);
    sink += compare_u128((u128)s1, u1);
}

/* Test case 3: Arithmetic operations that require constant folding */
static void test_constant_folding(void) {
    /* Large constants that require 128-bit representation */
    const u128 c1 = ((u128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    const u128 c2 = ((u128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* Arithmetic that might be constant-folded */
    u128 sum = c1 + c2;
    u128 diff = c1 - c2;
    u128 prod = c1 * 2;
    
    /* Comparisons that might be evaluated at compile time */
    if (c1 < c2) sink += 1;
    if (sum > diff) sink += 2;
    if (prod == (c1 << 1)) sink += 4;
    
    /* Ternary operator with wide comparison */
    u128 result = (c1 < c2) ? c1 : c2;
    sink += (int)(result >> 120);  /* Use some bits */
}

/* Test case 4: Array indexing with wide integers */
static void test_array_indexing(void) {
    /* Small array for bounds checking */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Loop with wide counter (but small range to avoid UB) */
    for (u128 i = 0; i < 10; i++) {
        /* Comparison in loop condition */
        arr[(int)i] += sink;
    }
    
    /* Wide index with bounds check */
    u128 idx = 5;
    if (idx < 10) {
        sink += arr[(int)idx];
    }
}

/* Test case 5: Compiler builtins */
static void test_builtins(void) {
    u128 a = ((u128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    u128 b = 1;
    u128 result;
    
    /* Overflow checking builtins */
    if (__builtin_add_overflow(a, b, &result)) {
        sink += 10;  /* Should trigger - addition overflows to 0 */
    }
    
    if (__builtin_mul_overflow(a, 2, &result)) {
        sink += 20;  /* Should trigger */
    }
    
    /* Count trailing zeros on wide integer */
    u128 x = 0x10000000000000000ULL;  /* 2^64 */
    int ctz = __builtin_ctzll((unsigned long long)(x >> 64));
    sink += ctz;
}

/* Test case 6: Switch statement with wide constants */
static void test_switch(void) {
    u128 val = ((u128)0x12345678ULL << 64) | 0x9ABCDEF0ULL;
    int result = 0;
    
    /* Switch that requires comparison with wide constants */
    if (val < ((u128)0x100000000ULL << 64)) {
        result = 1;
    } else if (val < ((u128)0x200000000ULL << 64)) {
        result = 2;
    } else if (val < ((u128)0x300000000ULL << 64)) {
        result = 3;
    } else {
        result = 4;
    }
    
    sink += result;
}

/* Test case 7: Complex expressions with comparisons */
static void test_complex_expressions(void) {
    /* Create values where high parts differ by 1 */
    u128 x = ((u128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL;
    u128 y = ((u128)0x123456789ABCDEEFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* Chain of comparisons */
    int cmp1 = (x < y) ? -1 : (x > y) ? 1 : 0;
    int cmp2 = (y < x) ? -1 : (y > x) ? 1 : 0;
    
    /* Arithmetic that affects comparison */
    u128 z = x + 1;
    int cmp3 = (z > x) ? 1 : (z < x) ? -1 : 0;
    
    sink += cmp1 + cmp2 + cmp3;
}

int main(void) {
    /* Run all test cases */
    test_high_part_comparisons();
    test_mixed_signed_unsigned();
    test_constant_folding();
    test_array_indexing();
    test_builtins();
    test_switch();
    test_complex_expressions();
    
    /* Print sink to prevent dead code elimination */
    printf("Result: %d\n", sink);
    
    return 0;
}
