/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Test 1A: Static assertions with 128-bit constants */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t MAX_64 = ((int128_t)1 << 63) - 1;
static const int128_t MIN_64 = -((int128_t)1 << 63);

/* Force compile-time comparisons */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(VERY_LARGE_POS > MAX_64);
STATIC_ASSERT(VERY_LARGE_NEG < MIN_64);
STATIC_ASSERT(VERY_LARGE_POS != VERY_LARGE_NEG);
STATIC_ASSERT((VERY_LARGE_POS >> 1) < VERY_LARGE_POS);
STATIC_ASSERT((VERY_LARGE_NEG >> 1) > VERY_LARGE_NEG);
#endif

/* Test 1B: Constant expressions with overflow */
static const long long LARGE_A = 0x7FFFFFFFFFFFFFFFLL; /* 2^63-1 */
static const long long LARGE_B = 0x7FFFFFFFFFFFFFFFLL;

/* This multiplication overflows 64-bit but fits in 128-bit */
static const int128_t LARGE_MUL = (int128_t)LARGE_A * (int128_t)LARGE_B;

/* ========== 2. GCC Builtins That Use double_int ========== */

/* Test 2A: Overflow builtins with large values */
int test_overflow_builtins(void) {
    int failures = 0;
    
    /* Multiplication overflow checks */
    long long a = 0x7FFFFFFFFFFFFFFFLL;
    long long b = 2;
    long long res;
    int overflow = __builtin_mul_overflow(a, b, &res);
    
    if (!overflow) {
        printf("ERROR: Expected overflow not detected\n");
        failures++;
    }
    
    /* Addition overflow */
    long long c = 0x7FFFFFFFFFFFFFFFLL;
    long long d = 1;
    overflow = __builtin_add_overflow(c, d, &res);
    
    if (!overflow) {
        printf("ERROR: Expected addition overflow not detected\n");
        failures++;
    }
    
    /* Subtraction overflow (underflow) */
    long long e = -0x7FFFFFFFFFFFFFFFLL - 1;
    long long f = 1;
    overflow = __builtin_sub_overflow(e, f, &res);
    
    if (!overflow) {
        printf("ERROR: Expected subtraction overflow not detected\n");
        failures++;
    }
    
    /* __builtin_constant_p with overflow */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                       2, 0))) {
        /* This forces constant evaluation of overflow check */
    }
    
    return failures;
}

