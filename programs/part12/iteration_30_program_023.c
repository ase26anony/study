#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

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

static void ref_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
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

/* Test kernels targeting specific uncovered patterns */

/* GT_EXPR with integers */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    int val1 = 0xAAAA, val2 = 0x5555;
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR with floats */
static void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

/* LT_EXPR with unsigned integers (reduction pattern) */
static unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* LE_EXPR with doubles */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2) {
    double val1 = 3.14159, val2 = 2.71828;
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
static void test_lt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    int val1 = 0x1234, val2 = 0x5678;
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_int(int ALIGNED *mask, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] <= src2[i];
    }
    escape(mask);
}

/* Mixed pattern to trigger different transformations */
static void test_mixed_comparisons(float ALIGNED *dest_f, int ALIGNED *dest_i,
                                   const float ALIGNED *src1_f, const float ALIGNED *src2_f,
                                   const int ALIGNED *src1_i, const int ALIGNED *src2_i) {
    for (int i = 0; i < N; i++) {
        /* GT with float */
        dest_f[i] = src1_f[i] > src2_f[i] ? 1.0f : 0.0f;
        /* LE with int */
        dest_i[i] = src1_i[i] <= src2_i[i] ? -1 : 1;
    }
    escape(dest_f);
    escape(dest_i);
}

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_i = aligned_alloc(32, N * sizeof(int));
    int *src2_i = aligned_alloc(32, N * sizeof(int));
    int *dest_i = aligned_alloc(32, N * sizeof(int));
    int *ref_i = aligned_alloc(32, N * sizeof(int));
    int *mask_i = aligned_alloc(32, N * sizeof(int));
    int *ref_mask_i = aligned_alloc(32, N * sizeof(int));
    
    unsigned *src1_u = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *src2_u = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_f = aligned_alloc(32, N * sizeof(float));
    float *src2_f = aligned_alloc(32, N * sizeof(float));
    
    double *src1_d = aligned_alloc(32, N * sizeof(double));
    double *src2_d = aligned_alloc(32, N * sizeof(double));
    double *dest_d = aligned_alloc(32, N * sizeof(double));
    double *ref_d = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_i[i] = (i % 3) * 100 - 50;
        src2_i[i] = (i % 4) * 75 - 25;
        
        /* Unsigned arrays: different pattern */
        src1_u[i] = (i * 37) % 1000;
        src2_u[i] = (i * 53) % 1000;
        
        /* Float arrays: create both true and false comparisons */
        src1_f[i] = (i % 5) * 0.5f - 1.0f;
        src2_f[i] = (i % 3) * 0.7f - 0.5f;
        
        /* Double arrays: similar pattern */
        src1_d[i] = (i % 7) * 0.3 - 1.0;
        src2_d[i] = (i % 6) * 0.4 - 0.8;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_i, src1_i, src2_i);
    ref_gt_int(ref_i, src1_i, src2_i, 0xAAAA, 0x5555);
    if (memcmp(dest_i, ref_i, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_i, src1_f, src2_f);
    ref_ge_float(ref_mask_i, src1_f, src2_f);
    if (memcmp(mask_i, ref_mask_i, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_u, src2_u);
    unsigned ref_count = ref_lt_unsigned(src1_u, src2_u);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: vec=%u, ref=%u\n", vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_d, src1_d, src2_d);
    ref_le_double(ref_d, src1_d, src2_d, 3.14159, 2.71828);
    if (memcmp(dest_d, ref_d, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_i, src1_i, src2_i);
    ref_gt_int(ref_i, src2_i, src1_i, 0x1234, 0x5678); /* Note: swapped for LT */
    if (memcmp(dest_i, ref_i, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed test failed\n");
        errors++;
    }
    
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(mask_i, src1_i, src2_i);
    ref_ge_float(ref_mask_i, (float*)src2_i, (float*)src1_i); /* Note: swapped for LE */
    if (memcmp(mask_i, ref_mask_i, N * sizeof(int)) != 0) {
        printf("  ERROR: LE_EXPR signed test failed\n");
        errors++;
    }
    
    printf("Testing mixed comparisons...\n");
    float *dest_f = aligned_alloc(32, N * sizeof(float));
    float *ref_f = aligned_alloc(32, N * sizeof(float));
    int *ref_mixed_i = aligned_alloc(32, N * sizeof(int));
    
    test_mixed_comparisons(dest_f, dest_i, src1_f, src2_f, src1_i, src2_i);
    
    /* Reference for mixed */
    for (i = 0; i < N; i++) {
        ref_f[i] = src1_f[i] > src2_f[i] ? 1.0f : 0.0f;
        ref_mixed_i[i] = src1_i[i] <= src2_i[i] ? -1 : 1;
    }
    
    if (memcmp(dest_f, ref_f, N * sizeof(float)) != 0) {
        printf("  ERROR: Mixed float comparison failed\n");
        errors++;
    }
    if (memcmp(dest_i, ref_mixed_i, N * sizeof(int)) != 0) {
        printf("  ERROR: Mixed integer comparison failed\n");
        errors++;
    }
    
    /* Cleanup */
    free(src1_i); free(src2_i); free(dest_i); free(ref_i);
    free(mask_i); free(ref_mask_i);
    free(src1_u); free(src2_u);
    free(src1_f); free(src2_f);
    free(src1_d); free(src2_d); free(dest_d); free(ref_d);
    free(dest_f); free(ref_f); free(ref_mixed_i);
    
    if (errors == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", errors);
        return 1;
    }
}
