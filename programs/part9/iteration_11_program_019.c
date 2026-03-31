/* Test program for constant-bounds analysis in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;
static volatile int g_volatile_one = 1;

/* Vector types for non-MEM_P scenarios */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) memory accesses ========== */

/* Single element access - count = 1 */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Constant bounds: arr[5] to arr[5] */
    sum += arr[5];  /* Single element - count = 1 */
    return sum;
}

/* Two adjacent elements - count = 2 */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Constant bounds: arr[3] to arr[4] */
    sum += arr[3] + arr[4];  /* Two elements - count = 2 */
    return sum;
}

/* Two-element struct access */
struct two_int { int a; int b; };

__attribute__((noinline))
static int test_small_struct(struct two_int *s) {
    /* Access both elements - count = 2 for the pair */
    return s->a + s->b;
}

/* Vector with two-element access */
__attribute__((noinline))
static v2si test_small_vector(v4si *vec) {
    /* Extract first two elements - constant bounds [0,1] */
    v2si result;
    result[0] = (*vec)[0];
    result[1] = (*vec)[1];
    return result;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Array slice with constant bounds, larger count */
__attribute__((noinline))
static int test_larger_slice(int *arr) {
    int sum = 0;
    /* Constant bounds: arr[2] to arr[9], count = 8 */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Different element type with known size */
__attribute__((noinline))
static long test_char_slice(char *data) {
    long sum = 0;
    /* Constant bounds: data[10] to data[49], count = 40 */
    for (int i = 10; i < 50; ++i) {
        sum += data[i];
    }
    return sum;
}

/* Mixed with volatile to prevent elimination */
__attribute__((noinline))
static int test_mixed_bounds(int *arr) {
    int sum = 0;
    /* Use volatile in condition to preserve analysis */
    int limit = g_volatile_zero ? 5 : 20;  /* Compiler sees both possibilities */
    
    /* Constant bounds in actual loop: arr[0] to arr[19], count = 20 */
    for (int i = 0; i < limit; ++i) {
        if (i < 20)  /* Always true at runtime when limit=20 */
            sum += arr[i];
    }
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices - not a MEM_P */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Constant bounds in shuffle: indices 0,2,4,6 */
    v4si result = __builtin_shufflevector(a, b, 0, 2, 4, 6);
    return result;
}

/* Vector constructor with constant indices */
__attribute__((noinline))
static v4si test_vector_constructor(v4si vec) {
    /* Extract specific elements with constant indices */
    v4si result = (v4si){vec[0], vec[2], vec[1], vec[3]};
    return result;
}

/* Compound literal with slice */
__attribute__((noinline))
static v4sf test_float_vector_slice(v4sf vec) {
    /* Reorganize with constant indices */
    v4sf result = (v4sf){vec[3], vec[1], vec[0], vec[2]};
    return result;
}

/* Vector permutation expression */
__attribute__((noinline))
static v8hi test_vector_permute(v8hi a, v8hi b) {
    /* Create a permutation pattern */
    v8hi mask = {0, 8, 2, 10, 4, 12, 6, 14};
    v8hi result = __builtin_shufflevector(a, b, 0, 8, 2, 10, 4, 12, 6, 14);
    return result;
}

/* ========== SCENARIO 4: Complex cases with multiple conditions ========== */

/* Nested loops with constant inner bounds */
__attribute__((noinline))
static int test_nested_constant_bounds(int arr[][10]) {
    int sum = 0;
    /* Outer loop uses volatile, inner has constant bounds */
    for (int i = g_volatile_zero; i < 3; ++i) {
        /* Inner loop: constant bounds j=2 to j=7, count=6 */
        for (int j = 2; j < 8; ++j) {
            sum += arr[i][j];
        }
    }
    return sum;
}

/* Switch with different constant bounds */
__attribute__((noinline))
static int test_switch_bounds(int *arr, int mode) {
    int sum = 0;
    switch (mode) {
        case 0:
            /* Constant bounds: arr[0] to arr[4], count=5 */
            for (int i = 0; i < 5; ++i) sum += arr[i];
            break;
        case 1:
            /* Constant bounds: arr[5] to arr[9], count=5 */
            for (int i = 5; i < 10; ++i) sum += arr[i];
            break;
        default:
            /* Constant bounds: arr[10] to arr[14], count=5 */
            for (int i = 10; i < 15; ++i) sum += arr[i];
            break;
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main() {
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    struct two_int mystruct = {42, 17};
    int matrix[5][10];
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    v8hi short_vec1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi short_vec2 = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Fill arrays with deterministic values */
    for (int i = 0; i < 100; ++i) {
        int_array[i] = i * 3 + 1;
        char_array[i] = (i % 26) + 'A';
    }
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 10; ++j) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    int checksum = 0;
    
    /* Execute all test scenarios */
    checksum += test_small_count_1(int_array);
    checksum += test_small_count_2(int_array);
    checksum += test_small_struct(&mystruct);
    
    v2si small_vec_result = test_small_vector(&vec1);
    checksum += small_vec_result[0] + small_vec_result[1];
    
    checksum += test_larger_slice(int_array);
    checksum += test_char_slice(char_array);
    checksum += test_mixed_bounds(int_array);
    
    v4si shuffle_result = test_vector_shuffle(vec1, vec2);
    for (int i = 0; i < 4; ++i) checksum += shuffle_result[i];
    
    v4si constr_result = test_vector_constructor(vec1);
    for (int i = 0; i < 4; ++i) checksum += constr_result[i];
    
    v4sf slice_result = test_float_vector_slice(fvec);
    checksum += (int)slice_result[0] + (int)slice_result[1];
    
    v8hi perm_result = test_vector_permute(short_vec1, short_vec2);
    for (int i = 0; i < 8; ++i) checksum += perm_result[i];
    
    checksum += test_nested_constant_bounds(matrix);
    checksum += test_switch_bounds(int_array, g_volatile_one);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
