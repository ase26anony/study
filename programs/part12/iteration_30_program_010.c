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
        count += (src1[i] < src2[i]);
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers */
void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    int val1 = 0xAAAA;
    int val2 = 0x5555;
    
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation */
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap transformation */
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2) {
    double val1 = 3.14159;
    double val2 = 2.71828;
    
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap transformation */
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional test cases for signed/unsigned variations */

/* GT_EXPR with signed integers in reduction */
int test_gt_signed_reduction(const int ALIGNED *src1, const int ALIGNED *src2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (src1[i] > src2[i]) ? 1 : 0;
    }
    escape(&sum);
    return sum;
}

/* GE_EXPR with unsigned integers */
void test_ge_unsigned(unsigned ALIGNED *dest, const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 0xFFFFFFFF : 0;
    }
    escape(dest);
}

/* LT_EXPR with floats */
void test_lt_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* LE_EXPR with signed integers */
void test_le_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? 100 : -100;
    }
    escape(dest);
}

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    int *mask_int = aligned_alloc(32, N * sizeof(int));
    int *ref_mask = aligned_alloc(32, N * sizeof(int));
    
    unsigned *src1_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *src2_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *dest_uint = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    float *dest_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to create mixed comparison results */
    for (i = 0; i < N; i++) {
        /* For integers: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* For unsigned: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* For floats: sine-like pattern */
        src1_float[i] = (i % 8) * 0.125f;
        src2_float[i] = ((i + 4) % 8) * 0.125f;
        
        /* For doubles: similar pattern */
        src1_double[i] = (i % 16) * 0.0625;
        src2_double[i] = ((i + 8) % 16) * 0.0625;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int);
    ref_gt_int(ref_int, src1_int, src2_int, 0xAAAA, 0x5555);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed!\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed!\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed! %u != %u\n", vec_count, ref_count);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed!\n");
        errors++;
    }
    
    printf("Testing additional patterns...\n");
    
    /* GT_EXPR signed reduction */
    int vec_sum = test_gt_signed_reduction(src1_int, src2_int);
    int ref_sum = 0;
    for (i = 0; i < N; i++) {
        ref_sum += (src1_int[i] > src2_int[i]) ? 1 : 0;
    }
    if (vec_sum != ref_sum) {
        printf("  ERROR: GT_EXPR signed reduction failed! %d != %d\n", vec_sum, ref_sum);
        errors++;
    }
    
    /* GE_EXPR unsigned */
    test_ge_unsigned(dest_uint, src1_uint, src2_uint);
    for (i = 0; i < N; i++) {
        unsigned expected = src1_uint[i] >= src2_uint[i] ? 0xFFFFFFFF : 0;
        if (dest_uint[i] != expected) {
            printf("  ERROR: GE_EXPR unsigned failed at index %d: %u != %u\n", 
                   i, dest_uint[i], expected);
            errors++;
            if (errors > 10) break;
        }
    }
    
    /* LT_EXPR float */
    test_lt_float(dest_float, src1_float, src2_float);
    for (i = 0; i < N; i++) {
        float expected = src1_float[i] < src2_float[i] ? 1.0f : 0.0f;
        if (dest_float[i] != expected) {
            printf("  ERROR: LT_EXPR float failed at index %d: %f != %f\n", 
                   i, dest_float[i], expected);
            errors++;
            if (errors > 10) break;
        }
    }
    
    /* LE_EXPR signed int */
    test_le_int(dest_int, src1_int, src2_int);
    for (i = 0; i < N; i++) {
        int expected = src1_int[i] <= src2_int[i] ? 100 : -100;
        if (dest_int[i] != expected) {
            printf("  ERROR: LE_EXPR signed int failed at index %d: %d != %d\n", 
                   i, dest_int[i], expected);
            errors++;
            if (errors > 10) break;
        }
    }
    
    /* Cleanup */
    free(src1_int);
    free(src2_int);
    free(dest_int);
    free(ref_int);
    free(mask_int);
    free(ref_mask);
    free(src1_uint);
    free(src2_uint);
    free(dest_uint);
    free(src1_float);
    free(src2_float);
    free(dest_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    
    if (errors == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", errors);
        return 1;
    }
}
