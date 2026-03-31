/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */

#include <stdio.h>
#include <stdint.h>

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function to force comparisons */
int compare_128(__int128 a, __int128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int compare_u128(unsigned __int128 a, unsigned __int128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Test case 1: High part comparisons (unsigned) */
void test_high_part_comparisons(void) {
    /* High part less */
    unsigned __int128 u1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    unsigned __int128 u2 = ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x2222222222222222ULL;
    
    /* High part greater */
    unsigned __int128 u3 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x3333333333333333ULL;
    unsigned __int128 u4 = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x4444444444444444ULL;
    
    /* High part equal, low part different */
    unsigned __int128 u5 = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    unsigned __int128 u6 = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xBBBBBBBBBBBBBBBBULL;
    
    /* High part equal, low part less */
    unsigned __int128 u7 = ((unsigned __int128)0x6666666666666666ULL << 64) | 0x1111111111111111ULL;
    unsigned __int128 u8 = ((unsigned __int128)0x6666666666666666ULL << 64) | 0x2222222222222222ULL;
    
    /* High part equal, low part greater */
    unsigned __int128 u9 = ((unsigned __int128)0x7777777777777777ULL << 64) | 0x9999999999999999ULL;
    unsigned __int128 u10 = ((unsigned __int128)0x7777777777777777ULL << 64) | 0x8888888888888888ULL;
    
    /* Equal values */
    unsigned __int128 u11 = ((unsigned __int128)0x8888888888888888ULL << 64) | 0xCCCCCCCCCCCCCCCCULL;
    unsigned __int128 u12 = u11;
    
    sink = compare_u128(u1, u2);  /* high < high */
    sink = compare_u128(u3, u4);  /* high > high */
    sink = compare_u128(u5, u6);  /* high = high, low < low */
    sink = compare_u128(u7, u8);  /* high = high, low < low */
    sink = compare_u128(u9, u10); /* high = high, low > low */
    sink = compare_u128(u11, u12); /* equal */
}

/* Test case 2: Mixed signed/unsigned comparisons */
void test_mixed_comparisons(void) {
    /* Signed negative vs unsigned large */
    __int128 s1 = -1;  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF (signed) */
    unsigned __int128 u1 = 0xFFFFFFFFFFFFFFFFULL;  /* Only low part set */
    
    /* Signed positive vs unsigned */
    __int128 s2 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 u2 = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    /* Same bit pattern, different interpretation */
    unsigned __int128 u3 = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    __int128 s3 = (__int128)u3;  /* Negative in signed interpretation */
    
    /* Ternary operator with wide comparison */
    __int128 result1 = (s1 < (__int128)u1) ? s1 : u1;
    unsigned __int128 result2 = ((unsigned __int128)s2 > u2) ? s2 : u2;
    
    sink = (int)result1;
    sink = (int)result2;
    
    /* Direct comparison with mixed types */
    if (s3 < u3) {
        sink = 1;  /* Should trigger unsigned comparison */
    }
}

/* Test case 3: Array indexing with wide integers */
void test_array_indexing(void) {
    /* Small array for bounds checking */
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Wide loop counter */
    for (unsigned __int128 i = 0; i < 10; i++) {
        if (i < 10) {  /* Comparison in loop condition */
            sink = arr[(int)i];
        }
    }
    
    /* Wide index with bounds check */
    unsigned __int128 idx = 5;
    if (idx < 10) {
        sink = arr[(int)idx];
    }
}

/* Test case 4: Compiler builtins */
void test_builtins(void) {
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b = 1;
    __int128 result;
    
    /* Overflow checking */
    if (__builtin_add_overflow(a, b, &result)) {
        sink = 1;  /* Overflow path */
    }
    
    if (__builtin_mul_overflow(a, b, &result)) {
        sink = 2;
    }
    
    /* Bit counting */
    unsigned __int128 u = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    sink = __builtin_clzg(u, 0);  /* Count leading zeros */
    sink = __builtin_ctzg(u, 0);  /* Count trailing zeros */
}

/* Test case 5: Switch statement with wide constants */
void test_switch(void) {
    unsigned __int128 val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    /* Switch with wide constant cases */
    switch (val) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            sink = 100;
            break;
        case ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL:
            sink = 200;
            break;
        default:
            sink = 300;
    }
}

/* Test case 6: Constant folding scenarios */
void test_constant_folding(void) {
    /* Compile-time constant expressions */
    const __int128 c1 = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    const __int128 c2 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 c3 = c1 + c2;
    const __int128 c4 = c1 - c2;
    const __int128 c5 = c1 * 2;
    
    /* Comparisons that should fold at compile time */
    const int cmp1 = (c1 < c2) ? -1 : ((c1 > c2) ? 1 : 0);
    const int cmp2 = (c3 < c4) ? -1 : ((c3 > c4) ? 1 : 0);
    const int cmp3 = (c5 < c1) ? -1 : ((c5 > c1) ? 1 : 0);
    
    sink = cmp1 + cmp2 + cmp3;
    
    /* Shift operations */
    const __int128 c6 = c1 << 3;
    const __int128 c7 = c2 >> 5;
    const int cmp4 = (c6 < c7) ? -1 : ((c6 > c7) ? 1 : 0);
    
    sink = cmp4;
}

/* Test case 7: Complex arithmetic expressions */
void test_complex_expressions(void) {
    unsigned __int128 x = ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL;
    unsigned __int128 y = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    
    /* Complex expression with multiple comparisons */
    unsigned __int128 z = (x < y) ? (x + y) : (x - y);
    unsigned __int128 w = (x > y) ? (x * 2) : (y * 3);
    
    /* Nested comparisons */
    if ((x < z) && (z < w)) {
        sink = 1;
    } else if ((x > z) || (z > w)) {
        sink = 2;
    } else {
        sink = 3;
    }
    
    /* Comparison in arithmetic context */
    unsigned __int128 diff = (x > y) ? (x - y) : (y - x);
    sink = (int)diff;
}

int main(void) {
    /* Execute all test cases */
    test_high_part_comparisons();
    test_mixed_comparisons();
    test_array_indexing();
    test_builtins();
    test_switch();
    test_constant_folding();
    test_complex_expressions();
    
    /* Print something to prevent complete optimization */
    printf("Coverage test completed. Sink value: %d\n", sink);
    
    return 0;
}
