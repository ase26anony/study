/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Force the use of 128-bit integers */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== SECTION 1: Constant Folding with Large Integers ========== */

/* Large constants that exceed 64 bits */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)0x7FFFFFFFFFFFFFFF) * 2;
static const uint128_t LARGE_MASK = ~((uint128_t)0);

/* Compile-time comparisons using static assertions */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant should be positive");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant should be negative");
_Static_assert(HUGE_PRODUCT > INT64_MAX, "Product should exceed 64-bit max");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static volatile int result = ((a) op (b)) ? 1 : 0; \
            (void)result; \
        } \
    } while(0)

/* ========== SECTION 2: GCC Builtins with Overflow ========== */

void test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Test cases that should trigger overflow comparisons */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MAX, 2) = %d, overflow = %d\n", (int)res, overflow);
    
    a = INT64_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(INT64_MIN, -1) = %d, overflow = %d\n", (int)res, overflow);
    
    /* Test with __builtin_constant_p */
    if (__builtin_constant_p(__builtin_mul_overflow_p(INT64_MAX, 2, INT64_MAX))) {
        printf("Constant overflow check performed\n");
    }
    
    /* Addition overflow with large values */
    int64_t x = INT64_MAX;
    int64_t y = 1;
    overflow = __builtin_add_overflow(x, y, &res);
    printf("add_overflow(INT64_MAX, 1) = %d, overflow = %d\n", (int)res, overflow);
}

/* ========== SECTION 3: Range Calculations and VRP ========== */

void test_range_calculations(int input) {
    /* Create complex range conditions */
    if (input > 1000 && input < 2000) {
        /* This multiplication's range analysis uses double_int comparisons */
        int64_t square = (int64_t)input * (int64_t)input;
        
        /* Further range refinement */
        if (square > 1500000 && square < 3000000) {
            int64_t cube = square * (int64_t)input;
            printf("Range test: input=%d, square=%lld, cube=%lld\n", 
                   input, (long long)square, (long long)cube);
        }
    }
    
    /* Test with potential wrap-around */
    for (int64_t i = INT64_MAX - 10; i < INT64_MAX + 5LL; i++) {
        /* Loop analysis may use double_int for wrap detection */
        volatile int64_t j = i * 2;
        (void)j;
    }
}

/* ========== SECTION 4: Large Integer Operations ========== */

void test_large_integer_ops(void) {
    uint128_t a = ((uint128_t)1 << 80) | 0x123456789ABCDEF0ULL;
    uint128_t b = ((uint128_t)1 << 79) | 0xFEDCBA9876543210ULL;
    
    /* Various comparisons that should use double_int::cmp */
    int cmp_results[6];
    cmp_results[0] = (a < b) ? -1 : (a > b) ? 1 : 0;
    cmp_results[1] = (a > b) ? 1 : (a < b) ? -1 : 0;
    cmp_results[2] = (a == b) ? 0 : (a < b) ? -1 : 1;
    cmp_results[3] = (b < a) ? -1 : (b > a) ? 1 : 0;
    cmp_results[4] = (a <= b) ? -1 : 1;
    cmp_results[5] = (a >= b) ? 1 : -1;
    
    /* Arithmetic that produces wide results */
    uint128_t sum = a + b;
    uint128_t diff = a - b;
    uint128_t prod = (a >> 40) * (b >> 40); /* Avoid full 128-bit multiplication */
    
    printf("Large int cmp results: %d %d %d %d %d %d\n",
           cmp_results[0], cmp_results[1], cmp_results[2],
           cmp_results[3], cmp_results[4], cmp_results[5]);
    
    /* Shift operations beyond 64 bits */
    uint128_t shifted = a << 65;
    if (shifted > b) {
        printf("Shifted value exceeds comparison target\n");
    }
}

/* ========== SECTION 5: Compile-time Template-like Tests ========== */

