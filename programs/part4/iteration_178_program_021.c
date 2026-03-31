/* fixed-value-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and inlining */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

/* Use __int128 if available for true double-int behavior */
#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MAX 127
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MAX 63
#endif

/* Core function that performs wide integer operations */
NOINLINE static wide_int_t wide_int_compute(int shift_amt, int modifier)
{
    /* Start with a value that can become negative when shifted */
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    /* Make shift amount bounded but non-constant */
    shift_amt = shift_amt & (WIDE_SHIFT_MAX - 1);
    
    /* Perform left shift - this creates a wide range of possible values */
    wide_int_t shifted = base << shift_amt;
    
    /* Add/subtract based on modifier to create signed range complexity */
    if (modifier & 1) {
        shifted += (wide_int_t)(modifier * 0x100000001LL);
    } else {
        shifted -= (wide_int_t)(modifier * 0x100000001LL);
    }
    
    return shifted;
}

/* Another function that creates unsigned comparisons */
NOINLINE static uwide_int_t unsigned_wide_compute(int shift_amt, int mask)
{
    /* Start with positive value */
    uwide_int_t base = (uwide_int_t)0xFEDCBA9876543210ULL;
    
    /* Bounded but non-constant shift */
    shift_amt = shift_amt & (WIDE_SHIFT_MAX / 2);
    
    /* Shift and mask to create unsigned range */
    uwide_int_t shifted = base << shift_amt;
    shifted = shifted & ((uwide_int_t)mask * 0xFFFFFFFFFFFFFFFFULL);
    
    return shifted;
}

int main(void)
{
    volatile int seed = 12345;  /* Prevent constant propagation */
    unsigned long long checksum = 0;
    
    /* Loop to create multiple analysis paths */
    for (int i = 0; i < 1000; i++) {
        /* Generate bounded but non-constant shift amounts */
        int shift1 = (seed + i * 3) & 31;
        int shift2 = (seed + i * 7) & 15;
        
        /* Call wide integer functions */
        wide_int_t signed_result = wide_int_compute(shift1, shift2);
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, shift1);
        
        /* 
         * KEY PART: Comparisons that should trigger fixed-value range analysis
         * These comparisons match the pattern in the uncovered lines
         */
        
        /* Signed comparison path - matches sgt comparison */
        if (signed_result > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            /* This should trigger max_r/min_s initialization for signed GT */
            checksum += 1;
        }
        
        /* Combined comparison with && - matches the uncovered pattern */
        if (signed_result > (wide_int_t)0x3FFFFFFFFFFFFFFFLL &&
            signed_result < (wide_int_t)0xBFFFFFFFFFFFFFFFLL) {
            checksum += 2;
        }
        
        /* Unsigned comparison path - matches ugt comparison */
        if (unsigned_result > (uwide_int_t)0xFFFFFFFFFFFFFFF0ULL) {
            checksum += 3;
        }
        
        /* Complex condition with both signed and unsigned aspects */
        wide_int_t temp = signed_result >> (shift1 & 7);
        if (temp > 0 || (temp == 0 && (uwide_int_t)unsigned_result > 1000)) {
            checksum += 5;
        }
        
        /* Update seed to vary inputs */
        seed = seed * 1103515245 + 12345;
        
        /* Additional complexity: nested conditions */
        if (i & 1) {
            int extra_shift = (seed >> 16) & 7;
            wide_int_t extra = signed_result << extra_shift;
            if (extra > (wide_int_t)(0x1LL << (WIDE_SHIFT_MAX - 1))) {
                checksum += 7;
            }
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
