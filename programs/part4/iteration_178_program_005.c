/* test-fixed-value.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure the analysis sees complex operations */
__attribute__((noinline)) 
long long wide_int_compute_signed(int shift_amt, int modifier) {
    /* Use volatile to prevent constant propagation */
    volatile long long base = 0x123456789ABCDEF0LL;
    long long result;
    
    /* Non-constant shift - forces range analysis */
    if (modifier & 1) {
        /* Path 1: Shift left with potential overflow */
        result = base << shift_amt;
    } else {
        /* Path 2: Different transformation */
        result = base >> (shift_amt & 0x1F);
    }
    
    /* Additional arithmetic to complicate analysis */
    if (modifier & 2) {
        result += (long long)modifier * 0x100000001LL;
    } else {
        result -= (long long)modifier * 0x0FFFFFFFFLL;
    }
    
    return result;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amt, int modifier) {
    volatile unsigned long long base = 0xFEDCBA9876543210ULL;
    unsigned long long result;
    
    /* Mixed shift types */
    if (modifier > 0) {
        result = base << (shift_amt & 0x3F);
    } else {
        result = base >> ((shift_amt + 1) & 0x3F);
    }
    
    /* Bitwise operations that affect range */
    result = result & (0xFFFFFFFFFFFFFFFFULL >> (modifier & 0x3F));
    
    return result;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_compute_128bit(int shift_amt, int modifier) {
    volatile __int128 base = ((__int128)0x123456789ABCDEF0LL << 64) | 0xFEDCBA9876543210LL;
    __int128 result;
    
    /* Complex 128-bit operations */
    if (shift_amt > 16) {
        result = base << (shift_amt & 0x7F);
    } else {
        result = base >> ((shift_amt + 32) & 0x7F);
    }
    
    /* Signed arithmetic that can produce negative values */
    if (modifier & 4) {
        result = -result;
    }
    
    return result;
}
#endif

int main() {
    volatile int seed = 42;  /* Non-constant initializer */
    unsigned long long checksum = 0;
    int i;
    
    for (i = 0; i < 1000; i++) {
        /* Generate bounded, non-constant shift amounts */
        int shift1 = (seed + i) & 0x3F;      /* 0-63 */
        int shift2 = (seed * i) & 0x1F;      /* 0-31 */
        int modifier = (seed ^ i) & 0x7;     /* 0-7 */
        
        /* 64-bit signed computations with comparisons */
        long long signed_result = wide_int_compute_signed(shift1, modifier);
        
        /* SIGNED comparison path - triggers sgt comparison */
        if (signed_result > 0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        /* Complex condition with multiple comparisons */
        if (signed_result > 0x3FFFFFFFFFFFFFFFLL && 
            signed_result < 0xBFFFFFFFFFFFFFFFLL) {
            checksum += 2;
        }
        
        /* 64-bit unsigned computations */
        unsigned long long unsigned_result = 
            wide_int_compute_unsigned(shift2, modifier);
        
        /* UNSIGNED comparison path - triggers ugt comparison */
        if (unsigned_result > 0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 4;
        }
        
        /* Mixed signed/unsigned comparison */
        if ((long long)unsigned_result > signed_result) {
            checksum += 8;
        }
        
#ifdef __SIZEOF_INT128__
        /* 128-bit operations for double-int analysis */
        __int128 result_128 = wide_int_compute_128bit(shift1, modifier);
        
        /* 128-bit signed comparison */
        if (result_128 > ((__int128)0x7FFFFFFFFFFFFFFFLL << 64)) {
            checksum += 16;
        }
        
        /* 128-bit unsigned comparison */
        if ((unsigned __int128)result_128 > 
            ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64)) {
            checksum += 32;
        }
        
        /* Complex condition mixing 128-bit operations */
        __int128 shifted_128 = result_128 << (shift2 & 0x7F);
        if (shifted_128 > 0 && shifted_128 < ((__int128)1 << 120)) {
            checksum += 64;
        }
#endif
        
        /* Update seed to vary inputs */
        seed = seed * 1103515245 + 12345;
        
        /* Additional control flow to complicate analysis */
        if (i % 100 == 0) {
            modifier = (modifier + 1) & 0x3;
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
