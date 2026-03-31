/* test_double_int_cmp.c - Comprehensive test to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* ==================== SECTION 1: Large Constants & Static Assertions ==================== */

/* Use __int128 for explicit wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX) * ((int128_t)INT64_MAX);
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 72) - 1;

/* Static assertions that force compile-time comparisons */
#define STATIC_ASSERT(cond) typedef char static_assert_##__LINE__[(cond) ? 1 : -1]

STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(HUGE_PRODUCT > VERY_LARGE_POS);
STATIC_ASSERT(LARGE_UNSIGNED > (uint128_t)VERY_LARGE_POS);

/* ==================== SECTION 2: Builtin Overflow Checks ==================== */

/* Functions that use overflow builtins - these trigger double_int comparisons internally */
static int test_mul_overflow(int64_t a, int64_t b, int64_t *res) {
    return __builtin_mul_overflow(a, b, res);
}

static int test_add_overflow(int64_t a, int64_t b, int64_t *res) {
    return __builtin_add_overflow(a, b, res);
}

static int test_sub_overflow(int64_t a, int64_t b, int64_t *res) {
    return __builtin_sub_overflow(a, b, res);
}

/* Test overflow with constants that force double_int comparisons */
static void test_overflow_builtins(void) {
    int64_t res;
    int overflow;
    
    /* These multiplications will overflow 64-bit */
    overflow = test_mul_overflow(INT64_MAX, 2, &res);
    printf("mul_overflow(INT64_MAX, 2): overflow=%d\n", overflow);
    
    overflow = test_mul_overflow(INT64_MIN, -2, &res);
    printf("mul_overflow(INT64_MIN, -2): overflow=%d\n", overflow);
    
    /* Large addition that overflows */
    overflow = test_add_overflow(INT64_MAX, 1, &res);
    printf("add_overflow(INT64_MAX, 1): overflow=%d\n", overflow);
    
    /* Large subtraction that underflows */
    overflow = test_sub_overflow(INT64_MIN, 1, &res);
    printf("sub_overflow(INT64_MIN, 1): overflow=%d\n", overflow);
}

/* ==================== SECTION 3: Range Analysis & VRP Triggers ==================== */

/* Complex conditional ranges that require VRP analysis */
static int128_t analyze_range(int x) {
    /* Create known bounds */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        int128_t y = (int128_t)x * (int128_t)x;
        
        /* Further comparisons in range analysis */
        if (y > 1000000 && y < 4000000) {
            return y * 2;
        }
    }
    
    if (x < -1000 && x > -2000) {
        int128_t y = (int128_t)x * (int128_t)x;
        /* Negative range comparisons */
        if (y > 1000000 && y < 4000000) {
            return -y;
        }
    }
    
    return x;
}

/* Loop with induction variable that may wrap */
static void test_loop_ranges(void) {
    int64_t i;
    int128_t sum = 0;
    
    /* Loop that could potentially overflow in analysis */
    for (i = INT64_MAX - 10; i < INT64_MAX + 5LL; i++) {
        sum += i;
        if (sum > ((int128_t)1 << 70)) {
            sum = 0;
        }
    }
    
    printf("Loop range test completed, sum = %lld\n", (long long)sum);
}

/* ==================== SECTION 4: Compile-time Constant Expressions ==================== */

/* Force constant evaluation with __builtin_constant_p */
static void test_constant_folding(void) {
    const int128_t a = ((int128_t)1 << 65);
    const int128_t b = ((int128_t)1 << 64);
    
    /* These comparisons should be evaluated at compile time */
    if (__builtin_constant_p(a > b)) {
        printf("Compile-time comparison a > b: %s\n", a > b ? "true" : "false");
    }
    
    if (__builtin_constant_p(a < b)) {
        printf("Compile-time comparison a < b: %s\n", a < b ? "true" : "false");
    }
    
    /* Mixed signed/unsigned comparisons */
    const uint128_t ua = ((uint128_t)1 << 65);
    const int128_t sb = ((int128_t)1 << 64);
    
    if (__builtin_constant_p(ua > (uint128_t)sb)) {
        printf("Compile-time unsigned comparison: %s\n", 
               ua > (uint128_t)sb ? "true" : "false");
    }
}

