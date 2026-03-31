/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 in double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT 0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "Shifted max should be larger");

/* Test 1: High word comparisons (signed) */
static int test_high_word_comparisons(void) {
    volatile __int128 results = 0;
    
    /* Case 1: High words differ, both positive */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    results += (a1 < b1) ? 1 : 0;
    results += (a1 > b1) ? 2 : 0;
    
    /* Case 2: High words differ, negative values */
    __int128 a2 = -((__int128)0x2ULL << 64);
    __int128 b2 = -((__int128)0x1ULL << 64);
    results += (a2 < b2) ? 4 : 0;
    results += (a2 > b2) ? 8 : 0;
    
    /* Case 3: High words equal, low words differ */
    __int128 a3 = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 b3 = ((__int128)0x1ULL << 64) | 0x2ULL;
    results += (a3 < b3) ? 16 : 0;
    results += (a3 > b3) ? 32 : 0;
    
    return (int)results;
}

/* Test 2: Boundary value comparisons */
static int test_boundary_values(void) {
    volatile unsigned __int128 results = 0;
    
    /* Signed boundaries */
    __int128 int128_max = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 int128_min = ((__int128)0x8000000000000000ULL << 64);
    
    results += (int128_min < int128_max) ? 1 : 0;
    results += (int128_min > 0) ? 2 : 0;
    results += (int128_max > 0) ? 4 : 0;
    
    /* Unsigned boundaries */
    unsigned __int128 uint128_max = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 uint128_mid = ((unsigned __int128)0x1ULL << 64);
    
    results += (uint128_mid < uint128_max) ? 8 : 0;
    results += (uint128_max > uint128_mid) ? 16 : 0;
    
    /* Cross signed/unsigned boundary */
    results += ((unsigned __int128)int128_max < uint128_max) ? 32 : 0;
    
    return (int)results;
}

/* Test 3: Range analysis triggers with loops */
static int test_range_analysis(void) {
    volatile __int128 checksum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundary */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFFF0ULL); 
         i < ((__int128)0x7FFFFFFFFFFFFFFFULL); 
         i++) {
        checksum += i;
    }
    
    /* Overflow checking with builtins */
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 y = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 overflow_result;
    
    if (__builtin_add_overflow(x, y, &overflow_result)) {
        checksum += 1;
    }
    
    if (__builtin_mul_overflow(x, 2, &overflow_result)) {
        checksum += 2;
    }
    
    return (int)(checksum & 0xFFFFFFFF);
}

/* Test 4: Mixed-precision operations */
static int test_mixed_precision(void) {
    volatile int result = 0;
    
    /* Compare __int128 with narrower types */
    __int128 wide_val = ((__int128)0x1ULL << 64) | 0x12345678ULL;
    long long narrow_val = 0x123456789ABCDEF0LL;
    unsigned long long u_narrow_val = 0xFEDCBA9876543210ULL;
    size_t size_val = (size_t)-1;
    
    result += (wide_val > narrow_val) ? 1 : 0;
    result += (wide_val < u_narrow_val) ? 2 : 0;
    result += (wide_val == size_val) ? 4 : 0;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (narrow_val > 0) ? 
        ((__int128)0x1ULL << 64) : (__int128)narrow_val;
    result += (ternary_result > 0) ? 8 : 0;
    
    /* Bitwise operations crossing 64-bit boundary */
    __int128 bitwise = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    bitwise = bitwise & ((__int128)MAX_64 << 32);
    bitwise = bitwise | ((__int128)0x5555555555555555ULL);
    bitwise = bitwise >> 65;  /* Shift across boundary */
    bitwise = bitwise << 33;
    
    result += (bitwise > 0) ? 16 : 0;
    
    return result;
}

/* Test 5: Compiler built-in functions */
static int test_builtin_functions(void) {
    volatile int result = 0;
    
    unsigned __int128 val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                            0xFEDCBA9876543210ULL;
    
    /* Test bitscan operations on high and low parts */
    unsigned long long low_part = (unsigned long long)val;
    unsigned long long high_part = (unsigned long long)(val >> 64);
    
    result += __builtin_clzll(high_part);
    result += __builtin_ctzll(low_part);
    result += __builtin_popcountll(high_part ^ low_part);
    
    /* __builtin_expect with __int128 comparison */
    __int128 a = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 b = ((__int128)0x1ULL << 64) | 0x2ULL;
    
    if (__builtin_expect(a < b, 1)) {
        result += 1;
    }
    
    if (__builtin_expect(a > b, 0)) {
        result += 2;
    }
    
    return result & 0xFF;
}

