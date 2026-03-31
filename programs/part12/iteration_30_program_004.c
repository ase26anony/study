/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
static void escape(void *p) {
    __asm__ volatile ("" : : "r"(p) : "memory");
}

/* Reference scalar implementations for verification */
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

static int ref_lt_unsigned(const unsigned *src1, const unsigned *src2) {
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

static int test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
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

/* Additional test for signed integer LT/LE with swapped operands */
static void test_lt_int_swapped(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_int_swapped(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test for unsigned comparisons */
static void test_gt_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

int main() {
    int i;
    int errors = 0;
    
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    ALIGNED int dest_int[N], ref_int[N], mask_int[N], ref_mask[N];
    ALIGNED unsigned dest_uint[N], ref_uint[N];
    ALIGNED double dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (i = 0; i < N; i++) {
        src1_int[i] = (i % 3) * 100 - 150;
        src2_int[i] = (i % 5) * 50 - 100;
        
        src1_uint[i] = (i * 7) % 1000;
        src2_uint[i] = (i * 11) % 1000;
        
        src1_float[i] = (i % 7) * 0.5f - 1.5f;
        src2_float[i] = (i % 3) * 0.8f - 1.0f;
        
        src1_double[i] = (i % 9) * 0.3 - 1.2;
        src2_double[i] = (i % 4) * 0.7 - 0.8;
    }
    
    printf("Testing vector comparison transformations...\n");
    
    /* Test 1: GT_EXPR with integers */
    test_gt_int(dest_int, src1_int, src2_int, 999, -999);
    ref_gt_int(ref_int, src1_int, src2_int, 999, -999);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("ERROR: test_gt_int failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("ERROR: test_ge_float failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    int count1 = test_lt_unsigned(src1_uint, src2_uint);
    int count2 = ref_lt_unsigned(src1_uint, src2_uint);
    if (count1 != count2) {
        printf("ERROR: test_lt_unsigned failed: %d != %d\n", count1, count2);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("ERROR: test_le_double failed\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with integers (should trigger std::swap) */
    test_lt_int_swapped(dest_int, src1_int, src2_int, 777, -777);
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? 777 : -777;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("ERROR: test_lt_int_swapped failed\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with integers (should trigger std::swap) */
    test_le_int_swapped(dest_int, src1_int, src2_int, 888, -888);
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] <= src2_int[i] ? 888 : -888;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("ERROR: test_le_int_swapped failed\n");
        errors++;
    }
    
    /* Test 7: GT_EXPR with unsigned integers */
    test_gt_unsigned(dest_uint, src1_uint, src2_uint, 0xFFFFFFFF, 0);
    for (i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] > src2_uint[i] ? 0xFFFFFFFF : 0;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(unsigned)) != 0) {
        printf("ERROR: test_gt_unsigned failed\n");
        errors++;
    }
    
    /* Test 8: GE_EXPR with unsigned integers */
    test_ge_unsigned(dest_uint, src1_uint, src2_uint, 0xAAAAAAAA, 0x55555555);
    for (i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] >= src2_uint[i] ? 0xAAAAAAAA : 0x55555555;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(unsigned)) != 0) {
        printf("ERROR: test_ge_unsigned failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("%d test(s) failed\n", errors);
    }
    
    return errors;
}
