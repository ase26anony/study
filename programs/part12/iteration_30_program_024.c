/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent dead code elimination */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Reference scalar implementations for verification */
static void ref_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void ref_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static uint32_t ref_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels - these should be vectorized */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

static uint32_t test_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test for mixed true/false results */
static void test_mixed_comparisons(int *results, const int *a, const int *b) {
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators */
        int r = 0;
        r |= (a[i] > b[i]) ? 0x1 : 0;
        r |= (a[i] >= b[i]) ? 0x2 : 0;
        r |= (a[i] < b[i]) ? 0x4 : 0;
        r |= (a[i] <= b[i]) ? 0x8 : 0;
        results[i] = r;
    }
    escape(results);
}

int main() {
    int ret = 0;
    
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED uint32_t src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED int mixed_results[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* For integer tests: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i - 1 : 
                     (i % 3 == 1) ? i + 1 : i;
        
        /* For unsigned tests: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* For float tests: create some NaN/inf cases */
        src1_float[i] = (i % 5 == 0) ? (float)i * 1.5f : (float)i;
        src2_float[i] = (i % 5 == 1) ? (float)i * 1.5f : (float)(i + 1);
        
        /* For double tests */
        src1_double[i] = (i % 7 == 0) ? (double)i * 0.5 : (double)i;
        src2_double[i] = (i % 7 == 1) ? (double)i * 0.5 : (double)(i - 1);
    }
    
    printf("Testing GT_EXPR (integer)...\n");
    test_gt_int(dest_int, src1_int, src2_int, 999, -999);
    ref_gt_int(ref_int, src1_int, src2_int, 999, -999);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing GE_EXPR (float)...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR (unsigned)...\n");
    uint32_t vec_count = test_lt_unsigned(src1_uint, src2_uint);
    uint32_t ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAIL: LT_EXPR unsigned test: vec=%u, ref=%u\n", vec_count, ref_count);
        ret = 1;
    }
    
    printf("Testing LE_EXPR (double)...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR (integer)...\n");
    test_lt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src2_int, src1_int, 100, 200); /* Note: a<b is same as b>a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing LE_EXPR (integer)...\n");
    test_le_int(dest_int, src1_int, src2_int, 300, 400);
    ref_ge_int(ref_int, src2_int, src1_int, 300, 400); /* Note: a<=b is same as b>=a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LE_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int);
    /* Verify a few positions */
    for (int i = 0; i < 10; i++) {
        int expected = 0;
        expected |= (src1_int[i] > src2_int[i]) ? 0x1 : 0;
        expected |= (src1_int[i] >= src2_int[i]) ? 0x2 : 0;
        expected |= (src1_int[i] < src2_int[i]) ? 0x4 : 0;
        expected |= (src1_int[i] <= src2_int[i]) ? 0x8 : 0;
        if (mixed_results[i] != expected) {
            printf("FAIL: mixed comparison at i=%d: got=0x%x, expected=0x%x\n", 
                   i, mixed_results[i], expected);
            ret = 1;
            break;
        }
    }
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}

/* Helper reference function for LE test */
static void ref_ge_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
}
