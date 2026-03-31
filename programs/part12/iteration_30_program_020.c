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

static unsigned ref_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += src1[i] < src2[i] ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += src1[i] < src2[i] ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional mixed tests to ensure all paths are covered */

/* Mixed GT/GE comparisons in same loop */
static void test_mixed_comparisons(int *results, const int *a, const int *b, const float *fa, const float *fb) {
    for (int i = 0; i < N; i++) {
        results[i] = (a[i] > b[i]) ? 1 : 0;
        results[i] |= (fa[i] >= fb[i]) ? 2 : 0;
    }
    escape(results);
}

/* LT/LE with different data types */
static void test_lt_le_mixed(float *fresults, const float *fa, const float *fb, 
                             double *dresults, const double *da, const double *db) {
    for (int i = 0; i < N; i++) {
        fresults[i] = fa[i] < fb[i] ? 1.0f : 0.0f;
        dresults[i] = da[i] <= db[i] ? 1.0 : 0.0;
    }
    escape(fresults);
    escape(dresults);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED int mask_int[N], ref_mask[N];
    ALIGNED int mixed_results[N];
    ALIGNED float fresults[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns: alternating greater/lesser */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 :  /* src1 < src2 */
                      (i % 3 == 1) ? i - 1 :  /* src1 > src2 */
                                     i;       /* src1 == src2 */
        
        /* Unsigned patterns */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float patterns with some NaN values to test all comparison paths */
        src1_float[i] = (i % 5 == 0) ? NAN : (float)(i * 1.5f);
        src2_float[i] = (i % 7 == 0) ? NAN : (float)(i * 1.3f);
        
        /* Double patterns */
        src1_double[i] = (i % 3 == 0) ? INFINITY : (double)(i * 1.7);
        src2_double[i] = (i % 4 == 0) ? -INFINITY : (double)(i * 1.9);
    }
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 0xAAAAAAAA, 0x55555555);
    ref_gt_int(ref_int, src1_int, src2_int, 0xAAAAAAAA, 0x55555555);
    
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers */
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: vec=%u, ref=%u\n", vec_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    
    int double_errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(dest_double[i] - ref_double[i]) > 1e-10) {
            double_errors++;
        }
    }
    if (double_errors > 0) {
        printf("  ERROR: LE_EXPR double test failed (%d mismatches)\n", double_errors);
        errors++;
    }
    
    /* Additional mixed tests */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int, src1_float, src2_float);
    test_lt_le_mixed(fresults, src1_float, src2_float, dest_double, src1_double, src2_double);
    
    /* Verify at least some results are non-zero to ensure comparisons executed */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += mixed_results[i];
    }
    if (sum == 0) {
        printf("  WARNING: Mixed comparisons may not have executed properly\n");
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
