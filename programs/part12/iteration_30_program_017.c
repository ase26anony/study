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
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
}

static int ref_lt_unsigned(const unsigned int *src1, const unsigned int *src2) {
    int count = 0;
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

/* Test kernels targeting specific uncovered transformations */

/* GT_EXPR transformation for integers */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR transformation for floats */
static void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* LT_EXPR transformation for unsigned integers (with potential swap) */
static int test_lt_unsigned(const unsigned int ALIGNED *src1, const unsigned int ALIGNED *src2) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* LE_EXPR transformation for doubles (with potential swap) */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2, 
                          double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for mixed patterns */

/* Mixed GT/GE comparisons in same loop */
static void test_mixed_comparisons(int ALIGNED *results, const int ALIGNED *a, const int ALIGNED *b, 
                                  const int ALIGNED *c) {
    for (int i = 0; i < N; i++) {
        results[i] = (a[i] > b[i]) ? (c[i] >= a[i] ? 1 : 2) : 3;
    }
    escape(results);
}

/* LT/LE with floating point arrays */
static void test_float_lt_le(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2,
                            const float ALIGNED *src3) {
    for (int i = 0; i < N; i++) {
        if (src1[i] < src2[i]) {
            dest[i] = src3[i];
        } else if (src1[i] <= src3[i]) {
            dest[i] = src2[i];
        } else {
            dest[i] = src1[i];
        }
    }
    escape(dest);
}

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    int *mask_int = aligned_alloc(32, N * sizeof(int));
    int *ref_mask = aligned_alloc(32, N * sizeof(int));
    
    unsigned int *src1_uint = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int *src2_uint = aligned_alloc(32, N * sizeof(unsigned int));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    float *dest_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        /* Float arrays: create both greater and lesser values */
        src1_float[i] = (float)i * 1.5f;
        src2_float[i] = (float)((i % 5) * 2.0f);
        
        /* Double arrays: similar pattern with different scale */
        src1_double[i] = (double)i * 0.75;
        src2_double[i] = (double)(i % 7) * 1.25;
    }
    
    printf("Testing GT_EXPR transformation with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR transformation with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR transformation with unsigned integers...\n");
    int vec_count = test_lt_unsigned(src1_uint, src2_uint);
    int ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: vec=%d, ref=%d\n", vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR transformation with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 1.0, 2.0);
    ref_le_double(ref_double, src1_double, src2_double, 1.0, 2.0);
    for (i = 0; i < N; i++) {
        if (dest_double[i] != ref_double[i]) {
            printf("  ERROR: LE_EXPR double test failed at index %d: %f != %f\n", 
                   i, dest_double[i], ref_double[i]);
            errors++;
            break;
        }
    }
    
    /* Additional mixed tests */
    printf("Testing mixed comparison patterns...\n");
    test_mixed_comparisons(dest_int, src1_int, src2_int, src1_int);  /* Using same array for simplicity */
    escape(dest_int);
    
    test_float_lt_le(dest_float, src1_float, src2_float, src1_float);
    escape(dest_float);
    
    /* Cleanup */
    free(src1_int);
    free(src2_int);
    free(dest_int);
    free(ref_int);
    free(mask_int);
    free(ref_mask);
    free(src1_uint);
    free(src2_uint);
    free(src1_float);
    free(src2_float);
    free(dest_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
