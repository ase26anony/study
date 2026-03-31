#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_MAX (((wide_int_t)1 << 127) - 1)
#define UNSIGNED_MAX ((uwide_int_t)-1)
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_MAX 0x7FFFFFFFFFFFFFFFLL
#define UNSIGNED_MAX 0xFFFFFFFFFFFFFFFFULL
#endif

/* Prevent inlining to ensure fixed-value analysis runs on the operations */
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    /* Start with a value that can become negative when shifted */
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    /* Non-constant shift - forces range analysis */
    wide_int_t shifted = base << shift_amount;
    
    /* Add a modifier that can make the result negative */
    wide_int_t result = shifted + (wide_int_t)modifier;
    
    return result;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int mask_bits) {
    /* Start with a positive value */
    uwide_int_t base = 0xFEDCBA9876543210ULL;
    
    /* Shift by non-constant amount */
    uwide_int_t shifted = base << shift_amount;
    
    /* Apply mask with non-constant bits */
    uwide_int_t mask = ((uwide_int_t)1 << mask_bits) - 1;
    uwide_int_t result = shifted & mask;
    
    return result;
}

int main() {
    volatile int seed = 42; /* volatile to prevent constant propagation */
    uint64_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        /* Generate bounded, non-constant shift amounts */
        int shift1 = (seed + i * 3) & 63;  /* 0-63 bits */
        int shift2 = (seed + i * 5) & 31;  /* 0-31 bits */
        int mod1 = (seed + i * 7) & 0xFF;  /* 0-255 */
        int mod2 = (seed + i * 11) & 0x1F; /* 0-31 */
        
        /* Path 1: Signed comparison with potential negative values */
        wide_int_t signed_result = wide_int_compute(shift1, mod1 - 128);
        
        /* This comparison should trigger signed range analysis */
        if (signed_result > WIDE_MAX) {
            checksum += 1;
        } else if (signed_result < -WIDE_MAX) {
            checksum += 2;
        }
        
        /* Complex condition with mixed comparisons */
        if (signed_result > 0 || (signed_result == 0 && signed_result < 0)) {
            /* Impossible condition, but forces analysis of both bounds */
            checksum += 3;
        }
        
        /* Path 2: Unsigned comparison */
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, mod2);
        
        /* This comparison should trigger unsigned range analysis */
        if (unsigned_result > UNSIGNED_MAX >> 1) {
            checksum += 4;
        }
        
        /* Mixed signed/unsigned comparison in complex condition */
        if ((wide_int_t)unsigned_result > signed_result && 
            unsigned_result > 0x8000000000000000ULL) {
            checksum += 5;
        }
        
        /* Update seed with pseudo-random sequence */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
