/* test-double-int.c - Stress GCC's 128-bit integer comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_U64       0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Large constants for comparison */
static const __int128 NEG_LARGE = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;  /* High negative, low negative */
static const __int128 POS_LARGE = ((__int128)MAX_U64 << 32) | 0x12345678ULL;    /* High positive, low positive */
static const __int128 MIXED_A   = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | HIGH_BIT_64;  /* High positive, low negative */
static const __int128 MIXED_B   = ((__int128)HIGH_BIT_64 << 64) | 0x7FFFFFFFFFFFFFFFULL;  /* High negative, low positive */

static const unsigned __int128 UMAX_128 = ((unsigned __int128)MAX_U64 << 64) | MAX_U64;
static const unsigned __int128 UMID_128 = ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)1 << 120) > ((__int128)1 << 119), "128-bit shift compare 1");
_Static_assert((unsigned __int128)UMAX_128 > UMID_128, "128-bit unsigned compare 1");

/* Test 1: Comparisons where high words differ */
static int test_high_word_diff(void) {
    int checksum = 0;
    __int128 vals[] = {
        ((__int128)HIGH_BIT_64 << 64),      /* Very negative */
        ((__int128)0x4000000000000000ULL << 64),  /* Less negative */
        0,                                   /* Zero */
        ((__int128)0x4000000000000000ULL << 64) | 1,  /* Positive high, low=1 */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_U64  /* Largest positive */
    };
    
    for (int i = 0; i < 4; i++) {
        if (vals[i] < vals[i+1]) checksum += 1;    /* Should trigger high word compare */
        if (vals[i] > vals[i+1]) checksum -= 1;
        if (vals[i] == vals[i+1]) checksum += 100; /* Shouldn't happen */
    }
    
    return checksum;
}

/* Test 2: Comparisons where high words equal, low words differ */
static int test_low_word_diff(void) {
    int checksum = 0;
    const __int128 base_high = ((__int128)0x123456789ABCDEF0ULL << 64);
    
    __int128 vals[] = {
        base_high | 0,
        base_high | 1,
        base_high | 0xFFFFFFFFULL,
        base_high | HIGH_BIT_64,
        base_high | MAX_U64
    };
    
    for (int i = 0; i < 4; i++) {
        if (vals[i] < vals[i+1]) checksum += 2;    /* Should trigger low word compare */
        if (vals[i] > vals[i+1]) checksum -= 2;
    }
    
    /* Mixed signed/unsigned comparisons */
    unsigned __int128 ubase = ((unsigned __int128)0x123456789ABCDEF0ULL << 64);
    unsigned __int128 uvals[] = {
        ubase | 0,
        ubase | HIGH_BIT_64,
        ubase | MAX_U64
    };
    
    for (int i = 0; i < 2; i++) {
        if (uvals[i] < uvals[i+1]) checksum += 3;
        if (uvals[i] > uvals[i+1]) checksum -= 3;
    }
    
    return checksum;
}

/* Test 3: Boundary value comparisons */
static int test_boundaries(void) {
    int checksum = 0;
    
    /* Signed boundaries */
    __int128 s_min = ((__int128)HIGH_BIT_64 << 64);
    __int128 s_max = ~s_min;
    __int128 zero = 0;
    __int128 neg_one = -1;
    
    if (s_min < s_max) checksum += 4;
    if (s_min < zero) checksum += 4;
    if (neg_one < zero) checksum += 4;
    if (s_max > zero) checksum += 4;
    
    /* Unsigned boundaries */
    unsigned __int128 u_min = 0;
    unsigned __int128 u_max = UMAX_128;
    unsigned __int128 u_mid = u_max / 2;
    
    if (u_min < u_max) checksum += 5;
    if (u_mid < u_max) checksum += 5;
    if (u_min < u_mid) checksum += 5;
    
    /* Cross-type comparisons */
    if ((unsigned __int128)zero == u_min) checksum += 6;
    if ((__int128)u_mid > zero) checksum += 6;
    
    return checksum;
}

/* Test 4: Arithmetic with overflow checking */
static int test_overflow_checks(void) {
    int checksum = 0;
    
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b = 1;
    __int128 c = -1;
    
    /* These may trigger overflow analysis */
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 prod = a * 2;
    
    if (sum > a) checksum += 7;      /* Overflow in signed */
    if (diff < a) checksum += 7;
    
    /* Use builtins for overflow checking */
    __int128 of_result;
    if (__builtin_add_overflow(a, b, &of_result)) checksum += 8;
    if (__builtin_mul_overflow(a, 2, &of_result)) checksum += 8;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 shifted = a << 2;
    __int128 masked = a & (((__int128)0xFFFFULL << 64) | 0xFFFFULL);
    
    if (shifted > a) checksum += 9;
    if (masked < a) checksum += 9;
    
    return checksum;
}

