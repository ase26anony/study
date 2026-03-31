#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_MAX ((__int128)(((unsigned __int128)1 << 127) - 1))
#define U_WIDE_MAX ((unsigned __int128)-1)
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_MAX 0x7FFFFFFFFFFFFFFFLL
#define U_WIDE_MAX 0xFFFFFFFFFFFFFFFFULL
#endif

/* Prevent inlining to ensure fixed-value analysis runs */
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    /* Start with a value that can become negative when shifted */
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    /* Non-constant shift - forces range analysis */
    wide_int_t shifted = base << shift_amount;
    
    /* Add a modifier that can make the value negative */
    shifted += (wide_int_t)modifier * 0x100000001LL;
    
    return shifted;
}

__attribute__((noinline))
int check_signed_range(wide_int_t value, int i_f_bits) {
    /* This comparison should trigger signed range analysis */
    if (value > (WIDE_MAX >> i_f_bits)) {
        return 1;
    }
    return 0;
}

__attribute__((noinline))
int check_unsigned_range(uwide_int_t value, int i_f_bits) {
    /* This comparison should trigger unsigned range analysis */
    if (value > (U_WIDE_MAX >> i_f_bits)) {
        return 1;
    }
    return 0;
}

int main() {
    volatile int seed = 12345;  /* Prevent constant propagation */
    unsigned long long checksum = 0;
    
    /* Loop to generate multiple analysis paths */
    for (int i = 0; i < 1000; i++) {
        /* Create bounded, non-constant shift amounts */
        int shift1 = (seed + i) & 63;      /* 0-63 bits */
        int shift2 = (seed * i) & 31;      /* 0-31 bits */
        int mod = (seed + i * 3) & 0xFF;   /* 0-255 */
        
        /* Compute wide integer with complex transformations */
        wide_int_t val1 = wide_int_compute(shift1, mod);
        wide_int_t val2 = wide_int_compute(shift2, mod ^ 0x55);
        
        /* Mixed signed/unsigned comparisons in conditional expressions */
        
        /* Path 1: Signed comparison that may trigger max_r/max_s initialization */
        if (val1 > (WIDE_MAX >> (shift1 & 7)) || 
            (val1 == (WIDE_MAX >> (shift1 & 7)) && 
             (uwide_int_t)val2 > (U_WIDE_MAX >> (shift2 & 7)))) {
            checksum += 1;
        }
        
        /* Path 2: Another combination to increase analysis complexity */
        if (val2 < (-WIDE_MAX >> (shift2 & 7)) &&
            (uwide_int_t)val1 > (U_WIDE_MAX >> (shift1 & 15))) {
            checksum += 2;
        }
        
        /* Path 3: Nested conditions with different comparison types */
        int i_f_bits = (i & 3) + 1;  /* 1-4 bits */
        if (check_signed_range(val1, i_f_bits)) {
            checksum += 3;
            if (!check_unsigned_range((uwide_int_t)val2, i_f_bits + 1)) {
                checksum += 5;
            }
        }
        
        /* Modify seed to vary inputs */
        seed = seed * 1103515245 + 12345;
        
        /* Additional arithmetic to create more value ranges */
        wide_int_t combined = val1 + (val2 << (i & 3));
        if (combined > 0 && (uwide_int_t)combined < U_WIDE_MAX) {
            checksum += (combined & 0xFF);
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}