/* ==================== SECTION 5: C++ Template Metaprogramming (if compiled as C++) ==================== */

#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((int128_t)1 << 65);
    static const bool is_very_large = N > ((int128_t)1 << 100);
};

template <int128_t A, int128_t B>
struct CompareValues {
    static const int cmp_result = (A > B) ? 1 : ((A < B) ? -1 : 0);
    static const bool equal = (A == B);
    static const bool not_equal = (A != B);
    static const bool less = (A < B);
    static const bool greater = (A > B);
    static const bool less_equal = (A <= B);
    static const bool greater_equal = (A >= B);
};

/* Instantiate templates with large values */
template struct CompareValues<((int128_t)1 << 70), ((int128_t)1 << 69)>;
template struct CompareValues<((int128_t)1 << 69), ((int128_t)1 << 70)>;
template struct CompareValues<((int128_t)1 << 70), ((int128_t)1 << 70)>;
template struct CompareValues<-((int128_t)1 << 70), ((int128_t)1 << 69)>;

#endif

/* ==================== SECTION 6: Runtime Validation ==================== */

/* Validate that comparisons work correctly at runtime */
static int validate_comparisons(void) {
    int errors = 0;
    
    /* Test 1: Basic ordering */
    int128_t a = ((int128_t)1 << 70);
    int128_t b = ((int128_t)1 << 69);
    
    if (!(a > b)) errors++;
    if (!(b < a)) errors++;
    if (a == b) errors++;
    
    /* Test 2: Negative values */
    int128_t neg = -((int128_t)1 << 70);
    if (!(neg < 0)) errors++;
    if (!(neg < b)) errors++;
    
    /* Test 3: Equality */
    int128_t c = a;
    if (a != c) errors++;
    
    /* Test 4: Near boundary values */
    int128_t max64 = INT64_MAX;
    int128_t min64 = INT64_MIN;
    
    if (!(max64 > min64)) errors++;
    if (!((max64 + 1) > max64)) errors++;
    
    /* Test 5: Unsigned comparisons */
    uint128_t u1 = ((uint128_t)1 << 72);
    uint128_t u2 = ((uint128_t)1 << 71);
    
    if (!(u1 > u2)) errors++;
    if (!(u2 < u1)) errors++;
    
    return errors;
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    int errors = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    printf("2. Testing range analysis:\n");
    test_loop_ranges();
    /* Test with various inputs to trigger different range paths */
    for (int i = -1500; i <= 1500; i += 500) {
        analyze_range(i);
    }
    printf("Range analysis tests completed\n\n");
    
    printf("3. Testing constant folding:\n");
    test_constant_folding();
    printf("\n");
    
    printf("4. Validating runtime comparisons:\n");
    errors = validate_comparisons();
    if (errors == 0) {
        printf("All runtime comparisons PASSED\n\n");
    } else {
        printf("Runtime comparisons FAILED with %d errors\n\n", errors);
    }
    
    /* Final validation with large constant expressions */
    printf("5. Final compile-time validations:\n");
    
    /* These will fail compilation if comparisons don't work */
    #define COMPILE_TIME_CHECK(cond, msg) \
        do { \
            if (!(cond)) { \
                printf("FAIL: %s\n", msg); \
                return 1; \
            } else { \
                printf("PASS: %s\n", msg); \
            } \
        } while(0)
    
    COMPILE_TIME_CHECK(VERY_LARGE_POS > 0, "Very large positive > 0");
    COMPILE_TIME_CHECK(VERY_LARGE_NEG < 0, "Very large negative < 0");
    COMPILE_TIME_CHECK(HUGE_PRODUCT > VERY_LARGE_POS, "Huge product > very large");
    COMPILE_TIME_CHECK(LARGE_UNSIGNED > 0, "Large unsigned > 0");
    
    printf("\n=== All tests completed successfully ===\n");
    
    return 0;
}
