/* fixed-value-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure arithmetic isn't optimized away before analysis */
__attribute__((noinline))
__int128 wide_int_compute(int shift_amount, int modifier, int use_signed) {
    /* Start with a value that can become negative when shifted */
    __int128 base = (__int128)0x123456789ABCDEF0LL;
    
    /* Non-constant shift - forces range analysis */
    __int128 shifted = base << shift_amount;
    
    /* Conditional arithmetic to create analysis complexity */
    if (use_signed & 1) {
        /* Path 1: May produce negative values */
        shifted = shifted - ((__int128)modifier << 32);
    } else {
        /* Path 2: Keep values positive */
        shifted = shifted & (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL);
    }
    
    return shifted;
}

__attribute__((noinline))
long long wide_ll_compute(int shift_amount, int modifier, int use_signed) {
    /* Similar logic with 64-bit long long */
    long long base = 0x123456789ABCDEF0LL;
    
    /* Variable shift creates range analysis requirements */
    long long shifted = base << shift_amount;
    
    /* Mixed signed/unsigned operations */
    if (use_signed & 1) {
        shifted = shifted - ((long long)modifier << 16);
    } else {
        shifted = shifted & 0x7FFFFFFFFFFFFFFFLL; /* Ensure non-negative */
    }
    
    return shifted;
}

int main(void) {
    volatile int seed = 12345; /* volatile prevents constant propagation */
    uint64_t checksum = 0;
    
    /* Loop to explore different shift amounts and conditions */
    for (int i = 0; i < 1000; i++) {
        /* Bounded shift amounts (0-63 for __int128, 0-31 for long long) */
        int shift1 = (seed + i) & 63;      /* For 128-bit shifts */
        int shift2 = (seed + i * 3) & 31;  /* For 64-bit shifts */
        int mod1 = (seed ^ i) & 0xFF;
        int mod2 = (seed * i) & 0xFF;
        
        /* Compute with __int128 - triggers double-int analysis */
        __int128 result128 = wide_int_compute(shift1, mod1, i & 1);
        
        /* Complex comparisons that require range analysis */
        
        /* 1. Signed comparison path (sgt) */
        if (result128 > (__int128)0x7FFFFFFFFFFFFFFFLL) {
            checksum += 1;
        }
        
        /* 2. Mixed signed/unsigned comparisons with AND condition */
        if (result128 > (__int128)0x3FFFFFFFFFFFFFFFLL &&
            (unsigned __int128)result128 > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 2;
        }
        
        /* 3. Equality check with unsigned comparison (ugt) */
        __int128 max_r_val = (__int128)0x7FFFFFFFFFFFFFFFLL;
        if (result128 == max_r_val) {
            /* This should trigger the a_high == max_r && a_low.ugt(max_s) path */
            checksum += 4;
        }
        
        /* Compute with long long - also triggers range analysis */
        long long result64 = wide_ll_compute(shift2, mod2, i & 2);
        
        /* More comparisons with 64-bit values */
        if (result64 > 0x3FFFFFFFFFFFFFFFLL) {
            checksum += 8;
        }
        
        if ((unsigned long long)result64 > 0xFFFFFFFFULL) {
            checksum += 16;
        }
        
        /* Update seed to vary inputs */
        seed = seed * 1103515245 + 12345;
        
        /* Additional complexity: nested conditions affecting shift amounts */
        if (checksum & 1) {
            shift1 = (shift1 + 1) & 63;
            result128 = wide_int_compute(shift1, mod1, i & 3);
            
            /* Another comparison after recomputation */
            if (result128 < (__int128)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
                checksum += 32;
            }
        }
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
