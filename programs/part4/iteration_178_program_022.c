/* fixed-value-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure arithmetic isn't optimized away before analysis */
__attribute__((noinline)) 
long long wide_int_compute_signed(int shift_amt, int add_val) {
    /* Start with a value that can become negative when shifted */
    long long base = 0x123456789ABCDEF0LL;
    
    /* Non-constant shift - forces range analysis */
    long long shifted = base << shift_amt;
    
    /* Add/subtract based on parameter - creates multiple possible ranges */
    if (add_val > 0) {
        shifted += (long long)add_val * 0x1000LL;
    } else {
        shifted -= (long long)(-add_val) * 0x800LL;
    }
    
    return shifted;
}

__attribute__((noinline))
unsigned long long wide_int_compute_unsigned(int shift_amt, int mask_bits) {
    /* Start with positive value */
    unsigned long long base = 0xFEDCBA9876543210ULL;
    
    /* Non-constant shift */
    unsigned long long shifted = base << shift_amt;
    
    /* Apply mask to create bounded range */
    unsigned long long mask = (1ULL << mask_bits) - 1;
    shifted &= mask;
    
    return shifted;
}

#ifdef __SIZEOF_INT128__
__attribute__((noinline))
__int128 wide_int_128bit_compute(int shift_amt, int sign_adj) {
    /* Use 128-bit type for double-int analysis */
    __int128 val = (__int128)0x123456789ABCDEF0LL;
    
    /* Create a 128-bit value */
    val = (val << 64) | 0xFEDCBA9876543210LL;
    
    /* Non-constant shift on 128-bit */
    __int128 shifted = val << shift_amt;
    
    /* Conditional adjustment based on sign */
    if (sign_adj > 0) {
        shifted += (__int128)sign_adj * 0x100000000LL;
    } else {
        shifted -= (__int128)(-sign_adj) * 0x80000000LL;
    }
    
    return shifted;
}
#endif

int main(void) {
    volatile int seed = 42;  /* Prevent constant propagation */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        /* Generate bounded, non-constant values */
        int shift1 = (seed + i) & 63;      /* 0-63 bits for 64-bit shift */
        int shift2 = (seed * i) & 31;      /* 0-31 bits */
        int add_val = (seed ^ i) & 0xFF;   /* 0-255 */
        int mask_bits = (seed + i * 3) & 63; /* 0-63 bits for mask */
        
        /* PATH 1: Signed comparison with wide integer */
        long long result1 = wide_int_compute_signed(shift1, add_val - 128);
        
        /* This comparison should trigger signed range analysis */
        if (result1 > 0x7FFFFFFFFFFFFFFFLL) {
            /* Exceeds maximum positive signed 64-bit value */
            checksum += 1;
        } else if (result1 < -0x7FFFFFFFFFFFFFFFLL) {
            /* Very negative value */
            checksum += 2;
        }
        
        /* Mixed signed/unsigned comparison in conditional */
        if (result1 > 0 || (unsigned long long)result1 > 0xFFFFFFFF00000000ULL) {
            checksum += 4;
        }
        
        /* PATH 2: Unsigned comparison */
        unsigned long long result2 = wide_int_compute_unsigned(shift2, mask_bits);
        
        /* This should trigger unsigned range analysis */
        if (result2 > 0xFFFFFFFFFFFFFFFFULL) {
            checksum += 8;
        } else if (result2 > 0x7FFFFFFFFFFFFFFFULL) {
            /* In unsigned range but above signed max */
            checksum += 16;
        }
        
        /* Complex conditional with both signed and unsigned comparisons */
        if ((result1 > 0x3FFFFFFFFFFFFFFFLL && 
             (unsigned long long)result2 > 0xFFFFFFFFULL) ||
            (result1 < -0x4000000000000000LL && result2 < 0x8000000000000000ULL)) {
            checksum += 32;
        }
        
        #ifdef __SIZEOF_INT128__
        /* PATH 3: 128-bit operations for double-int analysis */
        __int128 result3 = wide_int_128bit_compute(shift1 & 127, add_val - 128);
        
        /* Signed comparison on 128-bit value */
        __int128 max_signed_128 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFLL;
        if (result3 > max_signed_128) {
            checksum += 64;
        }
        
        /* Mixed comparisons with 128-bit values */
        unsigned __int128 result3_unsigned = (unsigned __int128)result3;
        if (result3_unsigned > 0xFFFFFFFFFFFFFFFFULL &&
            result3 < 0) {
            /* This complex condition may trigger both signed and unsigned analysis */
            checksum += 128;
        }
        #endif
        
        /* Update seed for next iteration */
        seed = seed * 1103515245 + 12345;
        
        /* Prevent loop unrolling from simplifying everything */
        if (i % 100 == 0) {
            seed ^= checksum;
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
