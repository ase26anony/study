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

// Prevent inlining to ensure analysis happens at optimization time
__attribute__((noinline))
wide_int_t wide_int_compute(int shift_count, int modifier) {
    // Start with a value that can become negative when shifted
    wide_int_t base = (wide_int_t)0x123456789ABCDEF0LL;
    
    // Non-constant shift - forces range analysis
    wide_int_t shifted = base << shift_count;
    
    // Add/subtract based on modifier to create signed complexity
    if (modifier & 1) {
        shifted += (wide_int_t)0xFEDCBA9876543210LL;
    } else {
        shifted -= (wide_int_t)0x123456789ABCDEFLL;
    }
    
    return shifted;
}

__attribute__((noinline))
int analyze_signed_range(wide_int_t value, int i) {
    int result = 0;
    
    // This comparison should trigger signed range analysis
    // The constant 0x7FFFFFFFFFFFFFFFLL is the max positive for signed 64-bit
    // For 128-bit, it's still a boundary that requires analysis
    if (value > (wide_int_t)0x7FFFFFFFFFFFFFFFLL) {
        result |= 1;
    }
    
    // Another signed comparison with negative boundary
    if (value < (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL - 1)) {
        result |= 2;
    }
    
    // Mixed signed/unsigned: cast to unsigned then compare
    uwide_int_t uval = (uwide_int_t)value;
    
    // This unsigned comparison should trigger the ugt logic
    if (uval > (uwide_int_t)0xFFFFFFFFFFFFFFFFULL) {
        result |= 4;
    }
    
    // Complex condition with && and || - encourages multiple range checks
    if ((value > 0 && uval < 1000) || (value < 0 && uval > 0xFFFFFFFF00000000ULL)) {
        result |= 8;
    }
    
    return result;
}

__attribute__((noinline))
wide_int_t create_min_max_scenarios(int shift, int mod) {
    wide_int_t x = (wide_int_t)1 << shift;
    
    // Create scenarios that might hit min/max boundaries
    if (mod & 2) {
        x = -x;
    }
    
    if (mod & 4) {
        x = x >> (shift / 2);
    }
    
    return x;
}

int main() {
    volatile int seed = 12345;  // volatile to prevent constant propagation
    unsigned long long checksum = 0;
    
    // Loop to provide multiple analysis opportunities
    for (int i = 0; i < 1000; i++) {
        // Bounded, non-constant shift amounts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & WIDE_SHIFT_MASK;
        int modifier = (seed ^ i) & 7;  // Keep modifier small but variable
        
        // Compute with wide integers
        wide_int_t val1 = wide_int_compute(shift1, modifier);
        wide_int_t val2 = create_min_max_scenarios(shift2, modifier);
        
        // Perform range analysis that should trigger the target code
        int analysis1 = analyze_signed_range(val1, i);
        int analysis2 = analyze_signed_range(val2, i);
        
        // Mix results to create control flow complexity
        wide_int_t combined;
        if (analysis1 & 1) {
            combined = val1 + val2;
        } else if (analysis2 & 2) {
            combined = val1 - val2;
        } else {
            combined = val1 * (modifier + 1);
        }
        
        // Final comparison that might use the initialized max_r/min_s structures
        uwide_int_t ucombined = (uwide_int_t)combined;
        if (combined > (wide_int_t)0x3FFFFFFFFFFFFFFFLL || 
            ucombined > (uwide_int_t)0xFFFFFFFFFFFFFFULL) {
            checksum += analysis1 + analysis2 + i;
        }
        
        // Update seed to vary inputs
        seed = seed * 1103515245 + 12345;
    }
    
    // Use checksum to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    // Additional test with specific boundary values
    {
        // Test near boundary cases
        wide_int_t near_max = (wide_int_t)0x7FFFFFFFFFFFFFFFLL;
        wide_int_t near_min = (wide_int_t)(-0x7FFFFFFFFFFFFFFFLL - 1);
        
        // These comparisons should exercise the range analysis
        int test1 = analyze_signed_range(near_max >> 1, 0);
        int test2 = analyze_signed_range(near_min << 1, 1);
        
        printf("Boundary tests: %d %d\n", test1, test2);
    }
    
    return 0;
}
