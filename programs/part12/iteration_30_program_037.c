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
void scalar_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

void scalar_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

unsigned scalar_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

void scalar_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Vectorized test functions targeting specific uncovered patterns */

/* Test GT_EXPR with integers */
void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test GE_EXPR with floats */
void test_ge_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* Test LT_EXPR with unsigned integers (reduction pattern) */
unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Test LE_EXPR with doubles */
void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
void test_lt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

void test_le_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed pattern test to trigger multiple transformations */
void test_mixed_patterns(float ALIGNED *f_dest, int ALIGNED *i_dest,
                         const float ALIGNED *f_src1, const float ALIGNED *f_src2,
                         const int ALIGNED *i_src1, const int ALIGNED *i_src2) {
    for (int i = 0; i < N; i++) {
        /* Mix GT and LE in same loop */
        f_dest[i] = f_src1[i] > f_src2[i] ? f_src1[i] : f_src2[i];
        i_dest[i] = i_src1[i] <= i_src2[i] ? i_src1[i] : i_src2[i];
    }
    escape(f_dest);
    escape(i_dest);
}

/* Initialize arrays with pattern that creates mixed comparison results */
void init_arrays(int ALIGNED *i1, int ALIGNED *i2,
                 unsigned ALIGNED *u1, unsigned ALIGNED *u2,
                 float ALIGNED *f1, float ALIGNED *f2,
                 double ALIGNED *d1, double ALIGNED *d2) {
    for (int i = 0; i < N; i++) {
        /* Create alternating pattern for mixed comparison results */
        i1[i] = (i % 3 == 0) ? i * 2 : i;
        i2[i] = (i % 2 == 0) ? i + 10 : i * 3;
        
        u1[i] = (i % 4 == 0) ? i * 5 : i + 100;
        u2[i] = (i % 3 == 0) ? i + 200 : i * 2;
        
        f1[i] = (i % 5 == 0) ? i * 1.5f : i * 0.5f;
        f2[i] = (i % 4 == 0) ? i * 2.0f : i * 0.8f;
        
        d1[i] = (i % 6 == 0) ? i * 3.14 : i * 1.414;
        d2[i] = (i % 5 == 0) ? i * 2.718 : i * 1.732;
    }
}

int verify_results(const void *vec, const void *scalar, size_t size) {
    return memcmp(vec, scalar, size) == 0;
}

int main() {
    /* Aligned arrays for vectorization */
    int ALIGNED i_src1[N], i_src2[N], i_dest_vec[N], i_dest_scalar[N];
    unsigned ALIGNED u_src1[N], u_src2[N];
    float ALIGNED f_src1[N], f_src2[N], f_dest_vec[N], f_dest_scalar[N];
    double ALIGNED d_src1[N], d_src2[N], d_dest_vec[N], d_dest_scalar[N];
    
    /* Additional arrays for mixed pattern test */
    float ALIGNED f_mixed_vec[N], f_mixed_scalar[N];
    int ALIGNED i_mixed_vec[N], i_mixed_scalar[N];
    
    int errors = 0;
    
    /* Initialize with pattern */
    init_arrays(i_src1, i_src2, u_src1, u_src2, f_src1, f_src2, d_src1, d_src2);
    
    printf("Testing vector comparison transformations...\n");
    
    /* Test 1: GT_EXPR with integers */
    test_gt_int(i_dest_vec, i_src1, i_src2, 999, -999);
    scalar_gt_int(i_dest_scalar, i_src1, i_src2, 999, -999);
    if (!verify_results(i_dest_vec, i_dest_scalar, N * sizeof(int))) {
        printf("FAIL: GT_EXPR integer test\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    test_ge_float(f_dest_vec, f_src1, f_src2);
    scalar_ge_float(f_dest_scalar, f_src1, f_src2);
    if (!verify_results(f_dest_vec, f_dest_scalar, N * sizeof(float))) {
        printf("FAIL: GE_EXPR float test\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    unsigned vec_count = test_lt_unsigned(u_src1, u_src2);
    unsigned scalar_count = scalar_lt_unsigned(u_src1, u_src2);
    if (vec_count != scalar_count) {
        printf("FAIL: LT_EXPR unsigned reduction test (vec=%u, scalar=%u)\n", 
               vec_count, scalar_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    test_le_double(d_dest_vec, d_src1, d_src2, 3.14159, 2.71828);
    scalar_le_double(d_dest_scalar, d_src1, d_src2, 3.14159, 2.71828);
    if (!verify_results(d_dest_vec, d_dest_scalar, N * sizeof(double))) {
        printf("FAIL: LE_EXPR double test\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with signed integers */
    test_lt_int(i_dest_vec, i_src1, i_src2, 100, -100);
    for (int i = 0; i < N; i++) {
        i_dest_scalar[i] = i_src1[i] < i_src2[i] ? 100 : -100;
    }
    if (!verify_results(i_dest_vec, i_dest_scalar, N * sizeof(int))) {
        printf("FAIL: LT_EXPR signed integer test\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with signed integers */
    test_le_int(i_dest_vec, i_src1, i_src2, 777, 333);
    for (int i = 0; i < N; i++) {
        i_dest_scalar[i] = i_src1[i] <= i_src2[i] ? 777 : 333;
    }
    if (!verify_results(i_dest_vec, i_dest_scalar, N * sizeof(int))) {
        printf("FAIL: LE_EXPR signed integer test\n");
        errors++;
    }
    
    /* Test 7: Mixed patterns in same loop */
    test_mixed_patterns(f_mixed_vec, i_mixed_vec, f_src1, f_src2, i_src1, i_src2);
    for (int i = 0; i < N; i++) {
        f_mixed_scalar[i] = f_src1[i] > f_src2[i] ? f_src1[i] : f_src2[i];
        i_mixed_scalar[i] = i_src1[i] <= i_src2[i] ? i_src1[i] : i_src2[i];
    }
    if (!verify_results(f_mixed_vec, f_mixed_scalar, N * sizeof(float)) ||
        !verify_results(i_mixed_vec, i_mixed_scalar, N * sizeof(int))) {
        printf("FAIL: Mixed pattern test\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("All tests passed!\n");
    } else {
        printf("%d test(s) failed\n", errors);
    }
    
    return errors;
}
