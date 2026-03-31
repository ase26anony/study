#include <stdio.h>
#include <stdint.h>

// Compile-time comparisons using static assertions
#define COMPILE_TIME_CHECK(cond, msg) _Static_assert(cond, msg)

// Large 128-bit constants that exercise different comparison paths
#define HIGH_DIFF_LOW_EQUAL_A   (((__int128)0x1ULL) << 64)          // 0x10000000000000000
#define HIGH_DIFF_LOW_EQUAL_B   (((__int128)0x2ULL) << 64)          // 0x20000000000000000
#define HIGH_EQUAL_LOW_DIFF_A   (((__int128)0x1ULL) << 64) | 0x1ULL // 0x10000000000000001
#define HIGH_EQUAL_LOW_DIFF_B   (((__int128)0x1ULL) << 64) | 0x2ULL // 0x10000000000000002
#define BOTH_DIFF_A             (((__int128)0x1ULL) << 64) | 0x1ULL // 0x10000000000000001
#define BOTH_DIFF_B             (((__int128)0x2ULL) << 64) | 0x2ULL // 0x20000000000000002
#define SIGNED_NEG_ONE          ((__int128)-1)                      // 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define SIGNED_ZERO             ((__int128)0)
#define UNSIGNED_MAX            ((unsigned __int128)-1)

// Compile-time assertions to force constant folding
COMPILE_TIME_CHECK(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
                   "High diff, low equal: A < B should be true");
COMPILE_TIME_CHECK(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B, 
                   "High equal, low diff: A < B should be true");
COMPILE_TIME_CHECK(BOTH_DIFF_A < BOTH_DIFF_B, 
                   "Both diff: A < B should be true");
COMPILE_TIME_CHECK(SIGNED_NEG_ONE < SIGNED_ZERO, 
                   "Signed -1 < 0 should be true");
COMPILE_TIME_CHECK(UNSIGNED_MAX > 0, 
                   "Unsigned max > 0 should be true");

// Constant expressions that use comparisons
const int cmp_high_diff = (HIGH_DIFF_LOW_EQUAL_A > HIGH_DIFF_LOW_EQUAL_B) ? 0 : 1;
const int cmp_high_equal = (HIGH_EQUAL_LOW_DIFF_A <= HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int cmp_both_diff = (BOTH_DIFF_A >= BOTH_DIFF_B) ? 0 : 1;
const int cmp_signed = (SIGNED_NEG_ONE <= SIGNED_ZERO) ? 1 : 0;

// Array size depending on comparison result
char array_high_diff[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char array_low_diff[(HIGH_EQUAL_LOW_DIFF_A > HIGH_EQUAL_LOW_DIFF_B) ? 5 : 15];

// Runtime comparison function
int runtime_comparisons(void) {
    int checksum = 0;
    
    // Use volatile to force runtime evaluation
    volatile __int128 v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile __int128 v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile __int128 v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile __int128 v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile __int128 v5 = BOTH_DIFF_A;
    volatile __int128 v6 = BOTH_DIFF_B;
    volatile __int128 v7 = SIGNED_NEG_ONE;
    volatile __int128 v8 = SIGNED_ZERO;
    volatile unsigned __int128 uv1 = UNSIGNED_MAX;
    volatile unsigned __int128 uv2 = 0;
    
    // High part differs, low part equal
    if ((unsigned __int128)v1 < (unsigned __int128)v2) checksum += 1;  // Should take
    if ((unsigned __int128)v1 > (unsigned __int128)v2) checksum += 0;  // Should not take
    if (v1 < v2) checksum += 1;  // Should take
    if (v1 > v2) checksum += 0;  // Should not take
    
    // High part equal, low part differs
    if (v3 < v4) checksum += 1;  // Should take (low < low)
    if (v3 > v4) checksum += 0;  // Should not take
    if (v3 <= v4) checksum += 1; // Should take
    if (v3 >= v4) checksum += 0; // Should not take
    
    // Both parts differ
    if (v5 < v6) checksum += 1;  // Should take
    if (v5 > v6) checksum += 0;  // Should not take
    
    // Signed comparisons with negative values
    if (v7 < v8) checksum += 1;  // Should take (-1 < 0)
    if (v7 > v8) checksum += 0;  // Should not take
    if (v7 <= v8) checksum += 1; // Should take
    if (v7 >= v8) checksum += 0; // Should not take
    
    // Unsigned comparisons
    if (uv1 > uv2) checksum += 1;  // Should take (max > 0)
    if (uv1 < uv2) checksum += 0;  // Should not take
    if (uv1 >= uv2) checksum += 1; // Should take
    if (uv1 <= uv2) checksum += 0; // Should not take
    
    // Mixed signed/unsigned (should trigger unsigned comparison for high parts)
    if ((unsigned __int128)v7 > (unsigned __int128)v8) checksum += 1;  // Should take
    
    return checksum;
}

// Use GCC built-ins that may involve comparisons
int builtin_comparisons(void) {
    int checksum = 0;
    __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    __int128 result;
    int overflow;
    
    // Multiplication overflow check
    overflow = __builtin_mul_overflow(a, b, &result);
    checksum += overflow ? 0 : 1;
    
    // Addition overflow check
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 0 : 1;
    
    // Subtraction overflow check
    overflow = __builtin_sub_overflow(b, a, &result);
    checksum += overflow ? 0 : 1;
    
    return checksum;
}

// C++ specific constexpr comparisons (if compiled as C++)
#ifdef __cplusplus
constexpr bool constexpr_compare_high_diff() {
    const __int128 a = HIGH_DIFF_LOW_EQUAL_A;
    const __int128 b = HIGH_DIFF_LOW_EQUAL_B;
    return a < b;
}

constexpr bool constexpr_compare_low_diff() {
    const __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    const __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    return a > b;
}

constexpr bool constexpr_compare_both_diff() {
    const __int128 a = BOTH_DIFF_A;
    const __int128 b = BOTH_DIFF_B;
    return a <= b;
}

constexpr int constexpr_checksum() {
    int sum = 0;
    sum += constexpr_compare_high_diff() ? 1 : 0;
    sum += constexpr_compare_low_diff() ? 0 : 1;  // a > b is false, so add 1
    sum += constexpr_compare_both_diff() ? 1 : 0;
    return sum;
}
#endif

int main(void) {
    int total_checksum = 0;
    
    // Add compile-time comparison results
    total_checksum += cmp_high_diff;
    total_checksum += cmp_high_equal;
    total_checksum += cmp_both_diff;
    total_checksum += cmp_signed;
    
    // Runtime comparisons
    total_checksum += runtime_comparisons();
    
    // Built-in function comparisons
    total_checksum += builtin_comparisons();
    
    // C++ constexpr comparisons
    #ifdef __cplusplus
    total_checksum += constexpr_checksum();
    #endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    // Additional runtime checks to prevent dead code elimination
    volatile __int128 test1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile __int128 test2 = HIGH_DIFF_LOW_EQUAL_B;
    if (test1 == test2) {
        printf("Unexpected equality\n");
    }
    
    return 0;
}
