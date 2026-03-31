/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should produce positive 128-bit value");

/* Test 1: Comparisons where high words differ */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Create values where only high words differ */
    __int128 a = ((__int128)1 << 64) | 0x123456789ABCDEF0ULL;
    __int128 b = ((__int128)2 << 64) | 0x123456789ABCDEF0ULL;
    __int128 c = ((__int128)-1 << 64) | 0x123456789ABCDEF0ULL;
    __int128 d = ((__int128)-2 << 64) | 0x123456789ABCDEF0ULL;
    
    /* These comparisons should trigger high-word comparison path */
    if (a < b) checksum += 1;  // high: 1 < 2
    if (c > d) checksum += 2;  // high: -1 > -2 (as unsigned: 0xFFFFFFFFFFFFFFFF > 0xFFFFFFFFFFFFFFFE)
    if (b > a) checksum += 4;  // high: 2 > 1
    
    /* Mix signed and unsigned comparisons */
    unsigned __int128 ua = (unsigned __int128)a;
    unsigned __int128 ub = (unsigned __int128)b;
    if (ua < ub) checksum += 8;  // Should use unsigned high-word comparison
    
    return checksum;
}

/* Test 2: Comparisons where high words are equal but low words differ */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    __int128 a = ((__int128)0x12345678 << 64) | 0x1111111111111111ULL;
    __int128 b = ((__int128)0x12345678 << 64) | 0x2222222222222222ULL;
    __int128 c = ((__int128)0x12345678 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* These should trigger low-word comparison path after high words are equal */
    if (a < b) checksum += 16;   // low: 0x1111... < 0x2222...
    if (c > b) checksum += 32;   // low: 0xFFFF... > 0x2222...
    if (b > a) checksum += 64;   // low: 0x2222... > 0x1111...
    
    /* Test with negative low words but same high word */
    __int128 d = ((__int128)-1 << 64) | 0x7FFFFFFFFFFFFFFFULL;
    __int128 e = ((__int128)-1 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    if (d < e) checksum += 128;  // low: 0x7F... < 0xFF... (both negative in signed context)
    
    return checksum;
}

/* Test 3: Boundary value comparisons */
static int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Define boundary values */
    __int128 int128_max = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 int128_min = ((__int128)0x8000000000000000ULL << 64);
    __int128 zero = 0;
    __int128 minus_one = -1;
    
    unsigned __int128 uint128_max = ~(unsigned __int128)0;
    
    /* Exercise various boundary comparisons */
    if (int128_max > zero) checksum += 256;
    if (int128_min < zero) checksum += 512;
    if (minus_one < zero) checksum += 1024;
    
    /* Unsigned comparisons at boundaries */
    unsigned __int128 uzero = 0;
    if (uint128_max > uzero) checksum += 2048;
    if ((unsigned __int128)minus_one == uint128_max) checksum += 4096;
    
    /* Test near overflow boundaries */
    __int128 near_max = int128_max - 1;
    __int128 near_min = int128_min + 1;
    if (near_max < int128_max) checksum += 8192;
    if (near_min > int128_min) checksum += 16384;
    
    return checksum;
}

/* Test 4: Range analysis triggers with loops */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable crossing 64-bit boundary */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFFF0ULL << 64); 
         i < ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) + 10; 
         i++) {
        checksum += (i & 1) ? 1 : 0;
    }
    
    /* Another loop that forces VRP to track 128-bit ranges */
    __int128 start = ((__int128)-10 << 64);
    __int128 end = ((__int128)10 << 64);
    for (__int128 j = start; j < end; j += ((__int128)1 << 60)) {
        if (j > 0) checksum += 2;
        else if (j < 0) checksum += 3;
        else checksum += 5;
    }
    
    return checksum;
}

