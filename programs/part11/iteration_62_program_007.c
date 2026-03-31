/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Use static assertions for compile-time evaluation */
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

/* Large constants that exercise high and low parts */
#define HIGH_1_LOW_0    (((__int128)0x1ULL) << 64)
#define HIGH_2_LOW_0    (((__int128)0x2ULL) << 64)
#define HIGH_1_LOW_1    ((((__int128)0x1ULL) << 64) | 0x1ULL)
#define HIGH_1_LOW_2    ((((__int128)0x1ULL) << 64) | 0x2ULL)
#define HIGH_2_LOW_2    ((((__int128)0x2ULL) << 64) | 0x2ULL)
#define ALL_ONES_128    ((__int128)-1)
#define HIGH_MSB_SET    (((__int128)0x8000000000000000ULL) << 64)

/* Unsigned versions */
#define UHIGH_1_LOW_0   (((unsigned __int128)0x1ULL) << 64)
#define UHIGH_2_LOW_0   (((unsigned __int128)0x2ULL) << 64)
#define UHIGH_1_LOW_1   ((((unsigned __int128)0x1ULL) << 64) | 0x1ULL)
#define UHIGH_1_LOW_2   ((((unsigned __int128)0x1ULL) << 64) | 0x2ULL)

/* Compile-time comparisons that should be evaluated in constant folding */
STATIC_ASSERT(HIGH_2_LOW_0 > HIGH_1_LOW_0, "High part greater comparison");
STATIC_ASSERT(HIGH_1_LOW_0 < HIGH_2_LOW_0, "High part less comparison");
STATIC_ASSERT(HIGH_1_LOW_2 > HIGH_1_LOW_1, "Low part greater comparison");
STATIC_ASSERT(HIGH_1_LOW_1 < HIGH_1_LOW_2, "Low part less comparison");
STATIC_ASSERT(HIGH_2_LOW_2 > HIGH_1_LOW_1, "Both parts greater comparison");
STATIC_ASSERT(HIGH_1_LOW_1 < HIGH_2_LOW_2, "Both parts less comparison");

/* Signed comparisons with sign extension handling */
STATIC_ASSERT(ALL_ONES_128 < 0, "Negative comparison (all ones < 0)");
STATIC_ASSERT(0 > ALL_ONES_128, "Zero greater than -1");
STATIC_ASSERT(HIGH_MSB_SET < 0, "MSB set is negative");

/* Unsigned comparisons */
STATIC_ASSERT(UHIGH_2_LOW_0 > UHIGH_1_LOW_0, "Unsigned high part greater");
STATIC_ASSERT(UHIGH_1_LOW_2 > UHIGH_1_LOW_1, "Unsigned low part greater");

/* Mixed signed/unsigned comparisons */
STATIC_ASSERT((unsigned __int128)ALL_ONES_128 > 0, "Unsigned -1 > 0");

/* Test overflow builtins that may use double_int comparisons */
static int test_overflow_builtins(void) {
    __int128 a, b, result;
    int overflow;
    
    /* These may trigger comparisons in overflow checking */
    a = HIGH_1_LOW_0;
    b = HIGH_1_LOW_0;
    overflow = __builtin_mul_overflow(a, b, &result);
    
    a = ALL_ONES_128;
    b = 2;
    overflow |= __builtin_mul_overflow(a, b, &result);
    
    return overflow;
}

/* Runtime comparisons with volatile to force code generation */
static int runtime_comparisons(void) {
    volatile __int128 var1, var2;
    volatile unsigned __int128 uvar1, uvar2;
    int checksum = 0;
    
    /* Test 1: High parts differ, low parts equal */
    var1 = HIGH_1_LOW_0;
    var2 = HIGH_2_LOW_0;
    if (var1 < var2) checksum += 1;  /* Should take */
    if (var2 > var1) checksum += 2;  /* Should take */
    
    /* Test 2: High parts equal, low parts differ */
    var1 = HIGH_1_LOW_1;
    var2 = HIGH_1_LOW_2;
    if (var1 < var2) checksum += 4;  /* Should take */
    if (var2 > var1) checksum += 8;  /* Should take */
    
    /* Test 3: Both parts differ */
    var1 = HIGH_1_LOW_1;
    var2 = HIGH_2_LOW_2;
    if (var1 < var2) checksum += 16; /* Should take */
    if (var2 > var1) checksum += 32; /* Should take */
    
    /* Test 4: Signed negative comparisons */
    var1 = ALL_ONES_128;  /* -1 */
    var2 = 0;
    if (var1 < var2) checksum += 64; /* Should take */
    if (var2 > var1) checksum += 128; /* Should take */
    
    /* Test 5: Unsigned comparisons */
    uvar1 = UHIGH_1_LOW_1;
    uvar2 = UHIGH_2_LOW_0;
    if (uvar1 < uvar2) checksum += 256; /* Should take */
    if (uvar2 > uvar1) checksum += 512; /* Should take */
    
    /* Test 6: Equality comparisons */
    var1 = HIGH_1_LOW_1;
    var2 = HIGH_1_LOW_1;
    if (var1 == var2) checksum += 1024; /* Should take */
    if (var1 <= var2) checksum += 2048; /* Should take */
    if (var1 >= var2) checksum += 4096; /* Should take */
    
    return checksum;
}

/* Constexpr-style function for C (using static inline) */
static inline int constexpr_comparisons(void) {
    const __int128 c1 = HIGH_1_LOW_0;
    const __int128 c2 = HIGH_2_LOW_0;
    const __int128 c3 = HIGH_1_LOW_1;
    const __int128 c4 = HIGH_1_LOW_2;
    
    /* These should be evaluated at compile time with optimization */
    int result = 0;
    result += (c1 < c2) ? 1 : 0;
    result += (c2 > c1) ? 2 : 0;
    result += (c3 < c4) ? 4 : 0;
    result += (c4 > c3) ? 8 : 0;
    result += (c1 == c1) ? 16 : 0;
    
    return result;
}

/* Array size depending on 128-bit comparison */
static char array_size_test[(HIGH_2_LOW_0 > HIGH_1_LOW_0) ? 100 : 200];

int main(void) {
    int checksum = 0;
    
    /* Compile-time evaluations */
    checksum += constexpr_comparisons();
    
    /* Runtime evaluations */
    checksum += runtime_comparisons();
    
    /* Overflow builtin tests */
    checksum += test_overflow_builtins() * 8192;
    
    /* Additional comparisons in main to ensure coverage */
    const unsigned __int128 ua = UHIGH_1_LOW_1;
    const unsigned __int128 ub = UHIGH_1_LOW_2;
    const unsigned __int128 uc = UHIGH_2_LOW_0;
    
    if (ua < ub) checksum += 16384;
    if (ub > ua) checksum += 32768;
    if (ua < uc) checksum += 65536;
    if (uc > ua) checksum += 131072;
    
    /* Signed with high bit set */
    const __int128 sa = HIGH_MSB_SET;  /* Negative */
    const __int128 sb = 0;
    if (sa < sb) checksum += 262144;
    if (sb > sa) checksum += 524288;
    
    printf("Checksum: %d\n", checksum);
    printf("Array size: %zu\n", sizeof(array_size_test));
    
    return 0;
}
