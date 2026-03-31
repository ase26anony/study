/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targets lines 7691-7700 in expr.cc. */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test functions */
#define NOINLINE __attribute__((noinline))

/* Volatile to prevent constant propagation from removing code */
extern volatile int g_volatile_zero;

/* Vector types for non-memory reference tests */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element access - count = 1 */
NOINLINE static void test_small_count_1(int *arr, int *out) {
    /* arr[5] is constant bounded: lo=5, hi=5, count=1 <= 2 */
    *out = arr[5];
}

/* Two adjacent elements - count = 2 */
NOINLINE static void test_small_count_2(int *arr, int *out) {
    /* Access arr[2] and arr[3] - constant bounds, count=2 */
    int sum = arr[2] + arr[3];
    *out = sum;
}

/* Two-element struct access */
struct two_int { int a; int b; };
NOINLINE static void test_small_struct(struct two_int *s, int *out) {
    /* Access s[0].a and s[0].b - effectively two elements */
    *out = s[0].a + s[0].b;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Fixed-size array slice with constant bounds */
NOINLINE static void test_larger_slice(int *arr, int *out) {
    /* lo=2, hi=9, count=8, element size=32 bits, total=256 bits */
    int sum = 0;
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Larger slice with char type */
NOINLINE static void test_larger_slice_char(char *arr, int *out) {
    /* lo=10, hi=49, count=40, element size=8 bits, total=320 bits */
    int sum = 0;
    for (int i = 10; i < 50; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* Mixed bounds using volatile to preserve analysis */
NOINLINE static void test_mixed_bounds(float *arr, float *out) {
    /* Use volatile to prevent constant folding but keep bounds constant */
    int start = g_volatile_zero ? 5 : 0;
    int end = g_volatile_zero ? 15 : 20;
    
    /* Both bounds are compile-time constants (0 and 20 or 5 and 15) */
    float sum = 0.0f;
    for (int i = start; i < end; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices */
NOINLINE static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Create constant-bounded vector section via shuffle */
    return __builtin_shufflevector(a, b, 0, 1, 4, 5);  /* First 2 from a, first 2 from b */
}

/* Vector compound literal with constant indices */
NOINLINE static v4si test_vector_constructor(v4si v) {
    /* Extract constant-bounded section via constructor */
    return (v4si){v[0], v[1], v[2], v[3]};  /* All elements, but constant indices */
}

/* Partial vector extraction */
NOINLINE static v8hi test_partial_vector(v8hi v) {
    /* Extract first 4 elements (constant bounds) */
    return (v8hi){v[0], v[1], v[2], v[3], 0, 0, 0, 0};
}

/* ========== SCENARIO 4: Complex cases with multiple conditions ========== */

/* Nested loops with constant bounds */
NOINLINE static void test_nested_constant_bounds(int arr[][10], int *out) {
    int sum = 0;
    /* Both loops have constant bounds */
    for (int i = 2; i < 6; ++i) {
        for (int j = 3; j < 8; ++j) {
            sum += arr[i][j];
        }
    }
    *out = sum;
}

/* Conditional constant bounds */
NOINLINE static void test_conditional_bounds(int *arr, int cond, int *out) {
    int sum = 0;
    /* Bounds are constant in each path */
    int start = cond ? 0 : 5;
    int end = cond ? 10 : 20;
    
    for (int i = start; i < end; ++i) {
        sum += arr[i];
    }
    *out = sum;
}

/* ========== Main test driver ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    int int_array[100] = {0};
    for (int i = 0; i < 100; i++) {
        int_array[i] = i + 1;
    }
    
    char char_array[100] = {0};
    for (int i = 0; i < 100; i++) {
        char_array[i] = (char)(i % 128);
    }
    
    float float_array[100] = {0};
    for (int i = 0; i < 100; i++) {
        float_array[i] = (float)i * 0.5f;
    }
    
    int matrix[10][10] = {0};
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    struct two_int s = {42, 24};
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_c = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Execute all test scenarios */
    int result;
    
    /* Scenario 1: Small count */
    test_small_count_1(int_array, &result);
    checksum += result;
    
    test_small_count_2(int_array, &result);
    checksum += result;
    
    test_small_struct(&s, &result);
    checksum += result;
    
    /* Scenario 2: Larger slices */
    test_larger_slice(int_array, &result);
    checksum += result;
    
    test_larger_slice_char(char_array, &result);
    checksum += result;
    
    test_mixed_bounds(float_array, (float*)&result);
    checksum += result;
    
    /* Scenario 3: Non-memory vector ops */
    v4si vec_result = test_vector_shuffle(vec_a, vec_b);
    checksum += vec_result[0] + vec_result[1];
    
    vec_result = test_vector_constructor(vec_a);
    checksum += vec_result[2] + vec_result[3];
    
    v8hi vec_result2 = test_partial_vector(vec_c);
    checksum += vec_result2[0] + vec_result2[1];
    
    /* Scenario 4: Complex cases */
    test_nested_constant_bounds(matrix, &result);
    checksum += result;
    
    test_conditional_bounds(int_array, 1, &result);
    checksum += result;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
