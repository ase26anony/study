/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Force runtime evaluation */
static volatile __int128 vol_sint128;
static volatile unsigned __int128 vol_uint128;

/* Compile-time constants with varying high/low parts */
#define HIGH1_LOW1 ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define HIGH1_LOW2 ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */
#define HIGH2_LOW1 ((unsigned __int128)0x20000000000000001ULL)  /* high=2, low=1 */
#define HIGH2_LOW2 ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */
#define HIGH0_LOW_MAX ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) /* high=0, low=UINT64_MAX */
#define HIGH1_LOW0 ((unsigned __int128)0x10000000000000000ULL)   /* high=1, low=0 */

/* Signed constants with sign bit implications */
#define SINT_NEG1 ((__int128)-1)  /* All bits set: high=UINT64_MAX, low=UINT64_MAX */
#define SINT_MIN  ((__int128)((unsigned __int128)1 << 127))  /* MSB set */
#define SINT_MAX  ((__int128)(((unsigned __int128)1 << 127) - 1))

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH1_LOW1 < HIGH2_LOW1, "High part less comparison");
_Static_assert(HIGH2_LOW1 > HIGH1_LOW1, "High part greater comparison");
_Static_assert(HIGH1_LOW1 < HIGH1_LOW2, "Low part less comparison");
_Static_assert(HIGH1_LOW2 > HIGH1_LOW1, "Low part greater comparison");
_Static_assert(HIGH1_LOW1 != HIGH2_LOW2, "Both parts differ");
_Static_assert(SINT_NEG1 < 0, "Signed negative comparison");
_Static_assert(SINT_MAX > 0, "Signed positive comparison");

/* Compile-time constant expressions */
const int ct_result1 = (HIGH1_LOW1 <= HIGH1_LOW2) ? 1 : 0;
const int ct_result2 = (HIGH2_LOW1 >= HIGH1_LOW1) ? 2 : 0;
const int ct_result3 = (SINT_NEG1 == (__int128)-1) ? 4 : 0;
const int ct_result4 = (SINT_MIN < SINT_MAX) ? 8 : 0;

/* Array size depending on comparison */
char array1[(HIGH1_LOW1 < HIGH2_LOW1) ? 16 : 32];
char array2[(HIGH1_LOW2 > HIGH1_LOW1) ? 64 : 128];

/* Test unsigned comparisons */
static unsigned test_unsigned_comparisons(void) {
    unsigned checksum = 0;
    
    /* High part differs, low part equal */
    if (HIGH1_LOW0 < HIGH2_LOW1) checksum |= 0x01;
    if (HIGH2_LOW1 > HIGH1_LOW0) checksum |= 0x02;
    
    /* High part equal, low part differs */
    if (HIGH1_LOW1 < HIGH1_LOW2) checksum |= 0x04;
    if (HIGH1_LOW2 > HIGH1_LOW1) checksum |= 0x08;
    
    /* Both parts differ */
    if (HIGH1_LOW1 < HIGH2_LOW2) checksum |= 0x10;
    if (HIGH2_LOW2 > HIGH1_LOW1) checksum |= 0x20;
    
    /* Edge cases */
    if (HIGH0_LOW_MAX < HIGH1_LOW0) checksum |= 0x40;  /* high: 0 < 1 */
    if (HIGH1_LOW0 > HIGH0_LOW_MAX) checksum |= 0x80;
    
    return checksum;
}

/* Test signed comparisons */
static unsigned test_signed_comparisons(void) {
    unsigned checksum = 0;
    const __int128 s_a = SINT_NEG1;  /* All bits set */
    const __int128 s_b = 0;
    const __int128 s_c = SINT_MAX;
    const __int128 s_d = SINT_MIN;
    
    /* Signed comparisons that use unsigned high-part comparison */
    if (s_a < s_b) checksum |= 0x01;  /* -1 < 0 */
    if (s_b > s_a) checksum |= 0x02;  /* 0 > -1 */
    
    if (s_c > s_b) checksum |= 0x04;  /* MAX > 0 */
    if (s_d < s_b) checksum |= 0x08;  /* MIN < 0 */
    
    /* Mixed high/low comparisons */
    const __int128 s_e = ((__int128)0x10000000000000001ULL);
    const __int128 s_f = ((__int128)0x20000000000000002ULL);
    
    if (s_e < s_f) checksum |= 0x10;
    if (s_f > s_e) checksum |= 0x20;
    
    return checksum;
}

