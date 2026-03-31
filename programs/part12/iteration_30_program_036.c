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

/* Vectorizable test functions targeting specific comparison operations */

/* Test GT_EXPR with integer arrays */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test GE_EXPR with float arrays */
static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* Test LT_EXPR with unsigned arrays (reduction pattern) */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Test LE_EXPR with double arrays */
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

static void test_ge_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed pattern test with multiple comparison types */
static void test_mixed_patterns(int *results, const int *a, const int *b, const float *fa, const float *fb) {
    for (int i = 0; i < N; i++) {
        int cond1 = a[i] > b[i];      // GT_EXPR
        int cond2 = a[i] >= b[i];     // GE_EXPR
        int cond3 = a[i] < b[i];      // LT_EXPR
        int cond4 = a[i] <= b[i];     // LE_EXPR
        float fcond1 = fa[i] > fb[i] ? 1.0f : 0.0f;
        float fcond2 = fa[i] <= fb[i] ? 1.0f : 0.0f;
        
        results[i] = cond1 + cond2 + cond3 + cond4 + (int)fcond1 + (int)fcond2;
    }
    escape(results);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N], dest_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: create crossing points */
        src1_float[i] = (float)i * 0.5f;
        src2_float[i] = (float)(i % 5) * 0.7f;
        
        /* Double arrays: similar pattern with doubles */
        src1_double[i] = (double)i * 0.3;
        src2_double[i] = (double)(i % 7) * 0.4;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed!\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: GE_EXPR float test failed!\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed! %u != %u\n", vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed!\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 300, 400);
    ref_gt_int(ref_int, src2_int, src1_int, 300, 400); // Inverse for verification
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed integer test failed!\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with unsigned...\n");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint, 500, 600);
    /* Reference implementation for unsigned GE */
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_uint[i] >= src2_uint[i] ? 500 : 600;
    }
    if (memcmp(dest_uint, ref_int, N * sizeof(unsigned)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed!\n");
        errors++;
    }
    
    /* Test mixed patterns */
    printf("Testing mixed comparison patterns...\n");
    ALIGNED int mixed_results[N];
    test_mixed_patterns(mixed_results, src1_int, src2_int, src1_float, src2_float);
    escape(mixed_results);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