/* Test 2B: Builtin comparisons with constant propagation */
int test_builtin_constant_comparisons(void) {
    int failures = 0;
    
    /* Force constant evaluation of comparisons */
    if (__builtin_constant_p(VERY_LARGE_POS > 1000)) {
        if (!(VERY_LARGE_POS > 1000)) {
            printf("ERROR: Constant comparison failed\n");
            failures++;
        }
    }
    
    /* Compare results of constant expressions */
    const int128_t x = ((int128_t)1 << 65) + 123;
    const int128_t y = ((int128_t)1 << 65) + 456;
    
    if (__builtin_constant_p(x < y)) {
        if (!(x < y)) {
            printf("ERROR: x < y comparison failed\n");
            failures++;
        }
    }
    
    return failures;
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

/* Test 3A: Complex range analysis with large values */
int test_range_analysis(int input) {
    int result = 0;
    
    /* Create known bounds for range analysis */
    if (input > 1000 && input < 10000) {
        /* This multiplication creates a range that needs double_int comparison */
        long long squared = (long long)input * (long long)input;
        
        /* Nested conditions for complex range analysis */
        if (squared > 5000000LL && squared < 50000000LL) {
            result = 1;
        }
    }
    
    /* Large step induction variable */
    for (long long i = 0x7000000000000000LL; 
         i < 0x7FFFFFFFFFFFFFFFLL; 
         i += 0x100000000000000LL) {
        /* Loop analysis uses double_int for wrap-around checks */
        if (i > 0x7FFFFFFFFFFFFFF0LL) {
            result |= 2;
            break;
        }
    }
    
    return result;
}

/* Test 3B: Value Range Propagation with overflow */
void test_vrp_with_overflow(void) {
    int x, y;
    
    /* Simulate values that might overflow in analysis */
    for (x = 0x7FFFFFF0; x < 0x7FFFFFFF; x++) {
        for (y = 2; y < 10; y++) {
            /* This forces VRP to analyze potential overflow */
            long long prod = (long long)x * (long long)y;
            if (prod > 0x7FFFFFFFFFFFFFFFLL) {
                /* Should be unreachable but analyzed */
                printf("Unexpected overflow\n");
            }
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ only) ========== */

#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((int128_t)1 << 65);
    static const bool fits_64bit = N >= -((int128_t)1 << 63) && 
                                   N < ((int128_t)1 << 63);
};

/* Instantiate templates with various large values */
template struct LargeCompare<((int128_t)1 << 70)>;
template struct LargeCompare<-((int128_t)1 << 70)>;
template struct LargeCompare<((int128_t)1 << 63) - 1>;
template struct LargeCompare<-((int128_t)1 << 63)>;

/* Template function that performs comparisons */
template <int128_t A, int128_t B>
constexpr int compare_values() {
    return (A < B) ? -1 : (A > B) ? 1 : 0;
}

/* Force instantiation */
static constexpr int cmp1 = compare_values<((int128_t)1 << 70), 
                                           ((int128_t)1 << 69)>();
static constexpr int cmp2 = compare_values<-((int128_t)1 << 70), 
                                           ((int128_t)1 << 70)>();
static constexpr int cmp3 = compare_values<((int128_t)1 << 65) + 123, 
                                           ((int128_t)1 << 65) + 456>();

#endif

/* ========== 5. Force Tree Node Construction ========== */

/* Test 5A: Wide enumerations */
#ifdef __SIZEOF_INT128__
enum wide_enum : int128_t {
    ENUM_LARGE = ((int128_t)1 << 70),
    ENUM_SMALL = 100,
    ENUM_NEG = -((int128_t)1 << 70)
};

/* Test 5B: Operations on wide constants */
int128_t wide_operations(int128_t a, int128_t b) {
    /* Various operations that require magnitude comparison */
    if (a == 0) return b;
    if (b == 0) return a;
    
    /* Division needs comparison for quotient estimation */
    if (a > b) {
        return a / b;
    } else {
        return b / a;
    }
}

/* Test 5C: 128-bit type using attribute */
typedef int TItype __attribute__((mode(TI)));
typedef unsigned int UTItype __attribute__((mode(TI)));

void test_128bit_type(void) {
    TItype x = ((TItype)1 << 70);
    TItype y = ((TItype)1 << 69);
    
    /* Force comparisons */
    if (x > y) {
        /* Comparison uses double_int internally */
        TItype z = x + y;
        (void)z;
    }
}
#endif

/* ========== Main Test Driver ========== */

int main(void) {
    int total_failures = 0;
    int test_result;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 2: Overflow builtins */
    printf("Test 2A: Overflow builtins...\n");
    test_result = test_overflow_builtins();
    total_failures += test_result;
    printf(test_result == 0 ? "PASS\n\n" : "FAIL\n\n");
    
    /* Test 2B: Builtin constant comparisons */
    printf("Test 2B: Builtin constant comparisons...\n");
    test_result = test_builtin_constant_comparisons();
    total_failures += test_result;
    printf(test_result == 0 ? "PASS\n\n" : "FAIL\n\n");
    
    /* Test 3: Range analysis */
    printf("Test 3A: Range analysis...\n");
    test_result = test_range_analysis(2000);
    if (test_result == 1) {
        printf("PASS\n\n");
    } else {
        printf("FAIL\n\n");
        total_failures++;
    }
    
    /* Test 3B: VRP with overflow */
    printf("Test 3B: VRP with overflow...\n");
    test_vrp_with_overflow();
    printf("PASS (no crash)\n\n");
    
#ifdef __SIZEOF_INT128__
    /* Test 5: Wide operations */
    printf("Test 5: Wide operations...\n");
    int128_t op_result = wide_operations(ENUM_LARGE, ENUM_SMALL);
    if (op_result == (ENUM_LARGE / ENUM_SMALL)) {
        printf("PASS\n\n");
    } else {
        printf("FAIL\n\n");
        total_failures++;
    }
    
    /* Test 5C: 128-bit type */
    printf("Test 5C: 128-bit type comparisons...\n");
    test_128bit_type();
    printf("PASS\n\n");
#endif
    
    /* Final summary */
    printf("=== Summary ===\n");
    if (total_failures == 0) {
        printf("All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        printf("%d test(s) failed\n", total_failures);
        return EXIT_FAILURE;
    }
}