/* Test 6: Array operations for optimizer */
static int test_array_operations(void) {
    /* Array of __int128 values that exercise different comparison paths */
    __int128 arr[8] = {
        ((__int128)0x0ULL << 64) | 0x1ULL,                    /* Small positive */
        ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,     /* Max 64-bit */
        ((__int128)0x1ULL << 64) | 0x0ULL,                    /* Just crossed 64-bit */
        ((__int128)0x1ULL << 64) | 0x1ULL,                    /* Crossed + small */
        -((__int128)0x1ULL << 64),                            /* Negative crossed */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x0ULL,     /* Near INT128_MAX */
        ((__int128)0x8000000000000000ULL << 64),              /* INT128_MIN */
        ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL /* -1 */
    };
    
    volatile int checksum = 0;
    
    /* Perform comparisons that will exercise all high/low word paths */
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (arr[i] < arr[j]) checksum += 1;
            if (arr[i] > arr[j]) checksum += 2;
            if (arr[i] == arr[j]) checksum += 4;
            
            /* Also compare with unsigned interpretation */
            unsigned __int128 ui = (unsigned __int128)arr[i];
            unsigned __int128 uj = (unsigned __int128)arr[j];
            if (ui < uj) checksum += 8;
            if (ui > uj) checksum += 16;
        }
    }
    
    /* Additional: switch with __int128 constants (compile-time evaluation) */
    __int128 switch_val = arr[2];
    switch (switch_val) {
        case ((__int128)0x1ULL << 64):
            checksum += 100;
            break;
        case ((__int128)0x1ULL << 64) | 0x1ULL:
            checksum += 200;
            break;
        default:
            checksum += 300;
    }
    
    return checksum;
}

/* Test 7: Arithmetic with overflow checking */
static int test_overflow_arithmetic(void) {
    volatile int result = 0;
    
    /* Large multiplications that require wide comparisons */
    __int128 large1 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 large2 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    
    __int128 product = large1 * large2;
    __int128 sum = large1 + large2;
    __int128 diff = large1 - (-large2);
    
    result += (product > 0) ? 1 : 0;
    result += (sum > large1) ? 2 : 0;
    result += (diff > large1) ? 4 : 0;
    
    /* Overflow detection */
    __int128 of_result;
    if (__builtin_add_overflow(large1, large2, &of_result)) {
        result += 8;
    }
    
    /* Division and modulo with wide integers */
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64);
    __int128 divisor = ((__int128)0x1000000000000000ULL);
    __int128 quotient = dividend / divisor;
    __int128 remainder = dividend % divisor;
    
    result += (quotient > divisor) ? 16 : 0;
    result += (remainder < divisor) ? 32 : 0;
    
    return result;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests to exercise different comparison paths */
    total_checksum += test_high_word_comparisons();
    printf("Test 1 complete: %d\n", total_checksum);
    
    total_checksum += test_boundary_values();
    printf("Test 2 complete: %d\n", total_checksum);
    
    total_checksum += test_range_analysis();
    printf("Test 3 complete: %d\n", total_checksum);
    
    total_checksum += test_mixed_precision();
    printf("Test 4 complete: %d\n", total_checksum);
    
    total_checksum += test_builtin_functions();
    printf("Test 5 complete: %d\n", total_checksum);
    
    total_checksum += test_array_operations();
    printf("Test 6 complete: %d\n", total_checksum);
    
    total_checksum += test_overflow_arithmetic();
    printf("Test 7 complete: %d\n", total_checksum);
    
    printf("Final checksum: %d\n", total_checksum);
    
    /* Force use of variadic function with __int128 */
    __int128 var1 = ((__int128)0x123456789ABCDEF0ULL << 64);
    printf("Variadic test: high=%llx low=%llx\n", 
           (unsigned long long)(var1 >> 64),
           (unsigned long long)var1);
    
    return total_checksum == 0 ? 1 : 0;
}
