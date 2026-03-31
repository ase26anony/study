#include <stdio.h>
#include <stdint.h>

#ifdef __SIZEOF_INT128__
typedef __int128 wide_int_t;
typedef unsigned __int128 uwide_int_t;
#define WIDE_SHIFT_MASK 63
#else
typedef long long wide_int_t;
typedef unsigned long long uwide_int_t;
#define WIDE_SHIFT_MASK 31
#endif

// Prevent inlining to ensure analysis happens
__attribute__((noinline)) 
wide_int_t wide_int_compute(int shift_amount, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_amount;
    
    // Add/subtract based on modifier to create signed complexity
    if (modifier & 1) {
        shifted += (wide_int_t)(modifier * 0x100000001LL);
    } else {
        shifted -= (wide_int_t)(modifier * 0x100000001LL);
    }
    
    return shifted;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_amount, int modifier) {
    // Start with unsigned value
    uwide_int_t base = (uwide_int_t)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    uwide_int_t shifted = base << shift_amount;
    
    // Apply mask to ensure unsigned behavior
    shifted &= (uwide_int_t)0xFFFFFFFFFFFFFFFFULL;
    
    // Add based on modifier
    shifted += (uwide_int_t)(modifier * 0x55555555ULL);
    
    return shifted;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    uwide_int_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Generate bounded, non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i + 12345) & WIDE_SHIFT_MASK;
        
        // Get signed wide integer result
        wide_int_t signed_result = wide_int_compute(shift1, i);
        
        // Get unsigned wide integer result  
        uwide_int_t unsigned_result = unsigned_wide_compute(shift2, i);
        
        // PATH 1: Signed comparison against large positive value
        // This should trigger signed greater-than (sgt) analysis
        if (signed_result > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
            checksum += (uwide_int_t)1;
        }
        
        // PATH 2: Combined signed equality and unsigned comparison
        // This mimics the structure in the uncovered code
        wide_int_t compare_val = (wide_int_t)0x12345678LL << shift1;
        if (signed_result > compare_val || 
            (signed_result == compare_val && 
             (uwide_int_t)signed_result > (uwide_int_t)compare_val)) {
            checksum += (uwide_int_t)2;
        }
        
        // PATH 3: Unsigned comparison against max value
        // This should trigger unsigned greater-than (ugt) analysis
        if (unsigned_result > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
            checksum += (uwide_int_t)4;
        }
        
        // PATH 4: Complex conditional with mixed comparisons
        // Force analysis of min_r/min_s initialization
        wide_int_t temp = (wide_int_t)i << (shift1 & 15);
        if (temp < (wide_int_t)0 || (uwide_int_t)temp < (uwide_int_t)0x1000) {
            checksum += (uwide_int_t)8;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    // Print result to prevent dead code elimination
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
