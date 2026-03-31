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
static void ref_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void ref_ge_float(float *dest, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static unsigned ref_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    return count;
}

static void ref_le_double(double *dest, double *src1, double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers */
void test_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
void test_ge_float(float *dest, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
unsigned test_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
void test_le_double(double *dest, double *src1, double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
void test_lt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

void test_le_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed pattern to ensure all transformations are triggered */
void test_mixed_comparisons(int *dest_int, float *dest_float, 
                           int *src1_int, int *src2_int,
                           float *src1_float, float *src2_float) {
    for (int i = 0; i < N; i++) {
        /* GT with integers */
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        
        /* LE with floats */
        dest_float[i] = src1_float[i] <= src2_float[i] ? 2.0f : -2.0f;
    }
    escape(dest_int);
    escape(dest_float);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = (i % 4 == 0) ? i + 10 : 
                     (i % 4 == 1) ? i - 5 : 
                     (i % 4 == 2) ? i : i + 20;
        src2_int[i] = (i % 4 == 0) ? i : 
                     (i % 4 == 1) ? i + 5 : 
                     (i % 4 == 2) ? i + 1 : i - 10;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = (i * 3) % 256;
        src2_uint[i] = (i * 5) % 256;
        
        /* Float arrays: include special values */
        src1_float[i] = (i % 8 == 0) ? INFINITY :
                       (i % 8 == 1) ? -INFINITY :
                       (i % 8 == 2) ? NAN :
                       (i % 8 == 3) ? 0.0f :
                       (i % 8 == 4) ? -0.0f :
                       (float)(i * 2);
        src2_float[i] = (i % 8 == 0) ? (float)i :
                       (i % 8 == 1) ? (float)(i + 100) :
                       (i % 8 == 2) ? NAN :
                       (i % 8 == 3) ? 1.0f :
                       (i % 8 == 4) ? 1.0f :
                       (float)(i * 3);
        
        /* Double arrays: similar pattern */
        src1_double[i] = (i % 6 == 0) ? (double)(i - 50) :
                        (i % 6 == 1) ? (double)(i + 50) :
                        (i % 6 == 2) ? 0.0 :
                        (i % 6 == 3) ? -0.0 :
                        (double)(i * 1.5);
        src2_double[i] = (i % 6 == 0) ? (double)(i + 50) :
                        (i % 6 == 1) ? (double)(i - 50) :
                        (i % 6 == 2) ? 1.0 :
                        (i % 6 == 3) ? 1.0 :
                        (double)(i * 2.5);
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 777, 333);
    ref_gt_int(ref_int, src1_int, src2_int, 777, 333);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    for (int i = 0; i < N; i++) {
        /* Handle NaN comparisons specially */
        if (isnan(src1_float[i]) || isnan(src2_float[i])) {
            if (!isnan(dest_float[i]) || !isnan(ref_float[i])) {
                printf("  ERROR: GE_EXPR float test failed at index %d (NaN handling)\n", i);
                errors++;
                break;
            }
        } else if (fabsf(dest_float[i] - ref_float[i]) > 1e-6) {
            printf("  ERROR: GE_EXPR float test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: vec=%u, ref=%u\n", 
               vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 999.0, 111.0);
    ref_le_double(ref_double, src1_double, src2_double, 999.0, 111.0);
    for (int i = 0; i < N; i++) {
        if (isnan(src1_double[i]) || isnan(src2_double[i])) {
            if (!isnan(dest_double[i]) || !isnan(ref_double[i])) {
                printf("  ERROR: LE_EXPR double test failed at index %d (NaN handling)\n", i);
                errors++;
                break;
            }
        } else if (fabs(dest_double[i] - ref_double[i]) > 1e-12) {
            printf("  ERROR: LE_EXPR double test failed at index %d: vec=%f, ref=%f\n", 
                   i, dest_double[i], ref_double[i]);
            errors++;
            break;
        }
    }
    
    /* Additional signed integer tests */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 888, 444);
    ref_gt_int(ref_int, src2_int, src1_int, 888, 444); /* LT(a,b) == GT(b,a) */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(dest_int, src1_int, src2_int, 666, 222);
    ref_ge_float(ref_float, src2_float, src1_float); /* Just to use ref_ge_float */
    /* Note: LE(a,b) == GE(b,a) for integers */
    int temp[N];
    for (int i = 0; i < N; i++) {
        temp[i] = src2_int[i] >= src1_int[i] ? 666 : 222;
    }
    if (memcmp(dest_int, temp, N * sizeof(int)) != 0) {
        printf("  ERROR: LE_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing mixed comparisons...\n");
    int dest_mixed_int[N];
    float dest_mixed_float[N];
    test_mixed_comparisons(dest_mixed_int, dest_mixed_float, 
                          src1_int, src2_int, src1_float, src2_float);
    
    /* Verify mixed results */
    for (int i = 0; i < N; i++) {
        int expected_int = src1_int[i] > src2_int[i] ? 100 : -100;
        float expected_float = src1_float[i] <= src2_float[i] ? 2.0f : -2.0f;
        
        if (dest_mixed_int[i] != expected_int) {
            printf("  ERROR: mixed test integer failed at index %d\n", i);
            errors++;
            break;
        }
        
        if (isnan(src1_float[i]) || isnan(src2_float[i])) {
            if (!isnan(dest_mixed_float[i])) {
                printf("  ERROR: mixed test float NaN handling failed at index %d\n", i);
                errors++;
                break;
            }
        } else if (fabsf(dest_mixed_float[i] - expected_float) > 1e-6) {
            printf("  ERROR: mixed test float failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed\n", errors);
    }
    
    return errors;
}
