/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force runtime evaluation */
static volatile unsigned __int128 vol_uint128;
static volatile __int128 vol_sint128;

/* Compile-time comparisons using static assertions */
#define STATIC_COMPARE_UINT128(a, b, op, msg) \
    _Static_assert((unsigned __int128)(a) op (unsigned __int128)(b), msg)

#define STATIC_COMPARE_SINT128(a, b, op, msg) \
    _Static_assert((__int128)(a) op (__int128)(b), msg)

/* Test cases that exercise different comparison paths */

/* High part differs, low part equal */
#define HIGH_DIFF_LOW_EQ_A ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define HIGH_DIFF_LOW_EQ_B ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

/* High part equal, low part differs */
#define HIGH_EQ_LOW_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define HIGH_EQ_LOW_DIFF_B ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

/* Both parts differ */
#define BOTH_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define BOTH_DIFF_B ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Edge cases with sign bits for signed comparisons */
#define SIGNED_NEG_ONE ((__int128)-1)  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO    ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)(-0x80000000000000000000000000000000ULL))

/* Compile-time static assertions to force constant folding */
STATIC_COMPARE_UINT128(HIGH_DIFF_LOW_EQ_A, HIGH_DIFF_LOW_EQ_B, <,  "High diff, low eq: A < B");
STATIC_COMPARE_UINT128(HIGH_DIFF_LOW_EQ_B, HIGH_DIFF_LOW_EQ_A, >,  "High diff, low eq: B > A");
STATIC_COMPARE_UINT128(HIGH_EQ_LOW_DIFF_A, HIGH_EQ_LOW_DIFF_B, <,  "High eq, low diff: A < B");
STATIC_COMPARE_UINT128(HIGH_EQ_LOW_DIFF_B, HIGH_EQ_LOW_DIFF_A, >,  "High eq, low diff: B > A");
STATIC_COMPARE_UINT128(BOTH_DIFF_A, BOTH_DIFF_B, <,  "Both diff: A < B");
STATIC_COMPARE_UINT128(BOTH_DIFF_B, BOTH_DIFF_A, >,  "Both diff: B > A");

STATIC_COMPARE_SINT128(SIGNED_NEG_ONE, SIGNED_ZERO, <,  "Signed: -1 < 0");
STATIC_COMPARE_SINT128(SIGNED_ZERO, SIGNED_NEG_ONE, >,  "Signed: 0 > -1");
STATIC_COMPARE_SINT128(SIGNED_LARGE_NEG, SIGNED_LARGE_POS, <, "Signed: large neg < large pos");

/* Constant expressions evaluated at compile-time */
const int ct_result1 = (HIGH_DIFF_LOW_EQ_A < HIGH_DIFF_LOW_EQ_B) ? 1 : 0;
const int ct_result2 = (HIGH_EQ_LOW_DIFF_A > HIGH_EQ_LOW_DIFF_B) ? 1 : 0;
const int ct_result3 = (BOTH_DIFF_A <= BOTH_DIFF_B) ? 1 : 0;
const int ct_result4 = (SIGNED_NEG_ONE >= SIGNED_ZERO) ? 1 : 0;

/* Array size depending on comparison result */
char array1[(HIGH_DIFF_LOW_EQ_A < HIGH_DIFF_LOW_EQ_B) ? 10 : 20];
char array2[(HIGH_EQ_LOW_DIFF_A > HIGH_EQ_LOW_DIFF_B) ? 30 : 40];

/* Test built-in overflow functions that may use comparisons */
int test_builtin_overflow(void) {
    unsigned __int128 a = HIGH_DIFF_LOW_EQ_A;
    unsigned __int128 b = HIGH_DIFF_LOW_EQ_B;
    unsigned __int128 result;
    int overflow;
    
    /* These built-ins may internally compare values */
    overflow = __builtin_mul_overflow(a, (unsigned __int128)2, &result);
    overflow |= __builtin_add_overflow_p(a, b, (unsigned __int128)0);
    
    return overflow;
}

