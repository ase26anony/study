/* Test program to cover constant bounds analysis in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test cases */
static volatile int g_volatile_zero = 0;
static volatile int g_volatile_one = 1;

/* Vector types for non-memory reference cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== Scenario 1: Small element count (count <= 2) ========== */
__attribute__((noinline))
static int test_small_count_memory(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Single element access - count = 1 */
    if (g_volatile_zero) {
        /* This path won't be taken at runtime, but compiler sees it */
        sum += arr[5];  /* const_bounds_p: lo=5, hi=5, count=1 <= 2 */
    } else {
        sum += arr[10]; /* const_bounds_p: lo=10, hi=10, count=1 <= 2 */
    }
    
    /* Two adjacent elements - count = 2 */
    /* Use volatile to prevent constant folding */
    int idx = g_volatile_zero ? 20 : 30;
    sum += arr[idx] + arr[idx + 1];  /* lo=idx, hi=idx+1, count=2 <= 2 */
    
    /* Vector type with 2 elements */
    typedef int v2si __attribute__((vector_size(8)));
    v2si vec2 = {1, 2};
    int* p = (int*)&vec2;
    sum += p[0] + p[1];  /* Access both elements, count=2 <= 2 */
    
    return sum;
}

/* ========== Scenario 2: Larger constant-sized memory access ========== */
__attribute__((noinline))
static int test_larger_constant_sized_access(void) {
    int arr[100];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    /* Access 8 elements - count=8, TYPE_SIZE(int)=32, total=256 bits */
    /* This should fit in unsigned HWI on 64-bit systems */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        sum += arr[i];
    }
    
    /* Another constant-sized slice with char type */
    char char_arr[200];
    for (int i = 0; i < 200; i++) {
        char_arr[i] = (char)(i % 100);
    }
    
    /* Access 100 chars - count=100, TYPE_SIZE(char)=8, total=800 bits */
    for (int i = 50; i < 150; ++i) {  /* lo=50, hi=149, count=100 */
        sum += char_arr[i];
    }
    
    /* Mixed with volatile condition to preserve analysis */
    int start = g_volatile_zero ? 15 : 25;
    int end = start + 20;  /* start and end are runtime, but difference is constant */
    for (int i = start; i < end; ++i) {  /* lo=start, hi=end-1, count=20 */
        sum += arr[i];
    }
    
    return sum;
}

/* ========== Scenario 3: Non-memory vector operations ========== */
__attribute__((noinline))
static v4si test_non_memory_vector_ops(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory reference */
    v4si result;
    
    /* Use __builtin_shuffle with constant mask */
    if (g_volatile_zero) {
        /* Shuffle that selects first two elements from a, last two from b */
        result = __builtin_shufflevector(a, b, 0, 1, 4, 5);
    } else {
        /* Different constant shuffle */
        result = __builtin_shufflevector(a, b, 2, 3, 6, 7);
    }
    
    /* Vector compound literal with constant indices */
    v4si slice;
    if (g_volatile_one) {
        /* Create new vector from constant indices */
        slice = (v4si){a[0], a[1], a[2], a[3]};  /* All constant indices */
    } else {
        slice = (v4si){a[1], a[2], a[3], a[0]};  /* Different constant indices */
    }
    
    /* Combine results */
    return result + slice;
}

/* ========== Scenario 4: Mixed array/vector with constant bounds ========== */
__attribute__((noinline))
static int test_mixed_constant_bounds(void) {
    int arr[50];
    v4si vec[10];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) arr[i] = i;
    for (int i = 0; i < 10; i++) {
        vec[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
    }
    
    /* Array section with compile-time constant bounds */
    /* The optimizer should recognize this as constant-bounded */
    int lo = g_volatile_zero ? 5 : 10;
    int hi = lo + 15;  /* Constant difference */
    for (int i = lo; i < hi; i++) {  /* lo and hi are runtime, but count=15 constant */
        sum += arr[i];
    }
    
    /* Vector array access with constant index */
    int* vdata = (int*)&vec[2];  /* vec[2] is constant index */
    for (int i = 0; i < 4; i++) {  /* count=4, TYPE_SIZE(int)=32, total=128 bits */
        sum += vdata[i];
    }
    
    /* Multi-dimensional array with constant inner bounds */
    int md_arr[10][20];
    for (int i = 0; i < 10; i++) {
        for (int j = 5; j < 15; j++) {  /* Inner loop has constant bounds: lo=5, hi=14, count=10 */
            md_arr[i][j] = i * j;
            sum += md_arr[i][j];
        }
    }
    
    return sum;
}

/* ========== Scenario 5: Edge cases with different types ========== */
__attribute__((noinline))
static int test_edge_cases(void) {
    /* Test with different element types and sizes */
    short short_arr[100];
    long long ll_arr[50];
    float float_arr[75];
    
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) short_arr[i] = (short)(i % 1000);
    for (int i = 0; i < 50; i++) ll_arr[i] = i * 1000LL;
    for (int i = 0; i < 75; i++) float_arr[i] = i * 1.5f;
    
    /* Short array - many elements but small total size */
    /* 64 elements * 16 bits = 1024 bits */
    for (int i = 10; i < 74; i++) {  /* lo=10, hi=73, count=64 */
        sum += short_arr[i];
    }
    
    /* long long array - few elements, large size per element */
    /* 4 elements * 64 bits = 256 bits */
    for (int i = 20; i < 24; i++) {  /* lo=20, hi=23, count=4 */
        sum += (int)(ll_arr[i] % 1000);
    }
    
    /* Float array with constant bounds in nested loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Inner loop bounds are constant relative to outer */
        int start = outer * 10;
        int end = start + 8;  /* Constant count of 8 */
        for (int i = start; i < end; i++) {  /* lo=start, hi=end-1, count=8 */
            sum += (int)float_arr[i];
        }
    }
    
    return sum;
}

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all test functions */
    checksum += test_small_count_memory();
    checksum += test_larger_constant_sized_access();
    
    v4si vec_result = test_non_memory_vector_ops();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_constant_bounds();
    checksum += test_edge_cases();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
