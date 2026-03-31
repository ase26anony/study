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

/* Mixed signed/unsigned comparison */
int compare_mixed(__int128 a, unsigned __int128 b) {
    return (a < b) ? -1 : ((a > b) ? 1 : 0);
}

int main(void) {
    /* 1. Wide Integer Constant Expressions */
    /* Large constants with non-zero high parts */
    const __int128 big1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 big2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0123456789ABCDEFULL;
    const __int128 big3 = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const __int128 big4 = ((__int128)0x1ULL << 64) | 0x0ULL;
    
    const unsigned __int128 ubig1 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const unsigned __int128 ubig2 = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0ULL;
    const unsigned __int128 ubig3 = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* 2. Control Flow Dependent on Wide Comparisons */
    /* Test all comparison outcomes */
    
    /* Case 1: High part less */
    if (big3 < big4) {
        sink = 1;  /* Should be taken */
    }
    
    /* Case 2: High part greater */
    if (big4 > big3) {
        sink = 2;  /* Should be taken */
    }
    
    /* Case 3: High part equal, low part less */
    if (big2 < big1) {
        sink = 3;  /* Should be taken (0x0123... < 0xFEDC...) */
    }
    
    /* Case 4: High part equal, low part greater */
    if (big1 > big2) {
        sink = 4;  /* Should be taken */
    }
    
    /* Case 5: Equality */
    if (big1 == big1) {
        sink = 5;  /* Should be taken */
    }
    
    /* 3. Array Indexing with Wide Indices */
    char arr[256];
    for (unsigned __int128 i = 0; i < 100; i++) {
        arr[i % 256] = (char)i;
    }
    
    /* 4. Mixed Signed/Unsigned Contexts */
    __int128 sval = -1;  /* All bits set in two's complement */
    unsigned __int128 uval = 0xFFFFFFFFFFFFFFFFULL;
    
    /* This will trigger unsigned comparison after conversion */
    if (sval < uval) {
        sink = 6;  /* Behavior depends on conversion rules */
    }
    
    /* Explicit mixed comparison */
    int mixed_result = compare_mixed(sval, uval);
    sink = mixed_result;
    
    /* 5. Compiler Builtins and Intrinsics */
    __int128 a = big1;
    __int128 b = big2;
    __int128 result;
    
    /* Overflow checking */
    if (__builtin_add_overflow(a, b, &result)) {
        sink = 7;
    }
    
    if (__builtin_mul_overflow(a, big3, &result)) {
        sink = 8;
    }
    
    /* Bit operations */
    unsigned __int128 x = ubig1;
    int leading_zeros = __builtin_clzg(x, 0);
    sink = leading_zeros;
    
    /* 6. Switch statement with wide constants */
    unsigned __int128 switch_val = ubig2;
    switch (switch_val) {
        case ((unsigned __int128)0x8000000000000000ULL << 64):
            sink = 100;
            break;
        case ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL:
            sink = 101;
            break;
        default:
            sink = 102;
    }
    
    /* 7. Arithmetic operations that create comparison opportunities */
    __int128 sum = big1 + big2;
    __int128 diff = big1 - big2;
    __int128 prod = big3 * big4;
    
    if (sum > diff) {
        sink = 9;
    }
    
    if (prod < big1) {
        sink = 10;
    }
    
    /* 8. Function calls to force comparisons */
    int cmp1 = compare_128(big1, big2);  /* high equal, low: big1 > big2 */
    int cmp2 = compare_128(big3, big4);  /* high: big3 < big4 */
    int cmp3 = compare_128(big4, big3);  /* high: big4 > big3 */
    int cmp4 = compare_128(big1, big1);  /* equal */
    
    int cmp5 = compare_u128(ubig1, ubig2);  /* ubig1 > ubig2 */
    int cmp6 = compare_u128(ubig2, ubig3);  /* ubig2 > ubig3 (0x8000... > 0x7FFF...) */
    
    /* Aggregate results to prevent elimination */
    sink = cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    
    /* Print something to ensure execution */
    printf("Test completed. Sink value: %d\n", sink);
    
    return 0;
}
