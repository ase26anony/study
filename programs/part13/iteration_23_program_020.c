#include <stdio.h>
#include <limits.h>
#include <stdint.h>

// Force compiler to perform range analysis on these operations
volatile unsigned long long checksum = 0;

// Helper to prevent optimization
static void accumulate(unsigned long long val) {
    checksum ^= val;
}

int main() {
    // 1. Large integer arithmetic with overflow/underflow
    // Using __int128 to force double-int representation
    __int128 large_signed = (__int128)LLONG_MAX * 4;
    unsigned __int128 large_unsigned = (unsigned __int128)ULLONG_MAX * 3ULL;
    
    // These operations should trigger double-int comparisons
    accumulate((unsigned long long)(large_signed >> 64));
    accumulate((unsigned long long)large_signed);
    accumulate((unsigned long long)(large_unsigned >> 64));
    accumulate((unsigned long long)large_unsigned);
    
    // 2. Complex boundary conditions near max/min values
    long long max_ll = LLONG_MAX;
    long long min_ll = LLONG_MIN;
    unsigned long long max_ull = ULLONG_MAX;
    
    // Operations that create values requiring range analysis
    __int128 product1 = (__int128)max_ll * max_ll;  // Will overflow signed 128-bit
    __int128 product2 = (__int128)min_ll * min_ll;  // Will overflow
    unsigned __int128 product3 = (unsigned __int128)max_ull * max_ull;  // Will overflow
    
    // Use these in comparisons that should trigger the uncovered logic
    if (product1 > (__int128)0x7FFFFFFFFFFFFFFFLL * 0x7FFFFFFFFFFFFFFFLL) {
        accumulate(1);
    }
    
    if (product2 < (__int128)LLONG_MIN * LLONG_MIN) {
        accumulate(2);
    }
    
    if (product3 > (unsigned __int128)ULLONG_MAX * ULLONG_MAX / 2) {
        accumulate(3);
    }
    
    // 3. Loop bounds with complex exit conditions
    // Loop that depends on 128-bit calculations
    for (long long i = 0; i < 100; i++) {
        __int128 limit = (__int128)i * max_ll;
        unsigned __int128 ulimit = (unsigned __int128)i * max_ull;
        
        // Comparisons that should trigger double-int range checks
        if (limit > (__int128)max_ll * 50) {
            accumulate(i * 4);
        }
        
        if (ulimit > (unsigned __int128)max_ull * 50) {
            accumulate(i * 5);
        }
        
        // Nested condition mimicking the uncovered logic structure
        // High part comparison followed by low part comparison
        __int128 val = (__int128)i << 60;
        __int128 threshold_high = (__int128)0x7FFFFFFFFFFFFFFFLL;
        __int128 threshold_low = (__int128)0xFFFFFFFFFFFFFFFFULL;
        
        if (val > threshold_high || 
            (val == threshold_high && (unsigned __int128)val > (unsigned __int128)threshold_low)) {
            accumulate(i * 6);
        }
    }
    
    // 4. Bit-field operations and masking with wide integers
    unsigned __int128 wide_mask = ~((unsigned __int128)0);
    unsigned __int128 shifted = wide_mask >> 32;
    unsigned __int128 combined = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    // Complex comparisons with bit masks
    for (int shift = 0; shift < 128; shift += 16) {
        unsigned __int128 masked = combined & (wide_mask >> shift);
        
        // Comparison that should trigger range analysis
        if (masked > (unsigned __int128)0xFFFFFFFFFFFFFFFFULL) {
            accumulate(shift * 7);
        }
        
        // Extract high and low parts for separate comparison
        unsigned long long high_part = (unsigned long long)(masked >> 64);
        unsigned long long low_part = (unsigned long long)masked;
        
        // This structure mimics the uncovered code's logic
        if (high_part > 0 || (high_part == 0 && low_part > 0xFFFFFFFF)) {
            accumulate(shift * 8);
        }
    }
    
    // 5. Compiler built-ins for overflow checking
    long long a = LLONG_MAX / 2;
    long long b = 3;
    long long result;
    
    // These built-ins may invoke the fixed-value logic
    if (__builtin_mul_overflow(a, b, &result)) {
        accumulate(9);
    }
    
    unsigned long long ua = ULLONG_MAX / 2;
    unsigned long long ub = 3;
    unsigned long long uresult;
    
    if (__builtin_mul_overflow(ua, ub, &uresult)) {
        accumulate(10);
    }
    
    // 6. Additional stress tests with mixed signed/unsigned
    // Comparisons that cross signed/unsigned boundaries
    __int128 signed_val = -((__int128)1 << 120);
    unsigned __int128 unsigned_val = (unsigned __int128)1 << 120;
    
    // These comparisons require careful range analysis
    if ((unsigned __int128)signed_val > unsigned_val) {
        accumulate(11);
    }
    
    // Chain of comparisons
    __int128 chain_val = (__int128)LLONG_MAX * LLONG_MAX;
    for (int i = 0; i < 10; i++) {
        chain_val /= 2;
        
        // Complex condition that might trigger the specific uncovered block
        // when high part equals max_r and low part exceeds max_s
        if (chain_val > ((__int128)0x3FFFFFFFFFFFFFFFLL << 64) ||
            (chain_val == ((__int128)0x3FFFFFFFFFFFFFFFLL << 64) && 
             (unsigned __int128)chain_val > (unsigned __int128)0xFFFFFFFFFFFFFFFFULL)) {
            accumulate(i * 12);
        }
    }
    
    // Final output to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