/* Test 5: Mixed-precision operations and conversions */
static int test_mixed_precision(void) {
    int checksum = 0;
    
    __int128 a = LARGE_CONSTANT_128;
    unsigned __int128 ua = (unsigned __int128)a;
    
    /* Compare with narrower types */
    long long ll_max = LLONG_MAX;
    unsigned long long ull_max = ULLONG_MAX;
    
    if (a > ll_max) checksum += 32768;
    if (ua > ull_max) checksum += 65536;
    
    /* Ternary operator with mixed types */
    __int128 result = (checksum & 1) ? a : (__int128)ll_max;
    if (result == a) checksum += 131072;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 shifted = a << 32;
    __int128 masked = shifted & (((__int128)0xFFFF << 96) | 0xFFFF);
    
    if (masked != 0) checksum += 262144;
    
    /* Overflow checking with builtins */
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64);
    __int128 y = 1;
    __int128 overflow_result;
    
    if (__builtin_add_overflow(x, y, &overflow_result)) {
        checksum += 524288;  // Should overflow
    }
    
    return checksum;
}

/* Test 6: Compiler built-in functions and switch statements */
static int test_builtins_and_switch(void) {
    int checksum = 0;
    
    __int128 values[] = {
        0,
        ((__int128)1 << 64),
        ((__int128)-1 << 64),
        ((__int128)0x12345678 << 64) | 0x9ABCDEF0,
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    };
    
    /* Use builtins that may trigger double_int comparisons */
    for (int i = 0; i < 5; i++) {
        unsigned __int128 uval = (unsigned __int128)values[i];
        
        /* Count leading zeros - may trigger comparisons */
        if (uval != 0) {
            int clz = __builtin_clzll(uval >> 64);
            if (clz == 64) {
                clz += __builtin_clzll((unsigned long long)uval);
            }
            checksum += clz;
        }
        
        /* Use __builtin_expect with 128-bit comparisons */
        if (__builtin_expect(values[i] > 0, 1)) {
            checksum += i * 1024;
        }
    }
    
    /* Switch statement with __int128 case labels (compile-time constants) */
    __int128 switch_val = ((__int128)0x12345678 << 64) | 0x9ABCDEF0;
    
    switch (switch_val) {
        case 0:
            checksum += 1;
            break;
        case ((__int128)1 << 64):
            checksum += 2;
            break;
        case ((__int128)0x12345678 << 64) | 0x9ABCDEF0:
            checksum += 1048576;  // This should match
            break;
        default:
            checksum += 4;
            break;
    }
    
    return checksum;
}

/* Test 7: Array operations for optimizer workload */
static int test_array_operations(void) {
    int checksum = 0;
    
    /* Array of 8 __int128 elements as requested */
    __int128 arr[8] = {
        0,
        ((__int128)1 << 63),
        ((__int128)1 << 64),
        ((__int128)-1 << 63),
        ((__int128)-1 << 64),
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64),
        ((__int128)0x8000000000000000ULL << 64),
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
    };
    
    /* Perform comparisons that exercise all paths */
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (arr[i] < arr[j]) checksum += 1;
            if (arr[i] > arr[j]) checksum += 2;
            if (arr[i] == arr[j]) checksum += 4;
            
            /* Mixed signed/unsigned comparisons */
            unsigned __int128 ui = (unsigned __int128)arr[i];
            unsigned __int128 uj = (unsigned __int128)arr[j];
            if (ui < uj) checksum += 8;
            if (ui > uj) checksum += 16;
        }
    }
    
    /* Arithmetic that may overflow */
    for (int i = 0; i < 8; i++) {
        __int128 doubled;
        if (!__builtin_mul_overflow(arr[i], 2, &doubled)) {
            if (doubled > arr[i]) checksum += 32;
        }
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests to exercise different comparison paths */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_low_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_range_analysis();
    total_checksum += test_mixed_precision();
    total_checksum += test_builtins_and_switch();
    total_checksum += test_array_operations();
    
    printf("Total checksum: %d\n", total_checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    /* Force use of results to prevent dead code elimination */
    volatile int result = total_checksum;
    
    return result != 0 ? 0 : 1;
}
