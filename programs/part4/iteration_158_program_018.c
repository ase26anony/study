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
    const __int128 max_64 = 0xFFFFFFFFFFFFFFFFULL;
    const __int128 big1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 big2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0123456789ABCDEFULL;
    const __int128 big3 = ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 big4 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    const unsigned __int128 ubig1 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const unsigned __int128 ubig2 = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    /* 2. Control Flow Dependent on Wide Comparisons */
    /* Test high part less */
    if (big1 < big3) {
        sink = 1;  /* Should be taken */
    }
    
    /* Test high part greater */
    if (big3 > big1) {
        sink = 2;  /* Should be taken */
    }
    
    /* Test low part less */
    if (big2 < big1) {
        sink = 3;  /* Should be taken */
    }
    
    /* Test low part greater */
    if (big1 > big2) {
        sink = 4;  /* Should be taken */
    }
    
    /* Test equality */
    if (big1 == big4) {
        sink = 5;  /* Should be taken */
    }
    
    /* 3. Array Indexing with Wide Indices */
    char arr[256] = {0};
    __int128 idx = 100;
    
    /* Bounds check with wide integer */
    if (idx >= 0 && idx < 256) {
        arr[idx] = 42;
    }
    
    /* Loop with wide counter */
    for (__int128 i = 0; i < 10; i++) {
        if (i >= 0 && i < 256) {
            arr[i] = (char)i;
        }
    }
    
    /* 4. Mixed Signed/Unsigned Contexts */
    __int128 sval = -1;  /* All bits set in two's complement */
    unsigned __int128 uval = ubig2;
    
    /* This comparison will use unsigned comparison for high parts */
    if (compare_mixed(sval, uval)) {
        sink = 6;
    }
    
    /* Ternary operator with wide comparison */
    __int128 result = (big1 < big2) ? big1 : big2;
    sink = (int)result;
    
    /* 5. Compiler Builtins and Intrinsics */
    __int128 a = big1;
    __int128 b = big2;
    __int128 sum;
    
    /* Overflow check forces comparison */
    if (__builtin_add_overflow(a, b, &sum)) {
        sink = 7;
    }
    
    /* Bit operations */
    unsigned __int128 x = ubig1;
    int leading_zeros = __builtin_clzg(x, 0);
    sink = leading_zeros;
    
    /* Switch statement with wide constants (GCC extension) */
    __int128 switch_val = big1;
    
    /* Note: GCC doesn't directly support __int128 in switch,
       but we can use if-else chain to simulate */
    if (switch_val == big1) {
        sink = 100;
    } else if (switch_val == big2) {
        sink = 101;
    } else if (switch_val == big3) {
        sink = 102;
    }
    
    /* Function calls to force comparisons */
    sink += compare_128(big1, big2);
    sink += compare_128(big3, big1);
    sink += compare_128(big1, big4);
    
    sink += compare_u128(ubig1, ubig2);
    sink += compare_u128(ubig2, ubig1);
    sink += compare_u128(ubig1, ubig1);
    
    /* Arithmetic that creates new wide values for comparison */
    __int128 prod = big1 * 2;
    __int128 diff = big3 - big1;
    
    if (prod > diff) {
        sink = 8;
    }
    
    /* Shift operations */
    __int128 shifted = big1 >> 64;
    if (shifted > 0) {
        sink = 9;
    }
    
    /* Print something to prevent complete optimization */
    printf("Result: %d\n", sink);
    
    return 0;
}
