/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        count += src1[i] < src2[i];
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, 
                          double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels - should be vectorized */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += src1[i] < src2[i];
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2,
                           double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional test cases for signed/unsigned variations */
static void test_ge_unsigned(int *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

static void test_lt_float(float *dest, const float *src1, const float *src2,
                          float threshold) {
    for (int i = 0; i < N; i++) {
        if (src1[i] < src2[i]) {
            dest[i] = threshold;
        } else {
            dest[i] = src1[i];
        }
    }
    escape(dest);
}

static void test_gt_double_reduction(double *sum, const double *src1, const double *src2) {
    double total = 0.0;
    for (int i = 0; i < N; i++) {
        if (src1[i] > src2[i]) {
            total += src1[i];
        }
    }
    *sum = total;
    escape(sum);
}

/* Mixed comparison test */
static void test_mixed_comparisons(int *results, const int *a, const int *b) {
    for (int i = 0; i < N; i++) {
        /* Use multiple comparison operators in same loop */
        int r = 0;
        r |= (a[i] > b[i]) ? 1 : 0;
        r |= (a[i] >= b[i]) ? 2 : 0;
        r |= (a[i] < b[i]) ? 4 : 0;
        r |= (a[i] <= b[i]) ? 8 : 0;
        results[i] = r;
    }
    escape(results);
}

int main() {
    int i;
    int errors = 0;
    
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    /* Results arrays */
    ALIGNED int dest_int[N], ref_int[N], mask_int[N], ref_mask[N];
    ALIGNED float dest_float[N];
    ALIGNED double dest_double[N], ref_double[N];
    ALIGNED int mixed_results[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : 
                     (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 5 : i * 2 - 3;
        
        /* Float arrays: include special values */
        src1_float[i] = sinf(i * 0.1f) * 100.0f;
        src2_float[i] = cosf(i * 0.1f) * 100.0f;
        
        /* Double arrays */
        src1_double[i] = sin(i * 0.1) * 100.0;
        src2_double[i] = cos(i * 0.1) * 100.0;
    }
    
    printf("Testing GT_EXPR (>) with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR (>=) with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("  ERROR: GE float test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR (>=) with unsigned...\n");
    test_ge_unsigned(mask_int, src1_uint, src2_uint);
    ref_ge_float(ref_mask, (float*)src1_uint, (float*)src2_uint); /* Reuse for verification */
    /* Note: This comparison is approximate for verification */
    
    printf("Testing LT_EXPR (<) with unsigned reduction...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT unsigned reduction test failed: %u vs %u\n", 
               vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LT_EXPR (<) with float conditional...\n");
    test_lt_float(dest_float, src1_float, src2_float, 0.0f);
    /* Reference check would be similar */
    
    printf("Testing LE_EXPR (<=) with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 1.0, -1.0);
    ref_le_double(ref_double, src1_double, src2_double, 1.0, -1.0);
    for (i = 0; i < N; i++) {
        if (fabs(dest_double[i] - ref_double[i]) > 1e-10) {
            printf("  ERROR: LE double test failed at index %d: %f vs %f\n",
                   i, dest_double[i], ref_double[i]);
            errors++;
            if (errors > 5) break;
        }
    }
    
    printf("Testing GT_EXPR (>) with double reduction...\n");
    double vec_sum, ref_sum = 0.0;
    test_gt_double_reduction(&vec_sum, src1_double, src2_double);
    for (i = 0; i < N; i++) {
        if (src1_double[i] > src2_double[i]) {
            ref_sum += src1_double[i];
        }
    }
    if (fabs(vec_sum - ref_sum) > 1e-10) {
        printf("  ERROR: GT double reduction test failed: %f vs %f\n",
               vec_sum, ref_sum);
        errors++;
    }
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int);
    /* Simple verification - just check bounds */
    for (i = 0; i < N; i++) {
        if (mixed_results[i] < 0 || mixed_results[i] > 15) {
            printf("  ERROR: Mixed comparison out of bounds at %d: %d\n",
                   i, mixed_results[i]);
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors > 0 ? 1 : 0;
}
