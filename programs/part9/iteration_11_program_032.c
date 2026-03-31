/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <string.h>

/* Prevent optimization from eliminating code */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small element count (count <= 2) ========== */

/* Single element access - count = 1 */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=5, hi=5, count=1 */
    sum += arr[5];
    return sum;
}

/* Two adjacent elements - count = 2 */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=10, hi=11, count=2 */
    sum += arr[10] + arr[11];
    return sum;
}

/* Two-element struct access */
struct TwoInts { int a; int b; };

__attribute__((noinline))
static int test_small_struct(struct TwoInts *s) {
    /* Access both elements - count = 2 */
    return s[g_volatile ? 0 : 3].a + s[g_volatile ? 0 : 3].b;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Larger block with constant bounds where TYPE_SIZE * count fits in uhwi */
__attribute__((noinline))
static int test_larger_constant_block(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=2, hi=9, count=8 */
    /* TYPE_SIZE(int) = 32 bits, 32 * 8 = 256 bits fits in uhwi */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Different element type (char) with larger count */
__attribute__((noinline))
static int test_char_array(char *arr) {
    int sum = 0;
    /* Constant bounds: lo=0, hi=99, count=100 */
    /* TYPE_SIZE(char) = 8 bits, 8 * 100 = 800 bits fits in uhwi */
    for (int i = 0; i < 100; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* Mixed bounds using volatile to prevent elimination */
__attribute__((noinline))
static int test_mixed_bounds(float *arr) {
    float sum = 0.0f;
    /* Use volatile to choose between two constant bounds */
    int start = g_volatile ? 5 : 10;
    int end = g_volatile ? 15 : 20;
    
    /* Both paths have constant bounds */
    if (g_volatile) {
        /* lo=5, hi=15, count=11 */
        for (int i = start; i <= end; ++i) {
            sum += arr[i];
        }
    } else {
        /* lo=10, hi=20, count=11 */
        for (int i = start; i <= end; ++i) {
            sum += arr[i] * 2.0f;
        }
    }
    return (int)sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices - not a MEM_REF */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Constant indices for shuffle */
    return __builtin_shufflevector(a, b, 0, 1, 4, 5);
}

/* Vector compound literal with constant indices */
__attribute__((noinline))
static v4si test_vector_constructor(v4si v) {
    /* Create new vector from constant indices */
    return (v4si){v[0], v[1], v[2], v[3]};
}

/* Vector slice using shuffle */
__attribute__((noinline))
static v2si test_vector_slice(v4si v) {
    /* Extract first two elements with constant indices */
    v2si result;
    result[0] = v[0];
    result[1] = v[1];
    return result;
}

/* Vector permutation with constant mask */
__attribute__((noinline))
static v4sf test_vector_permute(v4sf a, v4sf b) {
    /* Reverse order with constant indices */
    return __builtin_shufflevector(a, b, 3, 2, 1, 0);
}

/* ========== SCENARIO 4: Complex cases with nested structures ========== */

/* Array of vectors with constant indexing */
__attribute__((noinline))
static int test_vector_array(v4si *vec_arr) {
    int sum = 0;
    /* Access specific vector elements with constant indices */
    sum += vec_arr[2][0] + vec_arr[2][1] + vec_arr[2][2] + vec_arr[2][3];
    return sum;
}

/* Multi-dimensional array with constant bounds */
__attribute__((noinline))
static int test_multi_dim(int arr[][10]) {
    int sum = 0;
    /* Constant bounds in both dimensions */
    for (int i = 1; i < 4; ++i) {
        for (int j = 2; j < 7; ++j) {
            sum += arr[i][j];
        }
    }
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main() {
    int checksum = 0;
    
    /* Initialize test data */
    int int_array[100];
    char char_array[100];
    float float_array[100];
    struct TwoInts struct_array[10];
    v4si vector_array[10];
    int multi_dim[5][10];
    v4si vec1, vec2;
    v4sf vecf1, vecf2;
    
    /* Initialize with some values */
    for (int i = 0; i < 100; ++i) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
        float_array[i] = (float)i;
        if (i < 10) {
            struct_array[i].a = i * 2;
            struct_array[i].b = i * 2 + 1;
            vector_array[i] = (v4si){i, i+1, i+2, i+3};
        }
        if (i < 5) {
            for (int j = 0; j < 10; ++j) {
                multi_dim[i][j] = i * 10 + j;
            }
        }
    }
    
    vec1 = (v4si){1, 2, 3, 4};
    vec2 = (v4si){5, 6, 7, 8};
    vecf1 = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    vecf2 = (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Test all scenarios */
    checksum += test_small_count_1(int_array);
    checksum += test_small_count_2(int_array);
    checksum += test_small_struct(struct_array);
    checksum += test_larger_constant_block(int_array);
    checksum += test_char_array(char_array);
    checksum += test_mixed_bounds(float_array);
    
    /* Non-memory vector operations */
    v4si shuffled = test_vector_shuffle(vec1, vec2);
    checksum += shuffled[0] + shuffled[1] + shuffled[2] + shuffled[3];
    
    v4si constructed = test_vector_constructor(vec1);
    checksum += constructed[0] + constructed[1];
    
    v2si sliced = test_vector_slice(vec1);
    checksum += sliced[0] + sliced[1];
    
    v4sf permuted = test_vector_permute(vecf1, vecf2);
    checksum += (int)permuted[0] + (int)permuted[1];
    
    /* Complex cases */
    checksum += test_vector_array(vector_array);
    checksum += test_multi_dim(multi_dim);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
