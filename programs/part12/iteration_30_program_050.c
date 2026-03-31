#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Reference implementations for verification */
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
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels - these should trigger vectorization transformations */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation */
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
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

/* Test for mixed patterns to ensure all paths are exercised */
static void test_mixed_comparisons(int *dest_int, float *dest_float, 
                                   const int *src1_int, const int *src2_int,
                                   const float *src1_float, const float *src2_float) {
    for (int i = 0; i < N; i++) {
        /* Mix GT and LE in same loop */
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        dest_float[i] = src1_float[i] <= src2_float[i] ? 2.0f : -2.0f;
    }
    escape(dest_int);
    escape(dest_float);
}

int main() {
    int result = 0;
    
    /* Aligned arrays for vector loads */
    ALIGN int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGN unsigned src1_uint[N], src2_uint[N];
    ALIGN float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGN double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* For integers: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* For unsigned: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* For floats: create crossing points */
        src1_float[i] = (float)(i - N/2) * 0.5f;
        src2_float[i] = (float)(i % 10) * 0.3f;
        
        /* For doubles: similar pattern but with doubles */
        src1_double[i] = (double)(i - N/2) * 0.25;
        src2_double[i] = (double)(i % 8) * 0.4;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 1, 0);
    ref_gt_int(ref_int, src1_int, src2_int, 1, 0);
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
        printf("FAIL: LT_EXPR unsigned test: vec=%u, ref=%u\n", vec_count, ref_count);
        result = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14, -3.14);
    ref_le_double(ref_double, src1_double, src2_double, 3.14, -3.14);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        result = 1;
    }
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 5, -5);
    ref_gt_int(ref_int, src2_int, src1_int, 5, -5); /* a < b is same as b > a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR signed integer test\n");
        result = 1;
    }
    
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(dest_int, src1_int, src2_int, 7, -7);
    ref_ge_int(ref_int, src2_int, src1_int, 7, -7); /* a <= b is same as b >= a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LE_EXPR signed integer test\n");
        result = 1;
    }
    
    printf("Testing mixed comparisons...\n");
    ALIGN int dest_mixed_int[N];
    ALIGN float dest_mixed_float[N];
    test_mixed_comparisons(dest_mixed_int, dest_mixed_float, 
                          src1_int, src2_int, src1_float, src2_float);
    
    /* Verify mixed results */
    int mixed_ok = 1;
    for (int i = 0; i < N; i++) {
        int expected_int = src1_int[i] > src2_int[i] ? 100 : -100;
        float expected_float = src1_float[i] <= src2_float[i] ? 2.0f : -2.0f;
        
        if (dest_mixed_int[i] != expected_int || 
            dest_mixed_float[i] != expected_float) {
            mixed_ok = 0;
            break;
        }
    }
    
    if (!mixed_ok) {
        printf("FAIL: Mixed comparisons test\n");
        result = 1;
    }
    
    if (result == 0) {
        printf("All tests passed!\n");
    }
    
    return result;
}

/* Helper reference function for LE -> GE transformation */
static void ref_ge_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
}
