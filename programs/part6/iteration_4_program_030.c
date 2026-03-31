/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force compile-time evaluation with large integers */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* Test 1: Static assertions with very large 128-bit constants */
STATIC_ASSERT(((unsigned __int128)1 << 70) > 0);
STATIC_ASSERT(((unsigned __int128)1 << 70) > ((unsigned __int128)1 << 69));
STATIC_ASSERT(((unsigned __int128)1 << 100) > ((unsigned __int128)1 << 90));
STATIC_ASSERT(((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 0xFFFFFFFFFFFFFFFFULL);

/* Test 2: Template-like compile-time comparisons (C++ style in C) */
#define COMPARE_128(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

/* Force constant folding with __builtin_constant_p */
static int test_constant_folding(void) {
    const unsigned __int128 huge1 = ((unsigned __int128)1 << 72) | 0x123456789ABCDEFULL;
    const unsigned __int128 huge2 = ((unsigned __int128)1 << 71) | 0xFEDCBA987654321ULL;
    
    if (__builtin_constant_p(huge1 > huge2)) {
        return huge1 > huge2 ? 1 : 0;
    }
    return -1;
}

/* Test 3: Builtin overflow operations that use double_int internally */
static void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test with values that may overflow 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64 */
    b = 2;
    
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test: %lld * 2 overflowed? %s\n", 
           a, overflow ? "YES" : "NO");
    
    /* Test with large constant that forces double_int comparison */
    const __int128 c = ((__int128)1 << 63);
    const __int128 d = ((__int128)1 << 62);
    
    if (__builtin_constant_p(c > d)) {
        printf("Constant 128-bit comparison: %s\n", 
               c > d ? "c > d" : "c <= d");
    }
}

/* Test 4: Range analysis with large bounds */
static void test_range_analysis(void) {
    int x;
    
    /* Simulate range analysis with large values */
    for (x = 1000000000; x < 2000000000; x += 100000000) {
        /* Multiplication that may require 128-bit intermediate */
        long long y = (long long)x * x;
        
        /* Comparison that might be analyzed during VRP */
        if (y > (long long)1e18) {
            printf("Range test: x=%d, y=%lld exceeds 1e18\n", x, y);
        }
    }
    
    /* Test with explicit bounds checking */
    unsigned long long low = 0xFFFFFFFFFFFFFFF0ULL;
    unsigned long long high = 0xFFFFFFFFFFFFFFFFULL;
    
    /* This comparison may use double_int in VRP */
    if (high - low < 100) {
        printf("Range is small: %llu\n", high - low);
    }
}

/* Test 5: Complex arithmetic with 128-bit intermediates */
static void test_large_arithmetic(void) {
    /* Operations that require 128-bit precision */
    const unsigned __int128 mask = ~((unsigned __int128)0);
    const unsigned __int128 half_mask = mask >> 1;
    
    printf("Mask comparison: %s\n", 
           mask > half_mask ? "full mask > half mask" : "unexpected");
    
    /* Test lexicographic comparison scenarios */
    unsigned __int128 val1 = ((unsigned __int128)0x1ULL << 64) | 0x0ULL;
    unsigned __int128 val2 = ((unsigned __int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    printf("Lexicographic test: val1 %s val2\n",
           val1 > val2 ? ">" : (val1 < val2 ? "<" : "=="));
    
    /* Test equal high parts, different low parts */
    unsigned __int128 val3 = ((unsigned __int128)0x1ULL << 64) | 0x1ULL;
    unsigned __int128 val4 = ((unsigned __int128)0x1ULL << 64) | 0x2ULL;
    
    printf("Equal high parts test: val3 %s val4\n",
           val3 > val4 ? ">" : (val3 < val4 ? "<" : "=="));
}

/* Test 6: Enumeration with large values (forces INTEGER_CST creation) */
enum big_values {
    BIG_A = ((__int128)1 << 66),
    BIG_B = ((__int128)1 << 67),
    BIG_C = BIG_B - 1
};

/* Test 7: Bitfield with large width (non-portable but may trigger paths) */
struct large_bitfield {
    unsigned __int128 field1 : 70;
    unsigned __int128 field2 : 70;
};

/* Test 8: __int128 operations with comparison */
static void test_int128_comparisons(void) {
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFLL << 32);
    __int128 b = ((__int128)0x7FFFFFFFFFFFFFFFLL << 31);
    
    /* Multiple comparison types to hit all branches */
    int results[6];
    results[0] = a < b;
    results[1] = a > b;
    results[2] = a <= b;
    results[3] = a >= b;
    results[4] = a == b;
    results[5] = a != b;
    
    printf("128-bit comparisons: %d %d %d %d %d %d\n",
           results[0], results[1], results[2],
           results[3], results[4], results[5]);
    
    /* Test with negative values */
    __int128 neg = -((__int128)1 << 70);
    __int128 pos = ((__int128)1 << 69);
    
    printf("Signed comparison: %s\n", neg < pos ? "negative < positive" : "error");
}

/* Test 9: Compile-time switch with large constants */
static const char* classify_large_value(unsigned __int128 val) {
    /* GCC may generate double_int comparisons for each case */
    switch (val) {
        case ((unsigned __int128)1 << 65):
            return "2^65";
        case ((unsigned __int128)1 << 66):
            return "2^66";
        case ((unsigned __int128)1 << 67):
            return "2^67";
        default:
            return "other";
    }
}

/* Test 10: Array indexing with large offsets */
static void test_large_offset(void) {
    char buffer[1000];
    unsigned __int128 offset = ((unsigned __int128)1 << 63);
    
    /* Comparison in bounds checking */
    if (offset < sizeof(buffer)) {
        buffer[offset] = 'X';  /* Won't execute, but comparison happens */
    }
    
    printf("Offset comparison completed\n");
}

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Already executed via static assertions at compile time */
    printf("Test 1: Static assertions passed at compile time\n");
    
    /* Test 2: Constant folding */
    printf("Test 2: Constant folding result: %d\n", test_constant_folding());
    
    /* Test 3: Overflow builtins */
    test_overflow_builtins();
    
    /* Test 4: Range analysis */
    test_range_analysis();
    
    /* Test 5: Large arithmetic */
    test_large_arithmetic();
    
    /* Test 7: __int128 comparisons */
    test_int128_comparisons();
    
    /* Test 8: Large constant classification */
    printf("Test 8: Classification of 2^66: %s\n", 
           classify_large_value((unsigned __int128)1 << 66));
    
    /* Test 9: Large offset */
    test_large_offset();
    
    /* Additional test: Mixed signed/unsigned comparisons */
    unsigned __int128 ubig = ((unsigned __int128)1 << 127);
    __int128 sbig = ((__int128)1 << 126);
    
    /* This should trigger unsigned comparison in double_int::cmp */
    printf("Mixed signedness test: ubig %s sbig\n",
           ubig > (unsigned __int128)sbig ? ">" : "<=");
    
    /* Final validation */
    printf("\n=== All tests completed ===\n");
    
    /* Runtime assertion to ensure comparisons worked */
    assert(((unsigned __int128)1 << 70) > ((unsigned __int128)1 << 69));
    assert(((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) > 0xFFFFFFFFFFFFFFFFULL);
    
    return 0;
}
