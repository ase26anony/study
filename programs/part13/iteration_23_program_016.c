#include <stdio.h>
#include <limits.h>
#include <stdint.h>

// Force compiler to perform range analysis on wide integers
volatile unsigned long long checksum = 0;

// Helper to prevent optimization
static void accumulate(unsigned long long val) {
    checksum ^= val;
}

int main() {
    // 1. Large integer arithmetic with overflow/underflow
    // Use __int128 operations that require double-int representation
    __int128 large_signed = (__int128)LLONG_MAX * 4;
    unsigned __int128 large_unsigned = (unsigned __int128)ULLONG_MAX * 3ULL;
    
    // These will force double-int comparisons
    accumulate((unsigned long long)(large_signed >> 64));
    accumulate((unsigned long long)large_signed);
    accumulate((unsigned long long)(large_unsigned >> 64));
    accumulate((unsigned long long)large_unsigned);
    
    // 2. Complex boundary conditions
    // Near maximum signed 128-bit value
    __int128 near_max = ((__int128)LLONG_MAX << 64) | ULLONG_MAX;
    __int128 near_min = ((__int128)LLONG_MIN << 64);
    
    // Comparisons that should trigger a_high.sgt(max_r) logic
    if (near_max > (((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL)) {
        accumulate(1);
    }
    
    if (near_min < (((__int128)0x8000000000000000LL << 64))) {
        accumulate(2);
    }
    
    // 3. Loop bounds with complex exit conditions
    long long base = LLONG_MAX / 4;
    unsigned long long ubase = ULLONG_MAX / 4;
    
    // Loop with 128-bit limit calculation
    for (long long i = 0; i < 100; i++) {
        // Create 128-bit limit from multiplication
        __int128 limit = (__int128)base * i;
        unsigned __int128 ulimit = (unsigned __int128)ubase * (unsigned long long)i;
        
        // Conditional based on wide comparisons
        if ((__int128)i * base > (__int128)LLONG_MAX) {
            accumulate(i);
        }
        
        if ((unsigned __int128)i * ubase > (unsigned __int128)ULLONG_MAX) {
            accumulate(i + 1000);
        }
        
        // Chain comparisons mimicking the uncovered logic
        // This should trigger both high and low part comparisons
        if (limit > (((__int128)0x3FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL) ||
            (limit == (((__int128)0x3FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL) &&
             ulimit > 0xFFFFFFFFFFFFFFFFULL)) {
            accumulate(i + 2000);
        }
    }
    
    // 4. Bit-field operations and masking
    unsigned __int128 wide_val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    for (int shift = 0; shift < 128; shift += 16) {
        unsigned __int128 masked = (wide_val >> shift) & 0xFFFF;
        if (masked > 0x8000) {
            accumulate(shift);
        }
    }
    
    // 5. Compiler built-ins for overflow detection
    long long a = LLONG_MAX / 2;
    long long b = 3;
    long long result;
    
    // These built-ins may invoke fixed-value logic
    if (__builtin_mul_overflow(a, b, &result)) {
        accumulate(0xDEADBEEF);
    }
    
    if (__builtin_add_overflow(LLONG_MAX, 1, &result)) {
        accumulate(0xBEEFDEAD);
    }
    
    // 6. Mixed signed/unsigned comparisons
    unsigned long long umax = ULLONG_MAX;
    long long smax = LLONG_MAX;
    
    // This requires careful range analysis
    if ((unsigned __int128)umax > (__int128)smax) {
        accumulate(0xCAFEBABE);
    }
    
    // 7. Division with wide dividends
    __int128 huge_dividend = ((__int128)LLONG_MAX << 32);
    for (long long divisor = 1; divisor < 100; divisor++) {
        __int128 quotient = huge_dividend / divisor;
        if (quotient > (__int128)LLONG_MAX << 16) {
            accumulate(divisor);
        }
    }
    
    // 8. Nested expressions requiring multi-precision comparison
    __int128 complex_expr = ((__int128)a * b * 17) >> 4;
    unsigned __int128 ucomplex_expr = ((unsigned __int128)ubase * 23) << 3;
    
    // Direct analog of the uncovered comparison pattern
    // if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
    __int128 threshold_high = ((__int128)0x0 << 64) | 0x0;  // max_r
    unsigned __int128 threshold_low = (unsigned __int128)-1;  // max_s
    
    // Apply zero extension (simulating zext)
    threshold_low = threshold_low & (((unsigned __int128)1 << 64) - 1);
    
    // Create test values that should trigger the comparison
    __int128 test_val_high = 0;
    unsigned __int128 test_val_low = ULLONG_MAX;
    
    // This comparison structure should match the uncovered code
    if (test_val_high > threshold_high ||
        (test_val_high == threshold_high && 
         test_val_low > threshold_low)) {
        accumulate(0x12345678);
    }
    
    // 9. Final output to prevent dead code elimination
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
