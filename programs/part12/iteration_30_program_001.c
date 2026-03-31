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

/* Reference scalar implementations for verification */
void ref_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

void ref_ge_float(int *mask, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
}

unsigned ref_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += src1[i] < src2[i];
    }
    return count;
}

void ref_le_double(double *dest, double *src1, double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Vectorizable test kernels */
void test_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

void test_ge_float(int *mask, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

unsigned test_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += src1[i] < src2[i];
    }
    escape(&count);
    return count;
}

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

void test_le_int(int *mask, int *src1, int *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] <= src2[i];
    }
    escape(mask);
}

/* Mixed pattern data generation */
void init_data(int *int1, int *int2, unsigned *uint1, unsigned *uint2,
               float *f1, float *f2, double *d1, double *d2) {
    for (int i = 0; i < N; i++) {
        /* Create mixed comparison patterns */
        int1[i] = (i % 3 == 0) ? i + 10 : i - 5;
        int2[i] = i;
        
        uint1[i] = (i % 4 == 0) ? i * 2 : i;
        uint2[i] = i + 3;
        
        f1[i] = (i % 5 == 0) ? i * 1.5f : i * 0.8f;
        f2[i] = i * 1.0f;
        
        d1[i] = (i % 6 == 0) ? i * 2.5 : i * 0.7;
        d2[i] = i * 1.0;
    }
}

int verify_results(void *a, void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

int main(void) {
    int errors = 0;
    
    /* Aligned allocations for vectorization */
    ALIGNED int int_src1[N], int_src2[N];
    ALIGNED unsigned uint_src1[N], uint_src2[N];
    ALIGNED float float_src1[N], float_src2[N];
    ALIGNED double double_src1[N], double_src2[N];
    
    /* Destination arrays */
    ALIGNED int int_dest[N], int_dest_ref[N];
    ALIGNED int int_mask[N], int_mask_ref[N];
    ALIGNED int float_mask[N], float_mask_ref[N];
    ALIGNED double double_dest[N], double_dest_ref[N];
    
    /* Initialize test data */
    init_data(int_src1, int_src2, uint_src1, uint_src2,
              float_src1, float_src2, double_src1, double_src2);
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(int_dest, int_src1, int_src2, 100, 200);
    ref_gt_int(int_dest_ref, int_src1, int_src2, 100, 200);
    if (!verify_results(int_dest, int_dest_ref, N * sizeof(int))) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(float_mask, float_src1, float_src2);
    ref_ge_float(float_mask_ref, float_src1, float_src2);
    if (!verify_results(float_mask, float_mask_ref, N * sizeof(int))) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(uint_src1, uint_src2);
    unsigned ref_count = ref_lt_unsigned(uint_src1, uint_src2);
    if (count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u != %u\n", count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(double_dest, double_src1, double_src2, 1.0, 0.0);
    ref_le_double(double_dest_ref, double_src1, double_src2, 1.0, 0.0);
    if (!verify_results(double_dest, double_dest_ref, N * sizeof(double))) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(int_dest, int_src1, int_src2, 300, 400);
    /* Reference implementation for LT */
    for (int i = 0; i < N; i++) {
        int_dest_ref[i] = int_src1[i] < int_src2[i] ? 300 : 400;
    }
    if (!verify_results(int_dest, int_dest_ref, N * sizeof(int))) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(int_mask, int_src1, int_src2);
    /* Reference implementation for LE */
    for (int i = 0; i < N; i++) {
        int_mask_ref[i] = int_src1[i] <= int_src2[i];
    }
    if (!verify_results(int_mask, int_mask_ref, N * sizeof(int))) {
        printf("  ERROR: LE_EXPR signed integer test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