/* Test 5: Range analysis triggers with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = ((__int128)HIGH_BIT_64 << 62); 
         i < ((__int128)HIGH_BIT_64 << 61); 
         i += ((__int128)1 << 60)) {
        if (i < 0) checksum += 10;
        else checksum -= 10;
    }
    
    /* Value-dependent return */
    __int128 arr[] = {
        NEG_LARGE, POS_LARGE, MIXED_A, MIXED_B,
        ((__int128)1 << 127) - 1,
        -((__int128)1 << 127),
        0,
        ~(__int128)0
    };
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (arr[i] < arr[j]) checksum += 1;
            if (arr[i] > arr[j]) checksum -= 1;
            if (arr[i] == arr[j]) checksum += 2;
        }
    }
    
    return checksum;
}

/* Test 6: Mixed precision and conversions */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    long long ll_max = LLONG_MAX;
    unsigned long long ull_max = ULLONG_MAX;
    size_t size_max = SIZE_MAX;
    
    __int128 x = ll_max;
    __int128 y = ull_max;
    __int128 z = size_max;
    
    /* Comparisons with narrower types */
    if (x > ll_max) checksum += 11;
    if (y < ull_max) checksum += 11;
    if (z == size_max) checksum += 11;
    
    /* Ternary operator mixing types */
    __int128 t1 = (ll_max > 0) ? ((__int128)ll_max << 64) : -((__int128)ll_max << 64);
    __int128 t2 = (ull_max > 0) ? (__int128)ull_max : (__int128)-1;
    
    if (t1 > t2) checksum += 12;
    if (t1 < t2) checksum += 12;
    
    /* Variadic function argument (triggers conversions) */
    printf("Mixed precision: %lld vs %llu\n", 
           (long long)(x >> 64), 
           (unsigned long long)(y & ULLONG_MAX));
    
    return checksum;
}

/* Test 7: Builtin functions */
static int test_builtins(void) {
    int checksum = 0;
    
    unsigned __int128 uval = UMID_128;
    __int128 sval = MIXED_A;
    
    /* Bit counting operations */
    int clz_high = __builtin_clzll((unsigned long long)(uval >> 64));
    int ctz_low = __builtin_ctzll((unsigned long long)uval);
    int popcount = __builtin_popcountll((unsigned long long)(uval >> 64)) +
                   __builtin_popcountll((unsigned long long)uval);
    
    checksum += clz_high + ctz_low + popcount;
    
    /* Byte swap simulation */
    unsigned __int128 swapped = 0;
    for (int i = 0; i < 16; i++) {
        swapped |= ((uval >> (8*i)) & 0xFF) << (8*(15-i));
    }
    
    if (swapped != uval) checksum += 13;
    
    /* Branch prediction with wide comparisons */
    if (__builtin_expect(sval < 0, 1)) checksum += 14;
    if (__builtin_expect(uval > (UMAX_128 >> 1), 0)) checksum += 14;
    
    return checksum;
}

/* Test 8: Switch statement with __int128 cases */
static int test_switch_cases(__int128 val) {
    int result = 0;
    
    switch (val) {
        case ((__int128)0):
            result = 1;
            break;
        case ((__int128)1 << 64):
            result = 2;
            break;
        case ((__int128)HIGH_BIT_64 << 64):
            result = 3;
            break;
        case ((__int128)0x7FFFFFFFFFFFFFFFULL << 64):
            result = 4;
            break;
        default:
            result = 5;
            break;
    }
    
    return result;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Starting 128-bit comparison stress tests...\n");
    
    /* Run all tests */
    total_checksum += test_high_word_diff();
    total_checksum += test_low_word_diff();
    total_checksum += test_boundaries();
    total_checksum += test_overflow_checks();
    total_checksum += test_range_analysis();
    total_checksum += test_mixed_precision();
    total_checksum += test_builtins();
    
    /* Test switch with various values */
    __int128 switch_vals[] = {
        0,
        ((__int128)1 << 64),
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64),
        ((__int128)1)
    };
    
    for (int i = 0; i < 5; i++) {
        total_checksum += test_switch_cases(switch_vals[i]);
    }
    
    /* Final mixed comparison to ensure all code is used */
    volatile __int128 v1 = NEG_LARGE;
    volatile __int128 v2 = POS_LARGE;
    volatile __int128 v3 = MIXED_A;
    volatile __int128 v4 = MIXED_B;
    
    if (v1 < v2) total_checksum += 100;
    if (v3 > v4) total_checksum += 200;
    if ((v1 == NEG_LARGE) && (v2 == POS_LARGE)) total_checksum += 300;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
