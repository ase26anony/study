/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targets lines 7691-7700 in expr.cc for MEM_P and non-MEM_P
   paths with constant-bounded array/vector sections. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from eliminating test code */
static volatile int vol_zero = 0;
static volatile int vol_one = 1;

/* Vector types for non-MEM_P tests */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== SCENARIO 1: Small count (count <= 2) MEM_P path ========== */
static int __attribute__((noinline))
test_small_count_mem(void)
{
    int arr[100];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 100; i++)
        arr[i] = i * 3 + 1;
    
    /* Single element access - count = 1 */
    if (vol_zero) {
        /* This branch won't execute but prevents dead code elimination */
        sum += arr[99];
    } else {
        /* Constant bounds: lo=5, hi=5, count=1 */
        sum += arr[5];
    }
    
    /* Two adjacent elements - count = 2 */
    /* Use volatile to prevent constant folding of bounds */
    int lo = vol_one ? 10 : 20;
    if (lo == 10) {
        /* Constant bounds: lo=10, hi=11, count=2 */
        sum += arr[10] + arr[11];
    }
    
    /* Vector type with 2 elements */
    typedef int v2si __attribute__((vector_size(8)));
    v2si v2;
    int *p = (int*)&v2;
    p[0] = 100;
    p[1] = 200;
    
    /* Access both elements - still count=2 */
    int idx = vol_zero ? 1 : 0;
    sum += p[idx] + p[idx+1];
    
    return sum;
}

/* ========== SCENARIO 2: Larger constant-sized MEM_P access ========== */
static int __attribute__((noinline))
test_larger_constant_mem(void)
{
    /* Element type with constant size: int (usually 32 bits) */
    int arr[100];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++)
        arr[i] = i * 2;
    
    /* Constant-bounded loop: lo=2, hi=9, count=8 */
    /* TYPE_SIZE(int) * count = 32 * 8 = 256 bits, fits in uhwi */
    for (int i = 2; i < 10; ++i) {
        sum += arr[i];
    }
    
    /* Another constant slice with different bounds */
    /* lo=20, hi=39, count=20, total bits = 32*20=640 */
    int start = vol_one ? 20 : 30;
    if (start == 20) {
        for (int i = start; i < start + 20; ++i) {
            sum += arr[i] * 2;
        }
    }
    
    /* Char array: smaller element size */
    char carr[200];
    for (int i = 0; i < 200; i++)
        carr[i] = (char)(i % 100);
    
    /* lo=50, hi=149, count=100, total bits = 8*100=800 */
    for (int i = 50; i < 150; ++i) {
        sum += carr[i];
    }
    
    return sum;
}

/* ========== SCENARIO 3: Non-MEM_P vector operations ========== */
static v4si __attribute__((noinline))
test_non_mem_vector(void)
{
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a memory reference */
    /* This creates a VEC_PERM_EXPR or CONSTRUCTOR node */
    v4si v3;
    
    /* Use __builtin_shuffle with constant mask */
    int mask_v = vol_zero ? 3 : 0;
    if (mask_v == 0) {
        /* Constant bounds in shuffle: accessing elements 0,1,2,3 from v1 */
        v3 = __builtin_shufflevector(v1, v2, 0, 1, 2, 3);
    } else {
        /* Alternative constant pattern */
        v3 = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    }
    
    /* Vector compound literal with constant indices */
    v4si v4;
    int idx = vol_one ? 0 : 2;
    if (idx == 0) {
        /* Constant slice: elements 0 and 1 from v1, 2 and 3 from v2 */
        v4 = (v4si){v1[0], v1[1], v2[2], v2[3]};
    }
    
    /* Vector operation that might be analyzed as constant-bounded section */
    return v3 + v4;
}

/* ========== SCENARIO 4: Mixed array/vector with volatile bounds ========== */
static int __attribute__((noinline))
test_mixed_volatile_bounds(void)
{
    int arr[50];
    v4si vec[5];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) arr[i] = i;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            vec[i][j] = i * 10 + j;
        }
    }
    
    /* Use volatile in condition to preserve both paths during analysis */
    int bound = vol_one ? 5 : 15;
    
    /* Array section with constant bounds determined by volatile condition */
    for (int i = (bound == 5 ? 2 : 10); i < (bound == 5 ? 8 : 20); ++i) {
        sum += arr[i];
    }
    
    /* Vector element access with constant indices */
    int vec_idx = vol_zero ? 0 : 1;
    if (vec_idx == 0) {
        /* Access constant elements from vector array */
        sum += vec[0][0] + vec[0][1] + vec[0][2] + vec[0][3];
    }
    
    /* Multi-dimensional array with constant inner dimension */
    int md_arr[10][20];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            md_arr[i][j] = i * 20 + j;
    
    /* Constant slice in 2D array */
    int row = vol_one ? 3 : 5;
    if (row == 3) {
        for (int j = 5; j < 15; j++) {
            sum += md_arr[row][j];
        }
    }
    
    return sum;
}

/* ========== MAIN: Combine all scenarios ========== */
int main(void)
{
    int checksum = 0;
    
    checksum += test_small_count_mem();
    checksum += test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_volatile_bounds();
    
    /* Print checksum to prevent optimization and verify execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
