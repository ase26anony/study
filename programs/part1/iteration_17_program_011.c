/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_64         0x7FFFFFFFFFFFFFFFULL

/* Large 128-bit constants for constant folding */
static const __int128 LARGE_POS = ((__int128)HIGH_BIT_64 << 64) | MID_64;
static const __int128 LARGE_NEG = ((__int128)HIGH_BIT_64 << 64) | 0ULL;
static const __int128 MAX_POS_128 = ((__int128)MID_64 << 64) | MAX_64;
static const __int128 MIN_NEG_128 = ((__int128)HIGH_BIT_64 << 64) | 0ULL;

static const unsigned __int128 UMAX_128 = ((unsigned __int128)MAX_64 << 64) | MAX_64;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)0x1ULL << 64) > 0, "High word positive comparison");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "High word negative comparison");
_Static_assert(((__int128)0x1ULL << 64) < ((__int128)0x2ULL << 64), 
               "High word differs comparison");

/* Test 1: Comparisons where high words differ */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* High word positive, different values */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should trigger high word comparison */
    checksum += (b1 > a1) ? 2 : 0;
    
    /* High word negative comparisons */
    __int128 a2 = ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;  /* Very negative */
    __int128 b2 = ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x0ULL;  /* Less negative */
    checksum += (a2 < b2) ? 4 : 0;  /* Negative high word comparison */
    checksum += (b2 > a2) ? 8 : 0;
    
    /* Mixed positive/negative high words */
    __int128 pos_high = ((__int128)0x1ULL << 64);
    __int128 neg_high = ((__int128)HIGH_BIT_64 << 64);
    checksum += (pos_high > neg_high) ? 16 : 0;
    checksum += (neg_high < pos_high) ? 32 : 0;
    
    return checksum;
}

/* Test 2: Comparisons where high words equal, low words differ */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    /* Same high word, different low words */
    __int128 a1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    __int128 b1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL;
    checksum += (a1 < b1) ? 64 : 0;  /* Should trigger low word comparison */
    checksum += (b1 > a1) ? 128 : 0;
    
    /* Edge case: low word overflow affects high word? */
    __int128 a2 = ((__int128)0x0ULL << 64) | MAX_64;
    __int128 b2 = ((__int128)0x1ULL << 64) | 0x0ULL;
    checksum += (a2 < b2) ? 256 : 0;
    checksum += (b2 > a2) ? 512 : 0;
    
    /* Negative numbers with same high word */
    __int128 a3 = ((__int128)HIGH_BIT_64 << 64) | 0x1111111111111111ULL;
    __int128 b3 = ((__int128)HIGH_BIT_64 << 64) | 0x2222222222222222ULL;
    checksum += (a3 < b3) ? 1024 : 0;  /* Both negative, compare low words */
    checksum += (b3 > a3) ? 2048 : 0;
    
    return checksum;
}

/* Test 3: Range analysis with loops and __int128 induction */
static int test_range_analysis(void) {
    int checksum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFF00ULL;
         i < ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
         i++) {
        checksum += (i & 0xFF);
    }
    
    /* Another loop crossing 64-bit boundary */
    for (__int128 i = ((__int128)0x0ULL << 64) | MAX_64 - 100;
         i < ((__int128)0x1ULL << 64) | 100ULL;
         i++) {
        checksum += (i & 0xFF);
    }
    
    /* Value range propagation with conditions */
    __int128 x = ((__int128)0x1234ULL << 64) | 0x5678ULL;
    if (x > ((__int128)0x1000ULL << 64)) {
        checksum += 4096;
    }
    if (x < ((__int128)0x2000ULL << 64)) {
        checksum += 8192;
    }
    
    return checksum & 0xFFF;  /* Limit size */
}

/* Test 4: Overflow operations requiring wide comparisons */
static int test_overflow_operations(void) {
    int checksum = 0;
    
    /* Use builtin overflow functions */
    __int128 of_a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 of_b = 2;
    __int128 of_result;
    int overflow = __builtin_add_overflow(of_a, of_b, &of_result);
    checksum += overflow ? 1 : 0;
    
    /* Multiplication overflow check */
    __int128 mul_a = ((__int128)0x1ULL << 62);
    __int128 mul_b = 0x10ULL;
    overflow = __builtin_mul_overflow(mul_a, mul_b, &of_result);
    checksum += overflow ? 2 : 0;
    
    /* Subtraction with potential underflow */
    __int128 sub_a = ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;  /* Most negative */
    __int128 sub_b = 1;
    overflow = __builtin_sub_overflow(sub_a, sub_b, &of_result);
    checksum += overflow ? 4 : 0;
    
    return checksum;
}

/* Test 5: Mixed type comparisons and conversions */
static int test_mixed_type_comparisons(void) {
    int checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 wide_val = ((__int128)0x1ULL << 64) | 0x12345678ULL;
    unsigned long long narrow_val = 0x123456789ABCDEF0ULL;
    
    checksum += (wide_val > narrow_val) ? 1 : 0;
    checksum += (wide_val < (__int128)narrow_val) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    long long test_val = 100;
    __int128 ternary_result = (test_val > 50) ? 
        ((__int128)0x1ULL << 64) : 
        (__int128)test_val;
    checksum += (ternary_result > 0) ? 4 : 0;
    
    /* Compare with size_t */
    size_t size_val = (size_t)-1;  /* Maximum size_t */
    __int128 wide_for_size = (__int128)size_val;
    checksum += (wide_for_size == (__int128)size_val) ? 8 : 0;
    checksum += (wide_for_size < wide_val) ? 16 : 0;
    
    /* Array indexing with potential 128-bit calculations */
    __int128 array[8] = {
        ((__int128)0x0ULL << 64) | 0x1ULL,
        ((__int128)0x0ULL << 64) | 0x2ULL,
        ((__int128)0x1ULL << 64) | 0x1ULL,
        ((__int128)0x1ULL << 64) | 0x2ULL,
        ((__int128)HIGH_BIT_64 << 64) | 0x1ULL,
        ((__int128)HIGH_BIT_64 << 64) | 0x2ULL,
        ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x1ULL,
        ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x2ULL
    };
    
    for (int i = 0; i < 7; i++) {
        checksum += (array[i] < array[i + 1]) ? (1 << i) : 0;
    }
    
    return checksum & 0xFF;
}

