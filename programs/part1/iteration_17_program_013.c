/* test-double-int.c - Target coverage for double-int.cc lines 1285-1293 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x7FFFFFFFFFFFFFFFULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)1 << 64) > 0, "128-bit shift should work");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit value");

/* Test function that exercises range analysis with __int128 */
static __int128 process_range(__int128 start, __int128 end, int iterations) {
    __int128 sum = 0;
    __int128 step = (end - start) / iterations;
    
    for (__int128 i = start; i < end; i += step) {
        /* Force comparisons that may use double_int::cmp */
        if (i < 0) {
            sum += -i;
        } else if (i > (__int128)HIGH_BIT_64 << 64) {
            sum += i >> 1;
        } else {
            sum += i;
        }
        
        /* Cross 64-bit boundary comparisons */
        if ((unsigned __int128)i < (unsigned __int128)(HIGH_BIT_64 << 64)) {
            sum += 1;
        }
    }
    return sum;
}

/* Test overflow operations that require wide comparisons */
static int check_overflow(__int128 a, __int128 b) {
    __int128 result;
    int overflow = __builtin_add_overflow(a, b, &result);
    
    /* These comparisons should trigger the uncovered code */
    if (a < b) return -1;
    if (a > b) return 1;
    if ((unsigned __int128)a < (unsigned __int128)b) return -2;
    if ((unsigned __int128)a > (unsigned __int128)b) return 2;
    
    return overflow ? 3 : 0;
}

/* Mixed-precision comparisons */
static int mixed_comparisons(__int128 a, unsigned long long b) {
    /* Force conversions and comparisons */
    if (a < (__int128)b) return -1;
    if (a > (__int128)b) return 1;
    if ((unsigned __int128)a < (unsigned __int128)b) return -2;
    if ((unsigned __int128)a > (unsigned __int128)b) return 2;
    return 0;
}

/* Switch with __int128 cases - forces compile-time comparisons */
static const char* classify_int128(__int128 val) {
    /* These constants cross 64-bit boundaries */
    const __int128 case1 = (__int128)HIGH_BIT_64 << 32;  /* 0x800000000000000000000000 */
    const __int128 case2 = (__int128)MAX_64 << 64;       /* 0xFFFFFFFFFFFFFFFF0000000000000000 */
    const __int128 case3 = (__int128)MID_128 << 60;      /* 0x7FFFFFFFFFFFFFFF0000000000000000 */
    const __int128 case4 = -((__int128)HIGH_BIT_64 << 64); /* Most negative */
    
    /* GCC must generate comparison trees for these */
    if (val == case1) return "CASE1";
    if (val == case2) return "CASE2";
    if (val == case3) return "CASE3";
    if (val == case4) return "CASE4";
    
    /* Range comparisons */
    if (val < 0) return "NEGATIVE";
    if (val < (__int128)HIGH_BIT_64 << 32) return "SMALL_POS";
    if (val < (__int128)MAX_64 << 64) return "MEDIUM_POS";
    return "LARGE_POS";
}

/* Bitwise operations crossing 64-bit boundary */
static __int128 bitwise_ops(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Operations that affect both high and low words */
    result = (a << 65) | (b >> 63);
    result &= ~((__int128)MAX_64 << 60);
    result ^= (__int128)0x123456789ABCDEF0ULL << 64;
    
    return result;
}

/* Use __builtin_expect with wide comparisons */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

static int predictable_comparison(__int128 a, __int128 b) {
    if (LIKELY((unsigned __int128)a < (unsigned __int128)b)) {
        return -1;
    }
    if (UNLIKELY(a < b)) {
        return 1;
    }
    return 0;
}

/* Main test harness */
int main(void) {
    volatile __int128 checksum = 0;  /* volatile to prevent elimination */
    
    /* Test array with values that exercise different comparison paths */
    __int128 test_values[] = {
        0,
        1,
        -1,
        (__int128)HIGH_BIT_64,                    /* 0x8000000000000000 */
        (__int128)HIGH_BIT_64 << 1,               /* Cross 64-bit boundary */
        (__int128)MAX_64,                         /* 0xFFFFFFFFFFFFFFFF */
        (__int128)MAX_64 << 64,                   /* High word only */
        (__int128)HIGH_BIT_64 << 64,              /* Negative with high word */
        -((__int128)HIGH_BIT_64 << 64),           /* Most negative 128-bit */
        (__int128)0x123456789ABCDEF0ULL << 32,    /* Cross boundary */
    };
    
    const int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Test 1: Pairwise comparisons */
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            /* These should trigger high-word comparisons */
            if (test_values[i] < test_values[j]) checksum += 1;
            if (test_values[i] > test_values[j]) checksum += 2;
            if ((unsigned __int128)test_values[i] < (unsigned __int128)test_values[j]) checksum += 4;
            if ((unsigned __int128)test_values[i] > (unsigned __int128)test_values[j]) checksum += 8;
        }
    }
    
    /* Test 2: Range analysis with loops */
    checksum += process_range(-((__int128)HIGH_BIT_64 << 32),
                              (__int128)HIGH_BIT_64 << 32,
                              100);
    
    /* Test 3: Overflow checking */
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            checksum += check_overflow(test_values[i], test_values[j]);
        }
    }
    
    /* Test 4: Mixed precision */
    for (int i = 0; i < num_tests; i++) {
        checksum += mixed_comparisons(test_values[i], (unsigned long long)test_values[i]);
        checksum += mixed_comparisons(test_values[i], HIGH_BIT_64);
        checksum += mixed_comparisons(test_values[i], MAX_64);
    }
    
    /* Test 5: Bitwise operations */
    for (int i = 0; i < num_tests; i += 2) {
        checksum += bitwise_ops(test_values[i], test_values[i + 1]);
    }
    
    /* Test 6: Classification */
    for (int i = 0; i < num_tests; i++) {
        const char *cls = classify_int128(test_values[i]);
        checksum += (__int128)cls[0];  /* Use pointer value as checksum */
    }
    
    /* Test 7: Builtin usage with wide ints */
    for (int i = 0; i < num_tests; i++) {
        unsigned __int128 uval = (unsigned __int128)test_values[i];
        /* These may trigger internal conversions */
        checksum += __builtin_popcountll((unsigned long long)(uval >> 64));
        checksum += __builtin_popcountll((unsigned long long)uval);
        checksum += __builtin_clzll((unsigned long long)(uval >> 64));
        checksum += predictable_comparison(test_values[i], test_values[(i + 1) % num_tests]);
    }
    
    /* Test 8: Ternary operations with mixed types */
    for (int i = 0; i < num_tests; i++) {
        __int128 result = (test_values[i] < 0) ? 
                         (__int128)((unsigned long long)-test_values[i]) :
                         (__int128)((unsigned long long)test_values[i]);
        checksum += result;
    }
    
    /* Test 9: Variadic function (triggers conversions) */
    for (int i = 0; i < num_tests && i < 4; i++) {
        /* Force conversion sequences */
        printf("Value[%d]: high=%lld low=%lld\n", 
               i,
               (long long)(test_values[i] >> 64),
               (long long)test_values[i]);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: high=%lld low=%lld\n",
           (long long)(checksum >> 64),
           (long long)checksum);
    
    return 0;
}
