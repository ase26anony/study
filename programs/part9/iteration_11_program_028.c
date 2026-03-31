/* Test program for constant-bounds array/vector analysis in GCC expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating test cases */
static volatile int g_volatile = 0;

/* Vector types for non-memory reference cases */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v4f __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) memory references ========== */

/* Single element access - count = 1 */
__attribute__((noinline))
static int test_small_count_1(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=5, hi=5, count=1 */
    if (g_volatile) {
        sum += arr[5];  /* Single element load */
    } else {
        arr[5] = 42;    /* Single element store */
        sum = arr[5];
    }
    return sum;
}

/* Two adjacent elements - count = 2 */
__attribute__((noinline))
static int test_small_count_2(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=10, hi=11, count=2 */
    if (g_volatile) {
        sum += arr[10] + arr[11];  /* Two element loads */
    } else {
        arr[10] = 1;    /* Two element stores */
        arr[11] = 2;
        sum = arr[10] + arr[11];
    }
    return sum;
}

/* Two-element struct access */
struct two_int { int a; int b; };

__attribute__((noinline))
static int test_small_struct(struct two_int *s) {
    /* Constant bounds for struct members */
    if (g_volatile) {
        return s->a + s->b;  /* Accesses two adjacent ints */
    }
    s->a = 10;
    s->b = 20;
    return s->a + s->b;
}

/* ========== SCENARIO 2: Larger constant-sized memory access ========== */

/* Fixed-size array slice in loop */
__attribute__((noinline))
static int test_larger_slice(int *arr) {
    int sum = 0;
    /* Constant bounds: lo=2, hi=9, count=8 */
    /* TYPE_SIZE(int) = 32 bits, total = 32 * 8 = 256 bits (fits in uhwi) */
    for (int i = 2; i < 10; ++i) {  /* Compile-time constant bounds */
        arr[i] = i * 2;
        sum += arr[i];
    }
    return sum;
}

/* Multiple constant-sized slices with different element types */
__attribute__((noinline))
static int test_mixed_sizes(char *carr, short *sarr, int *iarr) {
    int sum = 0;
    
    /* char array: lo=0, hi=31, count=32, total=32*8=256 bits */
    for (int i = 0; i < 32; ++i) {
        carr[i] = (char)(i + 1);
        sum += carr[i];
    }
    
    /* short array: lo=5, hi=20, count=16, total=16*16=256 bits */
    for (int i = 5; i < 21; ++i) {
        sarr[i] = (short)(i * 3);
        sum += sarr[i];
    }
    
    /* int array: lo=100, hi=115, count=16, total=16*32=512 bits */
    for (int i = 100; i < 116; ++i) {
        iarr[i] = i * 5;
        sum += iarr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-memory vector operations ========== */

/* Vector shuffle with constant indices */
__attribute__((noinline))
static v4si test_vector_shuffle(v4si a, v4si b) {
    /* Constant bounds in shuffle operation */
    v4si result;
    if (g_volatile) {
        /* Shuffle with constant indices 0,2,1,3 */
        result = __builtin_shufflevector(a, b, 0, 2, 1, 3);
    } else {
        /* Different constant indices 3,1,2,0 */
        result = __builtin_shufflevector(a, b, 3, 1, 2, 0);
    }
    return result;
}

/* Vector constructor with constant element selection */
__attribute__((noinline))
static v4si test_vector_constructor(v4si v) {
    /* Create new vector from constant-indexed elements */
    v4si result;
    if (g_volatile) {
        /* Constant bounds: accessing elements 0,1,2,3 */
        result = (v4si){v[0], v[1], v[2], v[3]};
    } else {
        /* Different constant indices */
        result = (v4si){v[3], v[2], v[1], v[0]};
    }
    return result;
}

/* Vector slice extraction */
__attribute__((noinline))
static v2si test_vector_slice(v4si v) {
    /* Extract first two elements - constant bounds lo=0, hi=1 */
    v2si result;
    if (g_volatile) {
        result = (v2si){v[0], v[1]};
    } else {
        result = (v2si){v[2], v[3]};
    }
    return result;
}

/* ========== SCENARIO 4: Complex constant bounds with volatile control ========== */

__attribute__((noinline))
static int test_complex_bounds(int *arr) {
    int sum = 0;
    /* Use volatile to prevent constant propagation of bounds */
    volatile int base = g_volatile ? 20 : 30;
    
    /* The bounds themselves are constant, but chosen via volatile */
    int lo = base + 0;  /* Constant expression: base + 0 */
    int hi = base + 7;  /* Constant expression: base + 7 */
    
    /* lo and hi are constant at compile time for each path */
    for (int i = lo; i < hi; ++i) {
        arr[i] = i * 10;
        sum += arr[i];
    }
    return sum;
}

/* ========== SCENARIO 5: Multi-dimensional array constant bounds ========== */

__attribute__((noinline))
static int test_multi_dim(int arr[][10]) {
    int sum = 0;
    /* Constant bounds in 2D array */
    for (int i = 2; i < 6; ++i) {
        for (int j = 3; j < 8; ++j) {  /* Both bounds constant */
            arr[i][j] = i * j;
            sum += arr[i][j];
        }
    }
    return sum;
}

/* ========== Main test driver ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize test data */
    int int_array[200] = {0};
    char char_array[100] = {0};
    short short_array[100] = {0};
    int multi_array[10][10] = {0};
    struct two_int mystruct = {0, 0};
    
    /* Vector initialization */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Run all test scenarios */
    checksum += test_small_count_1(int_array);
    checksum += test_small_count_2(int_array);
    checksum += test_small_struct(&mystruct);
    checksum += test_larger_slice(int_array);
    checksum += test_mixed_sizes(char_array, short_array, int_array);
    
    v4si vec_result = test_vector_shuffle(vec1, vec2);
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    vec_result = test_vector_constructor(vec1);
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    v2si vec_slice = test_vector_slice(vec1);
    checksum += vec_slice[0] + vec_slice[1];
    
    checksum += test_complex_bounds(int_array);
    checksum += test_multi_dim(multi_array);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
