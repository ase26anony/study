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

/* Test kernels - these should trigger vectorization transformations */
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

/* Mixed pattern comparisons */
static void test_mixed_comparisons(int *results, const int *a, const int *b, const float *fa, const float *fb) {
    for (int i = 0; i < N; i++) {
        int r = 0;
        r |= (a[i] > b[i]) ? 0x1 : 0;
        r |= (a[i] >= b[i]) ? 0x2 : 0;
        r |= (fa[i] < fb[i]) ? 0x4 : 0;
        r |= (fa[i] <= fb[i]) ? 0x8 : 0;
        results[i] = r;
    }
    escape(results);
}

int main() {
    int ret = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    ALIGNED int dest_int[N], ref_int[N], mask_int[N], ref_mask[N];
    ALIGNED double dest_double[N], ref_double[N];
    ALIGNED int mixed_results[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* For integer tests: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : 
                     (i % 3 == 1) ? i - 1 : i;
        
        /* For unsigned tests: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* For float tests: use sine pattern for variety */
        src1_float[i] = sinf(i * 0.1f);
        src2_float[i] = cosf(i * 0.1f);
        
        /* For double tests: similar pattern */
        src1_double[i] = sin(i * 0.1);
        src2_double[i] = cos(i * 0.1);
    }
    
    printf("Testing GT_EXPR (integer)...\n");
    test_gt_int(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    ref_gt_int(ref_int, src1_int, src2_int, 0xAAAA, 0x5555);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing GE_EXPR (float)...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR (unsigned)...\n");
    unsigned count_vec = test_lt_unsigned(src1_uint, src2_uint);
    unsigned count_ref = ref_lt_unsigned(src1_uint, src2_uint);
    if (count_vec != count_ref) {
        printf("FAIL: LT_EXPR unsigned test: vec=%u, ref=%u\n", count_vec, count_ref);
        ret = 1;
    }
    
    printf("Testing LE_EXPR (double)...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    for (int i = 0; i < N; i++) {
        if (fabs(dest_double[i] - ref_double[i]) > 1e-10) {
            printf("FAIL: LE_EXPR double test at index %d\n", i);
            ret = 1;
            break;
        }
    }
    
    printf("Testing LT_EXPR with swapped operands (integer)...\n");
    test_lt_int_swapped(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    /* Reference is same since operation is symmetric for verification */
    ref_gt_int(ref_int, src2_int, src1_int, 0xAAAA, 0x5555); /* a < b == b > a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR swapped integer test\n");
        ret = 1;
    }
    
    printf("Testing LE_EXPR with swapped operands (float)...\n");
    test_le_float_swapped(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src2_float, src1_float); /* a <= b == b >= a */
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("FAIL: LE_EXPR swapped float test\n");
        ret = 1;
    }
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int, src1_float, src2_float);
    /* No reference check for mixed - just ensure it executes without error */
    escape(mixed_results);
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}
