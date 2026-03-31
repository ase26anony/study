#include <stdio.h>
#include <stdint.h>

// Compile-time comparisons using static assertions
#define COMPILE_TIME_CHECKS() do { \
    /* High part differs, low part equal */ \
    _Static_assert(((unsigned __int128)0x10000000000000000ULL) < \
                   ((unsigned __int128)0x20000000000000000ULL), \
                   "High part less comparison failed"); \
    \
    /* High part differs in opposite direction */ \
    _Static_assert(((unsigned __int128)0x20000000000000000ULL) > \
                   ((unsigned __int128)0x10000000000000000ULL), \
                   "High part greater comparison failed"); \
    \
    /* High part equal, low part differs */ \
    _Static_assert(((unsigned __int128)0x10000000000000001ULL) < \
                   ((unsigned __int128)0x10000000000000002ULL), \
                   "Low part less comparison failed"); \
    \
    /* High part equal, low part differs in opposite direction */ \
    _Static_assert(((unsigned __int128)0x10000000000000002ULL) > \
                   ((unsigned __int128)0x10000000000000001ULL), \
                   "Low part greater comparison failed"); \
    \
    /* Both parts differ */ \
    _Static_assert(((unsigned __int128)0x10000000000000001ULL) < \
                   ((unsigned __int128)0x20000000000000002ULL), \
                   "Both parts less comparison failed"); \
    \
    /* Signed comparisons with negative values */ \
    _Static_assert(((__int128)-1) < ((__int128)0), \
                   "Signed negative comparison failed"); \
    \
    /* Signed comparisons with high bits set */ \
    _Static_assert(((__int128)0x8000000000000000ULL << 64) < \
                   ((__int128)0x8000000000000001ULL << 64), \
                   "Signed high-bit comparison failed"); \
} while(0)

// Use array sizes to force compile-time evaluation
char array_high_less[(unsigned __int128)0x10000000000000000ULL < 
                     (unsigned __int128)0x20000000000000000ULL ? 10 : 20];
char array_high_greater[(unsigned __int128)0x20000000000000000ULL > 
                        (unsigned __int128)0x10000000000000000ULL ? 10 : 20];
char array_low_less[(unsigned __int128)0x10000000000000001ULL < 
                    (unsigned __int128)0x10000000000000002ULL ? 10 : 20];
char array_low_greater[(unsigned __int128)0x10000000000000002ULL > 
                       (unsigned __int128)0x10000000000000001ULL ? 10 : 20];

// Constant expressions
const int const_result_high = ((unsigned __int128)0x10000000000000000ULL < 
                               (unsigned __int128)0x20000000000000000ULL) ? 1 : 0;
const int const_result_low = ((unsigned __int128)0x10000000000000001ULL < 
                              (unsigned __int128)0x10000000000000002ULL) ? 1 : 0;

// Test built-in overflow functions
int test_builtin_overflow(void) {
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 b = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 result;
    int overflow = __builtin_mul_overflow(a, b, &result);
    return overflow;
}

int main(void) {
    int checksum = 0;
    
    // Execute compile-time checks
    COMPILE_TIME_CHECKS();
    
    // Runtime comparisons with volatile to prevent optimization
    volatile unsigned __int128 runtime_a = (unsigned __int128)0x10000000000000000ULL;
    volatile unsigned __int128 runtime_b = (unsigned __int128)0x20000000000000000ULL;
    volatile unsigned __int128 runtime_c = (unsigned __int128)0x10000000000000001ULL;
    volatile unsigned __int128 runtime_d = (unsigned __int128)0x10000000000000002ULL;
    
    // Test all four comparison branches at runtime
    if ((unsigned __int128)runtime_a < (unsigned __int128)runtime_b) {
        checksum += 1;  // High part less
    }
    
    if ((unsigned __int128)runtime_b > (unsigned __int128)runtime_a) {
        checksum += 2;  // High part greater
    }
    
    if ((unsigned __int128)runtime_c < (unsigned __int128)runtime_d) {
        checksum += 4;  // Low part less
    }
    
    if ((unsigned __int128)runtime_d > (unsigned __int128)runtime_c) {
        checksum += 8;  // Low part greater
    }
    
    // Test signed comparisons
    volatile __int128 signed_a = -1;
    volatile __int128 signed_b = 0;
    volatile __int128 signed_c = ((__int128)0x8000000000000000ULL << 64);
    volatile __int128 signed_d = ((__int128)0x8000000000000001ULL << 64);
    
    if (signed_a < signed_b) {
        checksum += 16;  // Signed negative comparison
    }
    
    if (signed_c < signed_d) {
        checksum += 32;  // Signed high-bit comparison
    }
    
    // Test equality cases (should not trigger the uncovered lines but good for completeness)
    volatile unsigned __int128 equal_a = (unsigned __int128)0x123456789ABCDEF0ULL << 64;
    volatile unsigned __int128 equal_b = (unsigned __int128)0x123456789ABCDEF0ULL << 64;
    
    if (equal_a == equal_b) {
        checksum += 64;  // Equality comparison
    }
    
    // Test built-in overflow functions
    checksum += test_builtin_overflow() * 128;
    
    // Mix signed and unsigned comparisons
    checksum += (signed_a < (__int128)runtime_a) ? 256 : 0;
    checksum += ((unsigned __int128)signed_c < runtime_b) ? 512 : 0;
    
    // Use constant expression results
    checksum += const_result_high * 1024;
    checksum += const_result_low * 2048;
    
    // Force evaluation of array sizes (prevent unused variable warnings)
    if (sizeof(array_high_less) > 0 &&
        sizeof(array_high_greater) > 0 &&
        sizeof(array_low_less) > 0 &&
        sizeof(array_low_greater) > 0) {
        checksum += 4096;
    }
    
    printf("Checksum: %d\n", checksum);
    
    // Additional complex comparisons to ensure all paths are taken
    // Compare values where high parts are equal but low parts need comparison
    const unsigned __int128 const1 = ((unsigned __int128)0x1ULL << 64) | 0x1ULL;
    const unsigned __int128 const2 = ((unsigned __int128)0x1ULL << 64) | 0x2ULL;
    
    // These should be evaluated at compile time
    const int complex1 = (const1 < const2) ? 1 : 0;
    const int complex2 = (const2 > const1) ? 1 : 0;
    
    // Compare values where high parts differ
    const unsigned __int128 const3 = ((unsigned __int128)0x1ULL << 64);
    const unsigned __int128 const4 = ((unsigned __int128)0x2ULL << 64);
    
    const int complex3 = (const3 < const4) ? 1 : 0;
    const int complex4 = (const4 > const3) ? 1 : 0;
    
    printf("Complex checks: %d %d %d %d\n", complex1, complex2, complex3, complex4);
    
    return 0;
}
