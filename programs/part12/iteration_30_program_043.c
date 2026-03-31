#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
static void escape(void *p) {
    __asm__ volatile ("" : : "r"(p) : "memory");
}

/* Reference scalar implementations for verification */
void ref_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

void ref_ge_float(float *dest, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

unsigned ref_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

void ref_le_double(double *dest, double *src1, double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers */
void test_gt_int(int ALIGNED *dest, int ALIGNED *src1, int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
void test_ge_float(float ALIGNED *dest, float ALIGNED *src1, float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
unsigned test_lt_unsigned(unsigned ALIGNED *src1, unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
void test_le_double(double ALIGNED *dest, double ALIGNED *src1, double ALIGNED *src2, 
                    double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
void test_lt_int(int ALIGNED *dest, int ALIGNED *src1, int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

void test_ge_int(int ALIGNED *dest, int ALIGNED *src1, int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed pattern test with alternating comparisons */
void test_mixed_comparisons(int ALIGNED *dest_int, float ALIGNED *dest_float,
                           int ALIGNED *src1_int, int ALIGNED *src2_int,
                           float ALIGNED *src1_float, float ALIGNED *src2_float) {
    for (int i = 0; i < N; i++) {
        /* GT with integers */
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        
        /* LE with floats (inverted pattern) */
        dest_float[i] = src1_float[i] <= src2_float[i] ? 2.5f : -2.5f;
    }
    escape(dest_int);
    escape(dest_float);
}

int verify_results(void *a, void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    
    unsigned *src1_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *src2_uint = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    float *dest_float = aligned_alloc(32, N * sizeof(float));
    float *ref_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer arrays: alternating pattern */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: include special values */
        src1_float[i] = (i % 5 == 0) ? (float)i : (float)(i * 1.5f);
        src2_float[i] = (i % 5 == 1) ? (float)(i + 0.5f) : (float)i;
        
        /* Double arrays: similar pattern */
        src1_double[i] = (i % 7 == 0) ? (double)i : (double)(i * 1.7);
        src2_double[i] = (i % 7 == 1) ? (double)(i + 0.3) : (double)i;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 999, -999);
    ref_gt_int(ref_int, src1_int, src2_int, 999, -999);
    if (!verify_results(dest_int, ref_int, N * sizeof(int))) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (!verify_results(dest_float, ref_float, N * sizeof(float))) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
               vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -3.14159);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -3.14159);
    if (!verify_results(dest_double, ref_double, N * sizeof(double))) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, -777);
    /* Reuse ref_gt_int with swapped arguments for LT verification */
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? 777 : -777;
    }
    if (!verify_results(dest_int, ref_int, N * sizeof(int))) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with signed integers...\n");
    test_ge_int(dest_int, src1_int, src2_int, 888, -888);
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] >= src2_int[i] ? 888 : -888;
    }
    if (!verify_results(dest_int, ref_int, N * sizeof(int))) {
        printf("  ERROR: GE_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing mixed comparisons...\n");
    int *dest_int2 = aligned_alloc(32, N * sizeof(int));
    float *dest_float2 = aligned_alloc(32, N * sizeof(float));
    test_mixed_comparisons(dest_int2, dest_float2, src1_int, src2_int, 
                          src1_float, src2_float);
    
    /* Verify mixed results */
    int mixed_errors = 0;
    for (i = 0; i < N; i++) {
        int expected_int = src1_int[i] > src2_int[i] ? 100 : -100;
        float expected_float = src1_float[i] <= src2_float[i] ? 2.5f : -2.5f;
        
        if (dest_int2[i] != expected_int) {
            mixed_errors++;
        }
        if (fabsf(dest_float2[i] - expected_float) > 0.0001f) {
            mixed_errors++;
        }
    }
    if (mixed_errors > 0) {
        printf("  ERROR: Mixed comparisons test failed with %d errors\n", mixed_errors);
        errors++;
    }
    
    /* Cleanup */
    free(src1_int);
    free(src2_int);
    free(dest_int);
    free(ref_int);
    free(src1_uint);
    free(src2_uint);
    free(src1_float);
    free(src2_float);
    free(dest_float);
    free(ref_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    free(dest_int2);
    free(dest_float2);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