/* Test runtime comparisons with volatile */
static unsigned test_runtime_comparisons(void) {
    unsigned checksum = 0;
    
    /* Initialize volatile variables */
    vol_uint128 = HIGH1_LOW1;
    unsigned __int128 runtime_uint_a = vol_uint128;
    vol_uint128 = HIGH2_LOW2;
    unsigned __int128 runtime_uint_b = vol_uint128;
    
    vol_sint128 = SINT_NEG1;
    __int128 runtime_sint_a = vol_sint128;
    vol_sint128 = 0;
    __int128 runtime_sint_b = vol_sint128;
    
    /* Runtime unsigned comparisons */
    if (runtime_uint_a < runtime_uint_b) checksum |= 0x01;
    if (runtime_uint_b > runtime_uint_a) checksum |= 0x02;
    if (runtime_uint_a <= runtime_uint_b) checksum |= 0x04;
    if (runtime_uint_b >= runtime_uint_a) checksum |= 0x08;
    
    /* Runtime signed comparisons */
    if (runtime_sint_a < runtime_sint_b) checksum |= 0x10;
    if (runtime_sint_b > runtime_sint_a) checksum |= 0x20;
    if (runtime_sint_a <= runtime_sint_b) checksum |= 0x40;
    if (runtime_sint_b >= runtime_sint_a) checksum |= 0x80;
    
    return checksum;
}

/* Test GCC builtins that may use double_int comparisons */
static unsigned test_builtin_comparisons(void) {
    unsigned checksum = 0;
    
    /* Test overflow builtins */
    __int128 mul_result;
    int overflow = __builtin_mul_overflow(
        (__int128)0x7FFFFFFFFFFFFFFFULL,
        (__int128)2,
        &mul_result
    );
    if (overflow) checksum |= 0x01;
    
    /* Test with unsigned __int128 */
    unsigned __int128 a = HIGH1_LOW1;
    unsigned __int128 b = HIGH2_LOW2;
    if (__builtin_add_overflow_p(a, b, (unsigned __int128)0)) {
        checksum |= 0x02;
    }
    
    /* Compare builtin */
    if (__builtin_isless(a, b)) checksum |= 0x04;
    
    return checksum;
}

/* Test mixed signed/unsigned comparisons */
static unsigned test_mixed_comparisons(void) {
    unsigned checksum = 0;
    
    /* These may trigger different comparison paths */
    const __int128 signed_val = -1;
    const unsigned __int128 unsigned_val = 0xFFFFFFFFFFFFFFFFULL; /* Same bit pattern */
    
    /* Direct comparisons between signed and unsigned */
    if ((unsigned __int128)signed_val == unsigned_val) checksum |= 0x01;
    if ((__int128)unsigned_val != signed_val) checksum |= 0x02;
    
    /* Comparisons through conditionals */
    unsigned __int128 temp = signed_val;  /* Implicit conversion */
    if (temp == unsigned_val) checksum |= 0x04;
    
    return checksum;
}

int main(void) {
    unsigned total_checksum = 0;
    
    /* Include compile-time results */
    total_checksum += ct_result1 + ct_result2 + ct_result3 + ct_result4;
    
    /* Run all comparison tests */
    total_checksum += test_unsigned_comparisons();
    total_checksum += test_signed_comparisons();
    total_checksum += test_runtime_comparisons();
    total_checksum += test_builtin_comparisons();
    total_checksum += test_mixed_comparisons();
    
    /* Prevent dead code elimination */
    printf("Array sizes: %zu, %zu\n", sizeof(array1), sizeof(array2));
    printf("Checksum: 0x%08x\n", total_checksum);
    
    /* Verify some expected results */
    if (total_checksum != 0) {
        printf("All comparison tests completed.\n");
    }
    
    return 0;
}
