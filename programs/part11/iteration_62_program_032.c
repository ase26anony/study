/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Use static assertions for compile-time comparisons */
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

/* Large 128-bit constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define HIGH_DIFF_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

#define HIGH_EQUAL_LOW_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define HIGH_EQUAL_LOW_DIFF_B ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

#define BOTH_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define BOTH_DIFF_B ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1)  /* All bits set: high=0xFFFFFFFFFFFFFFFF, low=0xFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFFULL)

/* Compile-time constant comparisons that should trigger all branches */
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
              "High part less comparison should be true");
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A,
              "High part greater comparison should be true");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
              "Low part less comparison should be true");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A,
              "Low part greater comparison should be true");
STATIC_ASSERT(BOTH_DIFF_A < BOTH_DIFF_B,
              "Both parts less comparison should be true");
STATIC_ASSERT(BOTH_DIFF_B > BOTH_DIFF_A,
              "Both parts greater comparison should be true");

/* Signed comparisons that use unsigned high-part semantics */
STATIC_ASSERT(SIGNED_NEG_ONE < SIGNED_ZERO,
              "Signed -1 < 0 should be true (uses unsigned high comparison)");
STATIC_ASSERT(SIGNED_ZERO > SIGNED_NEG_ONE,
              "Signed 0 > -1 should be true");

/* Test equality cases */
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_A == HIGH_DIFF_LOW_EQUAL_A,
              "Equality should hold");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_A == HIGH_EQUAL_LOW_DIFF_A,
              "Equality should hold");

/* Use in constant expressions */
const int compile_time_result_1 = (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 1 : 0;
const int compile_time_result_2 = (HIGH_EQUAL_LOW_DIFF_A > HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int compile_time_result_3 = (BOTH_DIFF_A <= BOTH_DIFF_B) ? 1 : 0;
const int compile_time_result_4 = (SIGNED_NEG_ONE >= SIGNED_ZERO) ? 1 : 0;

/* Array size depending on comparison result */
char array_high_less[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char array_low_less[(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 15 : 25];

/* Runtime comparisons with volatile to prevent constant folding */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Volatile variables force runtime evaluation */
    volatile unsigned __int128 v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile unsigned __int128 v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile unsigned __int128 v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile unsigned __int128 v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile unsigned __int128 v5 = BOTH_DIFF_A;
    volatile unsigned __int128 v6 = BOTH_DIFF_B;
    
    volatile __int128 vs1 = SIGNED_NEG_ONE;
    volatile __int128 vs2 = SIGNED_ZERO;
    volatile __int128 vs3 = SIGNED_LARGE_POS;
    
    /* Test all comparison operators with volatile variables */
    if (v1 < v2) checksum += 1;    /* High part less */
    if (v2 > v1) checksum += 2;    /* High part greater */
    if (v3 < v4) checksum += 4;    /* Low part less */
    if (v4 > v3) checksum += 8;    /* Low part greater */
    if (v5 < v6) checksum += 16;   /* Both parts less */
    if (v6 > v5) checksum += 32;   /* Both parts greater */
    
    /* Signed comparisons */
    if (vs1 < vs2) checksum += 64;   /* Negative < 0 */
    if (vs2 > vs1) checksum += 128;  /* 0 > Negative */
    if (vs3 > vs1) checksum += 256;  /* Large positive > -1 */
    
    /* Test <= and >= operators */
    if (v1 <= v2) checksum += 512;
    if (v2 >= v1) checksum += 1024;
    if (v3 <= v4) checksum += 2048;
    if (v4 >= v3) checksum += 4096;
    
    return checksum;
}

/* Use GCC built-ins that may trigger internal comparisons */
static int builtin_comparisons(void) {
    int result = 0;
    unsigned __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    unsigned __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    __int128 sa = SIGNED_NEG_ONE;
    __int128 sb = SIGNED_ZERO;
    
    /* __builtin_add_overflow_p may perform comparisons internally */
    if (__builtin_add_overflow_p(a, b, (unsigned __int128)0)) {
        result += 1;
    }
    
    /* __builtin_mul_overflow may also perform comparisons */
    unsigned __int128 mul_result;
    if (__builtin_mul_overflow(a, (unsigned __int128)2, &mul_result)) {
        result += 2;
    }
    
    /* Compare results of built-in operations */
    unsigned __int128 sum = a + b;
    if (sum > a) result += 4;
    if (sum > b) result += 8;
    
    /* Signed overflow check */
    __int128 signed_sum;
    if (__builtin_add_overflow(sa, sb, &signed_sum)) {
        result += 16;
    }
    
    return result;
}

/* C++ version with constexpr (compile as C++ with g++) */
#ifdef __cplusplus
#include <cstdint>

constexpr bool constexpr_compare_high_diff() {
    const unsigned __int128 ca = HIGH_DIFF_LOW_EQUAL_A;
    const unsigned __int128 cb = HIGH_DIFF_LOW_EQUAL_B;
    return ca < cb;
}

constexpr bool constexpr_compare_low_diff() {
    const unsigned __int128 ca = HIGH_EQUAL_LOW_DIFF_A;
    const unsigned __int128 cb = HIGH_EQUAL_LOW_DIFF_B;
    return ca > cb;
}

constexpr bool constexpr_compare_signed() {
    const __int128 ca = SIGNED_NEG_ONE;
    const __int128 cb = SIGNED_ZERO;
    return ca <= cb;
}

template<unsigned __int128 A, unsigned __int128 B>
struct CompareTemplate {
    static constexpr bool less = A < B;
    static constexpr bool greater = A > B;
    static constexpr bool equal = A == B;
};

/* Instantiate templates with different comparison scenarios */
constexpr bool template_high_less = CompareTemplate<
    HIGH_DIFF_LOW_EQUAL_A, HIGH_DIFF_LOW_EQUAL_B>::less;
constexpr bool template_low_greater = CompareTemplate<
    HIGH_EQUAL_LOW_DIFF_B, HIGH_EQUAL_LOW_DIFF_A>::greater;
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time results */
    total_checksum += compile_time_result_1;
    total_checksum += compile_time_result_2;
    total_checksum += compile_time_result_3;
    total_checksum += compile_time_result_4;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
#ifdef __cplusplus
    /* C++ constexpr comparisons */
    if (constexpr_compare_high_diff()) total_checksum += 1000;
    if (!constexpr_compare_low_diff()) total_checksum += 2000;
    if (constexpr_compare_signed()) total_checksum += 3000;
    
    /* Template comparisons */
    if (template_high_less) total_checksum += 4000;
    if (template_low_greater) total_checksum += 8000;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", 
           sizeof(array_high_less), sizeof(array_low_less));
    
    /* Additional runtime tests with mixed signed/unsigned */
    {
        unsigned __int128 ua = 0xFFFFFFFFFFFFFFFFULL;
        unsigned __int128 ub = ua << 64;
        __int128 sa = -1;
        __int128 sb = 0;
        
        /* These should trigger the comparison logic */
        if (ua < ub) total_checksum += 10000;
        if (sa < sb) total_checksum += 20000;
        if ((unsigned __int128)sa > ub) total_checksum += 30000;
    }
    
    printf("Final checksum: %d\n", total_checksum);
    
    return 0;
}
