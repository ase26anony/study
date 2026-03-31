#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Reference implementations */
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

static unsigned ref_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels - should trigger vectorization transformations */
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

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
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

/* Additional tests for LT/LE with swapped operands */
static void test_lt_int_swapped(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src2[i] < src1[i] ? val1 : val2;  /* Equivalent to src1[i] > src2[i] */
    }
    escape(dest);
}

static void test_le_float_swapped(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src2[i] <= src1[i] ? val1 : val2;  /* Equivalent to src1[i] >= src2[i] */
    }
    escape(dest);
}

/* Mixed pattern tests */
static void test_mixed_comparisons(int *dest_int, float *dest_float,
                                   const int *src1_int, const int *src2_int,
                                   const float *src1_float, const float *src2_float) {
    for (int i = 0; i < N; i++) {
        /* GT for integers */
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        
        /* LE for floats */
        dest_float[i] = src1_float[i] <= src2_float[i] ? 3.14f : -3.14f;
    }
    escape(dest_int);
    escape(dest_float);
}

int main() {
    int result = 0;
    
    /* Aligned arrays */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* For integers: alternating pattern */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* For unsigned: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* For floats: sine-like pattern */
        src1_float[i] = (i % 8) * 0.125f;
        src2_float[i] = ((i + 2) % 8) * 0.125f;
        
        /* For doubles: similar pattern */
        src1_double[i] = (i % 16) * 0.0625;
        src2_double[i] = ((i + 4) % 16) * 0.0625;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 999, -999);
    ref_gt_int(ref_int, src1_int, src2_int, 999, -999);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        result = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        result = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAIL: LT_EXPR unsigned test (vec=%u, ref=%u)\n", vec_count, ref_count);
        result = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 2.71828, -2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 2.71828, -2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        result = 1;
    }
    
    printf("Testing LT_EXPR with swapped operands (integers)...\n");
    test_lt_int_swapped(dest_int, src1_int, src2_int, 777, -777);
    ref_gt_int(ref_int, src1_int, src2_int, 777, -777);  /* Should give same result as GT */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR swapped test\n");
        result = 1;
    }
    
    printf("Testing LE_EXPR with swapped operands (floats)...\n");
    test_le_float_swapped(dest_float, src1_float, src2_float, 1.5f, -1.5f);
    ref_ge_float(ref_float, src1_float, src2_float);
    /* Adjust reference for different values */
    for (int i = 0; i < N; i++) {
        ref_float[i] = ref_float[i] > 0.5f ? 1.5f : -1.5f;
    }
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: LE_EXPR swapped test\n");
        result = 1;
    }
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(dest_int, dest_float, src1_int, src2_int, src1_float, src2_float);
    /* Verify with separate reference computations */
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    for (int i = 0; i < N; i++) {
        ref_float[i] = src1_float[i] <= src2_float[i] ? 3.14f : -3.14f;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0 ||
        memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: mixed comparisons test\n");
        result = 1;
    }
    
    if (result == 0) {
        printf("All tests passed!\n");
    }
    
    return result;
}
