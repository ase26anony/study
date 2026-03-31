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
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
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

/* Test kernels - should trigger vectorization transformations */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
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
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_float_swapped(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] <= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Mixed pattern tests */
static void test_mixed_comparisons(int *results, const int *a, const int *b, const float *fa, const float *fb) {
    for (int i = 0; i < N; i++) {
        int r = 0;
        r |= (a[i] > b[i]) ? 0x1 : 0;
        r |= (a[i] >= b[i]) ? 0x2 : 0;
        r |= (a[i] < b[i]) ? 0x4 : 0;
        r |= (a[i] <= b[i]) ? 0x8 : 0;
        r |= (fa[i] > fb[i]) ? 0x10 : 0;
        r |= (fa[i] >= fb[i]) ? 0x20 : 0;
        r |= (fa[i] < fb[i]) ? 0x40 : 0;
        r |= (fa[i] <= fb[i]) ? 0x80 : 0;
        results[i] = r;
    }
    escape(results);
}

int main() {
    int i;
    int errors = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    /* Results arrays */
    ALIGNED int dest_int[N], dest_int_ref[N];
    ALIGNED int mask_int[N], mask_int_ref[N];
    ALIGNED double dest_double[N], dest_double_ref[N];
    ALIGNED int mixed_results[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : 
                     (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: sine wave pattern */
        src1_float[i] = sinf(i * 0.1f);
        src2_float[i] = cosf(i * 0.1f);
        
        /* Double arrays: exponential pattern */
        src1_double[i] = exp(i * 0.01);
        src2_double[i] = exp((i % 5) * 0.01);
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    ref_gt_int(dest_int_ref, src1_int, src2_int, 0xAAAA, 0x5555);
    if (memcmp(dest_int, dest_int_ref, sizeof(dest_int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(mask_int_ref, src1_float, src2_float);
    if (memcmp(mask_int, mask_int_ref, sizeof(mask_int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u != %u\n", count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 1.0, -1.0);
    ref_le_double(dest_double_ref, src1_double, src2_double, 1.0, -1.0);
    if (memcmp(dest_double, dest_double_ref, sizeof(dest_double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with integers (swapped operands case)...\n");
    test_lt_int_swapped(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    /* Reference is same as GT but with swapped comparison */
    for (i = 0; i < N; i++) {
        dest_int_ref[i] = src1_int[i] < src2_int[i] ? 0xAAAA : 0x5555;
    }
    if (memcmp(dest_int, dest_int_ref, sizeof(dest_int)) != 0) {
        printf("  ERROR: LT_EXPR integer swapped test failed\n");
        errors++;
    }
    
    printf("Testing LE_EXPR with floats (swapped operands case)...\n");
    test_le_float_swapped(mask_int, src1_float, src2_float);
    for (i = 0; i < N; i++) {
        mask_int_ref[i] = src1_float[i] <= src2_float[i] ? -1 : 0;
    }
    if (memcmp(mask_int, mask_int_ref, sizeof(mask_int)) != 0) {
        printf("  ERROR: LE_EXPR float swapped test failed\n");
        errors++;
    }
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int, src1_float, src2_float);
    /* No reference check for mixed - just ensure it executes */
    escape(mixed_results);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", errors);
        return 1;
    }
}