/* C doesn't have templates, but we can use macros and static functions */
#define DECLARE_LARGE_COMPARE(name, val1, val2) \
    static inline int name##_compare(void) { \
        const int128_t v1 = (val1); \
        const int128_t v2 = (val2); \
        if (v1 < v2) return -1; \
        if (v1 > v2) return 1; \
        return 0; \
    }

/* Declare several comparison functions with different large values */
DECLARE_LARGE_COMPARE(test1, ((int128_t)1 << 66), ((int128_t)1 << 65))
DECLARE_LARGE_COMPARE(test2, -((int128_t)1 << 66), ((int128_t)1 << 65))
DECLARE_LARGE_COMPARE(test3, ((int128_t)0x7FFFFFFFFFFFFFFF) << 1, INT64_MAX)

/* ========== SECTION 6: Complex Conditional Expressions ========== */

int complex_compare(int128_t a, int128_t b) {
    /* This complex conditional should generate multiple comparison nodes */
    return (a < b) ? -1 : 
           (a > b) ? 1 : 
           ((a & 1) < (b & 1)) ? -1 :
           ((a & 1) > (b & 1)) ? 1 : 0;
}

void test_complex_conditions(void) {
    int128_t values[] = {
        ((int128_t)1 << 70),
        ((int128_t)1 << 69),
        -((int128_t)1 << 70),
        0,
        ((int128_t)0x123456789ABCDEF0) << 32
    };
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int result = complex_compare(values[i], values[j]);
            printf("complex_compare(v[%d], v[%d]) = %d\n", i, j, result);
        }
    }
}

/* ========== SECTION 7: Bit-field and Enumeration Tests ========== */

/* Enumeration with large values (GCC extension) */
enum big_enum {
    BIG_VAL1 = ((__int128)1 << 70),
    BIG_VAL2 = ((__int128)1 << 71),
    BIG_VAL3 = ((__int128)1 << 72)
};

void test_enum_comparisons(void) {
    /* Comparisons between enum values */
    _Static_assert(BIG_VAL2 > BIG_VAL1, "Enum values should compare correctly");
    _Static_assert(BIG_VAL3 > BIG_VAL2, "Enum values should compare correctly");
    
    /* Runtime enum comparisons */
    enum big_enum e1 = BIG_VAL1;
    enum big_enum e2 = BIG_VAL2;
    
    if (e1 < e2) {
        printf("Enum comparison correct: %llu < %llu\n",
               (unsigned long long)((uint128_t)e1 >> 64),
               (unsigned long long)((uint128_t)e2 >> 64));
    }
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Force evaluation of compile-time comparisons */
    COMPILE_TIME_CMP(VERY_LARGE_POS, VERY_LARGE_NEG, >);
    COMPILE_TIME_CMP(HUGE_PRODUCT, INT64_MAX, >);
    COMPILE_TIME_CMP(LARGE_MASK, 0, !=);
    
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    printf("2. Testing range calculations:\n");
    test_range_calculations(1500);
    test_range_calculations(1200);
    printf("\n");
    
    printf("3. Testing large integer operations:\n");
    test_large_integer_ops();
    printf("\n");
    
    printf("4. Testing template-like comparisons:\n");
    printf("test1_compare: %d\n", test1_compare());
    printf("test2_compare: %d\n", test2_compare());
    printf("test3_compare: %d\n", test3_compare());
    printf("\n");
    
    printf("5. Testing complex conditions:\n");
    test_complex_conditions();
    printf("\n");
    
    printf("6. Testing enum comparisons:\n");
    test_enum_comparisons();
    printf("\n");
    
    /* Additional tests with command-line input */
    if (argc > 1) {
        int val = atoi(argv[1]);
        test_range_calculations(val);
    }
    
    /* Final validation */
    printf("=== All tests completed ===\n");
    
    /* Verify some fundamental properties */
    assert(VERY_LARGE_POS > 0);
    assert(VERY_LARGE_NEG < 0);
    assert(((int128_t)1 << 65) > INT64_MAX);
    
    return 0;
}
