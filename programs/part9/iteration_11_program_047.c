/* expr-coverage.c
 * Test cases for constant-bounds analysis in GCC's expr.cc
 * Specifically targeting lines 7691-7700
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating test functions */
#define NOINLINE __attribute__((noinline))

/* Volatile variables to prevent constant propagation */
static volatile int g_volatile_zero = 0;
static volatile int g_volatile_one = 1;

/* Vector types for non-memory reference tests */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ==================== SCENARIO 1: Small Count (count <= 2) ==================== */

/* Single element access - count = 1 */
NOINLINE static void test_small_count_1(int *arr, int *out) {
    /* arr[5] is constant-bounded single element */
    *out = arr[5];
}

/* Two adjacent elements - count = 2 */
NOINLINE static void test_small_count_2(int *arr, int *out) {
    /* Access two adjacent elements with constant bounds */
    int sum = arr[2] + arr[3];  /* lo=2, hi=3, count=2 */
    *out = sum;
}

/* Vector type with 2 elements */
NOINLINE static void test_small_count_vector(v2si *vec, int *out) {
    /* Access v[0] and v[1] - count = 2 */
    v2si v = *vec;
    int sum = v[0] + v[1];
    *out = sum;
}

/* ==================== SCENARIO 2: Larger Constant-Sized Access ==================== */

/* Array slice with constant bounds, total size fits in unsigned HWI */
NOINLINE static void test_larger_constant_array(int *arr, int *out) {
    int sum = 0;
    /* Constant bounds: lo=2, hi=9, count=8 */
    /* TYPE_SIZE(int) = 32 bits, total = 32 * 8 = 256 bits fits in uhwi */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Different element type (char) with larger count */
NOINLINE static void test_larger_constant_char(char *arr, int *out) {
    int sum = 0;
    /* Constant bounds: lo=0, hi=31, count=32 */
    /* TYPE_SIZE(char) = 8 bits, total = 8 * 32 = 256 bits fits in uhwi */
    for (int i = 0; i < 32; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Mixed bounds using volatile to prevent elimination */
NOINLINE static void test_volatile_bounds(int *arr, int *out) {
    int sum = 0;
    /* Use volatile in condition to preserve both bounds during analysis */
    int limit = g_volatile_one ? 20 : 30;
    for (int i = 5; i < limit; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* ==================== SCENARIO 3: Non-Memory Reference Cases ==================== */

/* Vector shuffle with constant indices - not a memory reference */
NOINLINE static void test_vector_shuffle(v4si *vec, v4si *out) {
    v4si v = *vec;
    /* Create constant-bounded vector section via shuffle */
    v4si result = __builtin_shufflevector(v, v, 0, 1, 2, 3);
    *out = result;
}

/* Vector compound literal with constant indices */
NOINLINE static void test_vector_constructor(v4si *vec, v4si *out) {
    v4si v = *vec;
    /* Constant-bounded section via constructor */
    v4si result = (v4si){v[0], v[1], v[2], v[3]};
    *out = result;
}

/* Vector permutation with constant mask */
NOINLINE static void test_vector_permute(v4sf *vec1, v4sf *vec2, v4sf *out) {
    v4sf a = *vec1;
    v4sf b = *vec2;
    /* Constant permutation mask */
    const int mask[4] = {0, 2, 1, 3};
    v4sf result = __builtin_shuffle(a, b, mask[0], mask[1], mask[2], mask[3]);
    *out = result;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(void) {
    int checksum = 0;
    int temp_result;
    
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    v2si v2 = {10, 20};
    v4si v4 = {1, 2, 3, 4};
    v4sf v4f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v4f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4si v4_result;
    v4sf v4f_result;
    
    /* Fill arrays with deterministic values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 3 + 1;
        char_array[i] = (i % 26) + 'A';
    }
    
    /* Execute all test scenarios */
    
    /* Scenario 1: Small count memory references */
    test_small_count_1(int_array, &temp_result);
    checksum += temp_result;
    
    test_small_count_2(int_array, &temp_result);
    checksum += temp_result;
    
    test_small_count_vector(&v2, &temp_result);
    checksum += temp_result;
    
    /* Scenario 2: Larger constant-sized memory accesses */
    test_larger_constant_array(int_array, &temp_result);
    checksum += temp_result;
    
    test_larger_constant_char(char_array, &temp_result);
    checksum += temp_result;
    
    test_volatile_bounds(int_array, &temp_result);
    checksum += temp_result;
    
    /* Scenario 3: Non-memory vector operations */
    test_vector_shuffle(&v4, &v4_result);
    checksum += v4_result[0] + v4_result[1];
    
    test_vector_constructor(&v4, &v4_result);
    checksum += v4_result[2] + v4_result[3];
    
    test_vector_permute(&v4f1, &v4f2, &v4f_result);
    checksum += (int)v4f_result[0] + (int)v4f_result[1];
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
