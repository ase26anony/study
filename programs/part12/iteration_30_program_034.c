/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization removal */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Test kernels for different comparison types and data types */

/* GT (greater than) comparisons */
void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

void test_gt_float(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

/* GE (greater than or equal) comparisons */
void test_ge_unsigned(unsigned *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
}

void test_ge_double(double *dest, const double *src1, const double *src2, double val) {
    for (int i = 0; i < N; i++) {
        if (src1[i] >= src2[i]) {
            dest[i] = val;
        } else {
            dest[i] = 0.0;
        }
    }
}

/* LT (less than) comparisons */
unsigned test_lt_int_reduction(const int *src1, const int *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    return count;
}

void test_lt_float_mask(float *dest, const float *src1, const float *src2, float threshold) {
    for (int i = 0; i < N; i++) {
        dest[i] = (src1[i] < src2[i]) ? threshold : -threshold;
    }
}

/* LE (less than or equal) comparisons */
void test_le_int(int *dest, const int *src1, const int *src2, int val) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val : -val;
    }
}

void test_le_unsigned_reduction(unsigned *dest, const unsigned *src1, const unsigned *src2) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
    *dest = sum;
}

/* Mixed comparison types in same loop */
void test_mixed_comparisons(int *dest, const int *src1, const int *src2, const float *fsrc1, const float *fsrc2) {
    for (int i = 0; i < N; i++) {
        /* Use different comparison operators */
        dest[i] = (src1[i] > src2[i]) ? 1 : 0;
        dest[i] += (src1[i] >= src2[i]) ? 2 : 0;
        dest[i] += (src1[i] < src2[i]) ? 4 : 0;
        dest[i] += (src1[i] <= src2[i]) ? 8 : 0;
        /* Also use float comparisons to trigger floating-point paths */
        if (fsrc1[i] > fsrc2[i]) dest[i] += 16;
        if (fsrc1[i] >= fsrc2[i]) dest[i] += 32;
        if (fsrc1[i] < fsrc2[i]) dest[i] += 64;
        if (fsrc1[i] <= fsrc2[i]) dest[i] += 128;
    }
}

/* Reference implementations for verification */
void ref_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

unsigned ref_lt_int_reduction(const int *src1, const int *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    return count;
}

/* Verification helper */
int verify_int(const int *a, const int *b, const char *test_name) {
    for (int i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            printf("FAIL %s at index %d: %d != %d\n", test_name, i, a[i], b[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N], mask_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : 
                     (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: sine-like pattern */
        src1_float[i] = (i % 8) * 0.125f;
        src2_float[i] = ((i + 2) % 8) * 0.125f;
        
        /* Double arrays: similar pattern */
        src1_double[i] = (i % 16) * 0.0625;
        src2_double[i] = ((i + 4) % 16) * 0.0625;
    }
    
    int all_pass = 1;
    
    /* Test 1: GT comparisons with integers */
    memset(dest_int, 0, sizeof(dest_int));
    memset(ref_int, 0, sizeof(ref_int));
    
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    all_pass &= verify_int(dest_int, ref_int, "test_gt_int");
    
    /* Test 2: GT comparisons with floats */
    test_gt_float(dest_float, src1_float, src2_float, 1.0f, -1.0f);
    escape(dest_float);
    
    /* Test 3: GE comparisons with unsigned */
    test_ge_unsigned(mask_uint, src1_uint, src2_uint);
    escape(mask_uint);
    
    /* Test 4: GE comparisons with doubles */
    test_ge_double(dest_double, src1_double, src2_double, 3.14159);
    escape(dest_double);
    
    /* Test 5: LT reduction with integers */
    unsigned count1 = test_lt_int_reduction(src1_int, src2_int);
    unsigned count2 = ref_lt_int_reduction(src1_int, src2_int);
    if (count1 != count2) {
        printf("FAIL test_lt_int_reduction: %u != %u\n", count1, count2);
        all_pass = 0;
    }
    
    /* Test 6: LT mask with floats */
    test_lt_float_mask(dest_float, src1_float, src2_float, 2.5f);
    escape(dest_float);
    
    /* Test 7: LE comparisons with integers */
    test_le_int(dest_int, src1_int, src2_int, 77);
    escape(dest_int);
    
    /* Test 8: LE reduction with unsigned */
    unsigned sum;
    test_le_unsigned_reduction(&sum, src1_uint, src2_uint);
    escape(&sum);
    
    /* Test 9: Mixed comparisons */
    test_mixed_comparisons(dest_int, src1_int, src2_int, src1_float, src2_float);
    escape(dest_int);
    
    /* Additional tests with different data patterns */
    
    /* Test with all true comparisons */
    for (int i = 0; i < N; i++) {
        src1_int[i] = i + 10;
        src2_int[i] = i;
    }
    test_gt_int(dest_int, src1_int, src2_int, 1, 0);
    escape(dest_int);
    
    /* Test with all false comparisons */
    for (int i = 0; i < N; i++) {
        src1_int[i] = i;
        src2_int[i] = i + 10;
    }
    test_gt_int(dest_int, src1_int, src2_int, 1, 0);
    escape(dest_int);
    
    /* Test with alternating true/false */
    for (int i = 0; i < N; i++) {
        src1_int[i] = (i % 2 == 0) ? i + 5 : i - 5;
        src2_int[i] = i;
    }
    test_le_int(dest_int, src1_int, src2_int, 99);
    escape(dest_int);
    
    if (all_pass) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