/* Test 6: Bitwise operations crossing 64-bit boundaries */
static int test_bitwise_operations(void) {
    int checksum = 0;
    
    /* Shift operations that affect high word */
    __int128 shift_val = 0x1ULL;
    __int128 shifted = shift_val << 65;  /* Crosses 64-bit boundary */
    checksum += (shifted > shift_val) ? 1 : 0;
    
    /* Right shift of negative number */
    __int128 neg_val = ((__int128)HIGH_BIT_64 << 64) | MAX_64;
    __int128 shifted_neg = neg_val >> 65;
    checksum += (shifted_neg < 0) ? 2 : 0;
    
    /* Bitwise AND/OR across boundary */
    __int128 mask = ((__int128)0xFFFFULL << 64) | 0xFFFFULL;
    __int128 val = ((__int128)0x12345678ULL << 64) | 0x9ABCDEF0ULL;
    __int128 masked = val & mask;
    checksum += (masked < val) ? 4 : 0;
    
    /* Builtin count functions on 128-bit values */
    unsigned __int128 uval = ((unsigned __int128)0x5555ULL << 64) | 0x5555ULL;
    int popcount = __builtin_popcountll((unsigned long long)(uval >> 64)) +
                   __builtin_popcountll((unsigned long long)uval);
    checksum += (popcount > 0) ? 8 : 0;
    
    return checksum;
}

/* Test 7: Switch statement with __int128 cases (compile-time evaluation) */
static int test_switch_statement(__int128 value) {
    int result = 0;
    
    /* Switch forces compiler to generate comparison trees */
    switch ((unsigned __int128)value) {
        case ((unsigned __int128)0x0ULL << 64) | 0x0ULL:
            result = 1;
            break;
        case ((unsigned __int128)0x0ULL << 64) | 0x1ULL:
            result = 2;
            break;
        case ((unsigned __int128)0x1ULL << 64) | 0x0ULL:
            result = 3;  /* High word differs */
            break;
        case ((unsigned __int128)0x1ULL << 64) | 0x1ULL:
            result = 4;
            break;
        case ((unsigned __int128)HIGH_BIT_64 << 64) | 0x0ULL:
            result = 5;  /* Very large value */
            break;
        default:
            result = 6;
            break;
    }
    
    return result;
}

/* Test 8: __builtin_expect with 128-bit comparisons */
static int test_builtin_expect(void) {
    int checksum = 0;
    
    __int128 likely_val = ((__int128)0x1ULL << 64) | 0x1234ULL;
    __int128 unlikely_val = ((__int128)HIGH_BIT_64 << 64) | 0x5678ULL;
    
    if (__builtin_expect(likely_val > 0, 1)) {
        checksum += 1;
    }
    
    if (__builtin_expect(unlikely_val < 0, 0)) {
        checksum += 2;
    }
    
    /* Compare two 128-bit values with branch prediction */
    __int128 a = ((__int128)0x1ULL << 64) | 0x1111ULL;
    __int128 b = ((__int128)0x2ULL << 64) | 0x2222ULL;
    
    if (__builtin_expect(a < b, 1)) {
        checksum += 4;
    }
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all tests to exercise different comparison paths */
    total_checksum += test_high_word_comparisons();
    printf("Test 1 (high word) checksum: %d\n", test_high_word_comparisons());
    
    total_checksum += test_low_word_comparisons();
    printf("Test 2 (low word) checksum: %d\n", test_low_word_comparisons());
    
    total_checksum += test_range_analysis();
    printf("Test 3 (range analysis) checksum: %d\n", test_range_analysis());
    
    total_checksum += test_overflow_operations();
    printf("Test 4 (overflow) checksum: %d\n", test_overflow_operations());
    
    total_checksum += test_mixed_type_comparisons();
    printf("Test 5 (mixed types) checksum: %d\n", test_mixed_type_comparisons());
    
    total_checksum += test_bitwise_operations();
    printf("Test 6 (bitwise) checksum: %d\n", test_bitwise_operations());
    
    /* Test switch with different values */
    total_checksum += test_switch_statement(((unsigned __int128)0x1ULL << 64) | 0x0ULL);
    total_checksum += test_switch_statement(((unsigned __int128)0x0ULL << 64) | 0x1ULL);
    total_checksum += test_switch_statement(((unsigned __int128)HIGH_BIT_64 << 64) | 0x0ULL);
    printf("Test 7 (switch) partial checksum: %d\n", 
           test_switch_statement(((unsigned __int128)0x1ULL << 64) | 0x0ULL));
    
    total_checksum += test_builtin_expect();
    printf("Test 8 (builtin_expect) checksum: %d\n", test_builtin_expect());
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Force use of constants to prevent dead code elimination */
    volatile __int128 force_use = LARGE_POS + LARGE_NEG;
    (void)force_use;
    
    return total_checksum == 0 ? 1 : 0;
}
