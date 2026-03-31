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
    
    // Add a modifier that can make the value negative
    shifted += (wide_int_t)modifier * 0x100000001LL;
    
    return shifted;
}

__attribute__((noinline))
uwide_int_t unsigned_wide_compute(int shift_count, int mask) {
    // Start with unsigned value
    uwide_int_t base = (uwide_int_t)0xFEDCBA9876543210ULL;
    
    // Non-constant shift
    uwide_int_t shifted = base << shift_count;
    
    // Apply mask to create bounded but non-constant range
    shifted &= (uwide_int_t)mask * 0xFFFFFFFFULL;
    
    return shifted;
}

int main() {
    volatile int seed = 12345;  // Prevent constant propagation
    uint64_t checksum = 0;
    
    for (int i = 0; i < 1000; i++) {
        // Create bounded, non-constant shift counts
        int shift1 = (seed + i) & WIDE_SHIFT_MASK;
        int shift2 = (seed * i) & WIDE_SHIFT_MASK;
        int mod1 = (seed + i * 3) & 0xFF;
        int mod2 = (seed - i) & 0x7F;
        
        // Compute wide integer values
        wide_int_t signed_val = wide_int_compute(shift1, mod1);
        uwide_int_t unsigned_val = unsigned_wide_compute(shift2, mod2);
        
        // PATH 1: Mixed signed/unsigned comparisons that should trigger
        // the uncovered range analysis code
        
        // This comparison structure mimics the uncovered code's logic:
        // if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
        
        // First, test signed greater-than comparison
        // Use a large positive constant that requires multi-word comparison
#ifdef __SIZEOF_INT128__
        wide_int_t signed_threshold = ((wide_int_t)0x7FFFFFFFFFFFFFFFLL << 32) | 0xFFFFFFFFLL;
#else
        wide_int_t signed_threshold = 0x7FFFFFFFFFFFFFFFLL;
#endif
        
        if (signed_val > signed_threshold) {
            checksum += 1;
        }
        
        // Second, test equality followed by unsigned comparison
        // Create a scenario where high parts might be equal
        wide_int_t compare_base = signed_threshold;
        
        // Force analysis to consider both high and low parts
        if (signed_val > compare_base || 
            (signed_val == compare_base && 
             (uwide_int_t)signed_val > (uwide_int_t)compare_base)) {
            checksum += 2;
        }
        
        // PATH 2: Test unsigned comparisons with complex transformations
        // Create a value that might overflow into high bits
        uwide_int_t large_unsigned = (uwide_int_t)1 << (shift1 + 32);
        
        // This comparison should trigger unsigned range analysis
        if (unsigned_val > large_unsigned) {
            checksum += 4;
        }
        
        // PATH 3: Test with negative values (signed comparisons)
        wide_int_t negative_val = signed_val - 0x8000000000000000LL;
        
        // Compare against negative threshold
        if (negative_val < (wide_int_t)-0x4000000000000000LL) {
            checksum += 8;
        }
        
        // PATH 4: Complex conditional with arithmetic shift simulation
        // The uncovered code uses alshift (arithmetic left shift) and sext
        // Simulate similar operations
        wide_int_t temp = signed_val;
        
        // Simulate arithmetic left shift by variable amount
        if (shift1 > 0) {
            temp = temp << shift1;
        }
        
        // Compare after shift - this should trigger the min_s/max_s initialization
        if (temp > 0 && temp < (wide_int_t)1 << 60) {
            checksum += 16;
        }
        
        // Update seed for next iteration
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
