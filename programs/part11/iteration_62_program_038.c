/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Define large 128-bit constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A   (((__int128)0x1ULL) << 64)      /* 0x10000000000000000 */
#define HIGH_DIFF_LOW_EQUAL_B   (((__int128)0x2ULL) << 64)      /* 0x20000000000000000 */

#define HIGH_EQUAL_LOW_DIFF_A   ((((__int128)0x1ULL) << 64) | 0x1ULL)  /* 0x10000000000000001 */
#define HIGH_EQUAL_LOW_DIFF_B   ((((__int128)0x1ULL) << 64) | 0x2ULL)  /* 0x10000000000000002 */

#define BOTH_PARTS_DIFF_A       ((((__int128)0x1ULL) << 64) | 0x1ULL)  /* 0x10000000000000001 */
#define BOTH_PARTS_DIFF_B       ((((__int128)0x2ULL) << 64) | 0x2ULL)  /* 0x20000000000000002 */

#define SIGNED_NEGATIVE         ((__int128)-1)                  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO             ((__int128)0)

/* Unsigned versions */
#define UHIGH_DIFF_LOW_EQUAL_A  ((unsigned __int128)0x1ULL << 64)
#define UHIGH_DIFF_LOW_EQUAL_B  ((unsigned __int128)0x2ULL << 64)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High part less comparison should pass");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B, 
               "Low part less comparison should pass");
_Static_assert(BOTH_PARTS_DIFF_A < BOTH_PARTS_DIFF_B, 
               "Both parts less comparison should pass");
_Static_assert(SIGNED_NEGATIVE < SIGNED_ZERO, 
               "Signed negative < zero should pass");
_Static_assert(UHIGH_DIFF_LOW_EQUAL_A < UHIGH_DIFF_LOW_EQUAL_B,
               "Unsigned high part less should pass");

/* Compile-time comparisons in constant expressions */
const int cmp_high_greater = (HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A) ? 1 : 0;
const int cmp_low_greater = (HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A) ? 1 : 0;
const int cmp_both_greater = (BOTH_PARTS_DIFF_B > BOTH_PARTS_DIFF_A) ? 1 : 0;
const int cmp_signed_eq = (SIGNED_NEGATIVE == SIGNED_NEGATIVE) ? 1 : 0;

/* Array size based on comparison */
char arr_high_less[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char arr_low_less[(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 15 : 25];

/* Runtime comparison function */
int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Volatile variables to force runtime evaluation */
    volatile __int128 v1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile __int128 v2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile __int128 v3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile __int128 v4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile __int128 v5 = BOTH_PARTS_DIFF_A;
    volatile __int128 v6 = BOTH_PARTS_DIFF_B;
    volatile __int128 v7 = SIGNED_NEGATIVE;
    volatile __int128 v8 = SIGNED_ZERO;
    
    volatile unsigned __int128 uv1 = UHIGH_DIFF_LOW_EQUAL_A;
    volatile unsigned __int128 uv2 = UHIGH_DIFF_LOW_EQUAL_B;
    
    /* Test all comparison operators with different combinations */
    
    /* 1. High part less, low equal */
    if (v1 < v2) checksum += 1;    /* Should take */
    if (v1 > v2) checksum += 100;  /* Should NOT take */
    if (v1 <= v2) checksum += 2;   /* Should take */
    if (v1 >= v2) checksum += 200; /* Should NOT take */
    
    /* 2. High equal, low less */
    if (v3 < v4) checksum += 4;    /* Should take */
    if (v3 > v4) checksum += 400;  /* Should NOT take */
    
    /* 3. Both parts less */
    if (v5 < v6) checksum += 8;    /* Should take */
    if (v5 > v6) checksum += 800;  /* Should NOT take */
    
    /* 4. High part greater (reverse of case 1) */
    if (v2 > v1) checksum += 16;   /* Should take */
    if (v2 < v1) checksum += 1600; /* Should NOT take */
    
    /* 5. High equal, low greater (reverse of case 2) */
    if (v4 > v3) checksum += 32;   /* Should take */
    if (v4 < v3) checksum += 3200; /* Should NOT take */
    
    /* 6. Both parts greater (reverse of case 3) */
    if (v6 > v5) checksum += 64;   /* Should take */
    if (v6 < v5) checksum += 6400; /* Should NOT take */
    
    /* 7. Signed comparisons with negative values */
    if (v7 < v8) checksum += 128;  /* Should take (negative < 0) */
    if (v7 > v8) checksum += 12800;/* Should NOT take */
    if (v7 <= v8) checksum += 256; /* Should take */
    if (v7 >= v8) checksum += 25600;/* Should NOT take */
    
    /* 8. Equality comparisons */
    if (v1 == v1) checksum += 512; /* Should take */
    if (v1 != v2) checksum += 1024;/* Should take */
    
    /* 9. Unsigned comparisons */
    if (uv1 < uv2) checksum += 2048;   /* Should take */
    if (uv1 > uv2) checksum += 4096;   /* Should NOT take */
    if (uv1 <= uv2) checksum += 8192;  /* Should take */
    if (uv1 >= uv2) checksum += 16384; /* Should NOT take */
    
    return checksum;
}

/* Test built-in overflow functions that may use comparisons */
int test_builtin_overflow(void) {
    int checksum = 0;
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL) << 1;
    __int128 b = 1;
    __int128 result;
    
    /* These built-ins may perform comparisons internally */
    if (__builtin_add_overflow(a, b, &result)) {
        checksum += 1;  /* May trigger on overflow */
    }
    
    __int128 c = ((__int128)0x1ULL) << 120;
    __int128 d = ((__int128)0x1ULL) << 120;
    
    if (__builtin_mul_overflow(c, d, &result)) {
        checksum += 2;  /* May trigger on overflow */
    }
    
    return checksum;
}

/* C++ version with constexpr (compile as .cpp to use) */
#ifdef __cplusplus
constexpr bool constexpr_compare_high_less() {
    const __int128 a = HIGH_DIFF_LOW_EQUAL_A;
    const __int128 b = HIGH_DIFF_LOW_EQUAL_B;
    return a < b;
}

constexpr bool constexpr_compare_low_less() {
    const __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    const __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    return a < b;
}

constexpr bool constexpr_compare_signed() {
    const __int128 a = SIGNED_NEGATIVE;
    const __int128 b = SIGNED_ZERO;
    return a < b;
}

/* Force compile-time evaluation */
constexpr bool ct_high_less = constexpr_compare_high_less();
constexpr bool ct_low_less = constexpr_compare_low_less();
constexpr bool ct_signed = constexpr_compare_signed();
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_greater;
    total_checksum += cmp_low_greater;
    total_checksum += cmp_both_greater;
    total_checksum += cmp_signed_eq;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in overflow tests */
    total_checksum += test_builtin_overflow();
    
#ifdef __cplusplus
    /* Add C++ constexpr results */
    total_checksum += ct_high_less ? 32768 : 0;
    total_checksum += ct_low_less ? 65536 : 0;
    total_checksum += ct_signed ? 131072 : 0;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", 
           sizeof(arr_high_less), sizeof(arr_low_less));
    
    /* Verify expected values */
    assert(cmp_high_greater == 1);
    assert(cmp_low_greater == 1);
    assert(cmp_both_greater == 1);
    assert(cmp_signed_eq == 1);
    
    return 0;
}