/* Runtime comparisons with volatile variables */
int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Setup volatile variables */
    vol_uint128 = HIGH_DIFF_LOW_EQ_A;
    unsigned __int128 var1 = vol_uint128;
    vol_uint128 = HIGH_DIFF_LOW_EQ_B;
    unsigned __int128 var2 = vol_uint128;
    
    /* Exercise all comparison operators */
    if ((unsigned __int128)var1 < (unsigned __int128)var2) checksum |= 0x01;
    if ((unsigned __int128)var2 > (unsigned __int128)var1) checksum |= 0x02;
    if ((unsigned __int128)var1 <= (unsigned __int128)var2) checksum |= 0x04;
    if ((unsigned __int128)var2 >= (unsigned __int128)var1) checksum |= 0x08;
    if ((unsigned __int128)var1 != (unsigned __int128)var2) checksum |= 0x10;
    
    /* Test high equal, low different */
    vol_uint128 = HIGH_EQ_LOW_DIFF_A;
    var1 = vol_uint128;
    vol_uint128 = HIGH_EQ_LOW_DIFF_B;
    var2 = vol_uint128;
    
    if ((unsigned __int128)var1 < (unsigned __int128)var2) checksum |= 0x20;
    if ((unsigned __int128)var2 > (unsigned __int128)var1) checksum |= 0x40;
    
    /* Test both different */
    vol_uint128 = BOTH_DIFF_A;
    var1 = vol_uint128;
    vol_uint128 = BOTH_DIFF_B;
    var2 = vol_uint128;
    
    if ((unsigned __int128)var1 < (unsigned __int128)var2) checksum |= 0x80;
    if ((unsigned __int128)var2 > (unsigned __int128)var1) checksum |= 0x100;
    
    /* Signed comparisons with sign bit handling */
    vol_sint128 = SIGNED_NEG_ONE;
    __int128 svar1 = vol_sint128;
    vol_sint128 = SIGNED_ZERO;
    __int128 svar2 = vol_sint128;
    
    if (svar1 < svar2) checksum |= 0x200;
    if (svar2 > svar1) checksum |= 0x400;
    
    /* Test equality cases */
    vol_uint128 = HIGH_DIFF_LOW_EQ_A;
    var1 = vol_uint128;
    var2 = var1;  /* Same value */
    
    if ((unsigned __int128)var1 == (unsigned __int128)var2) checksum |= 0x800;
    if ((unsigned __int128)var1 <= (unsigned __int128)var2) checksum |= 0x1000;
    if ((unsigned __int128)var1 >= (unsigned __int128)var2) checksum |= 0x2000;
    
    return checksum;
}

/* Mixed signed/unsigned comparisons */
int mixed_comparisons(void) {
    int checksum = 0;
    
    /* Compare unsigned __int128 with __int128 */
    unsigned __int128 uval = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL; /* Max unsigned */
    __int128 sval = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL; /* Max signed positive */
    
    /* These should trigger unsigned comparison of high parts */
    if (uval > (unsigned __int128)sval) checksum |= 0x01;
    if ((__int128)uval < sval) checksum |= 0x02;  /* uval interpreted as signed is negative */
    
    return checksum;
}

int main(void) {
    printf("Starting double_int comparison tests...\n");
    
    /* Force compile-time evaluation */
    printf("Compile-time results: %d %d %d %d\n", 
           ct_result1, ct_result2, ct_result3, ct_result4);
    
    printf("Array sizes: %zu %zu\n", sizeof(array1), sizeof(array2));
    
    /* Runtime tests */
    int runtime_result = runtime_comparisons();
    printf("Runtime comparison checksum: 0x%x\n", runtime_result);
    
    int mixed_result = mixed_comparisons();
    printf("Mixed signed/unsigned checksum: 0x%x\n", mixed_result);
    
    int overflow_result = test_builtin_overflow();
    printf("Built-in overflow test result: %d\n", overflow_result);
    
    /* Additional complex compile-time expression */
    const __int128 complex_ct = (SIGNED_LARGE_POS > SIGNED_LARGE_NEG) ? 
                                SIGNED_LARGE_POS : SIGNED_LARGE_NEG;
    printf("Complex compile-time expression evaluated\n");
    
    /* Test comparisons in loop (may be optimized differently) */
    unsigned __int128 loop_sum = 0;
    for (unsigned __int128 i = 0; i < 100; i++) {
        if (i < HIGH_EQ_LOW_DIFF_A) {
            loop_sum += i;
        }
    }
    printf("Loop sum (truncated): %llu\n", (unsigned long long)loop_sum);
    
    printf("All tests completed.\n");
    return 0;
}
