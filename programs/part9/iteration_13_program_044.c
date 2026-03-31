/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define CHECKSUM_SEED 0xDEADBEEF

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit strides */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[32]; /* Force larger stride */
} TestStruct;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 1: Simple post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment store */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern: Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Pattern 1: Simple post-increment load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Pointer arithmetic with constant stride */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0); /* Zero offset, ptr increments in loop */
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    /* Pattern: Post-increment store with computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Pattern: Post-increment load in nested loop */
    for (int outer = 0; outer < 4; outer++) {
        vptr = arr; /* Reset pointer each outer iteration */
        for (int i = 0; i < ARRAY_SIZE/4; i++) {
            sum += *vptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    /* Pattern: Post-increment store with stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr = value + i;
        vptr += 1; /* Equivalent to vptr++ for doubles */
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE int test_struct_postinc_load(TestStruct *arr) {
    volatile TestStruct *vptr = arr;
    int sum = 0;
    
    /* Pattern: Access struct member with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        vptr++; /* Large stride due to struct size */
    }
    
    return sum;
}

NOINLINE void test_struct_postinc_store(TestStruct *arr, int ival, float fval, double dval) {
    volatile TestStruct *vptr = arr;
    
    /* Pattern: Store to struct members with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->val = ival + i;
        vptr->fval = fval * i;
        vptr->dval = dval / (i + 1);
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_postinc(int arr[16][16]) {
    volatile int *vptr = &arr[0][0];
    int sum = 0;
    
    /* Pattern: Traverse 2D array as 1D with pointer */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *vptr++;
    }
    
    /* Pattern: Nested loops with pointer reset */
    for (int row = 0; row < 16; row++) {
        vptr = &arr[row][0];
        for (int col = 0; col < 16; col++) {
            sum -= *vptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_2d_array_postinc_store(int arr[16][16], int value) {
    volatile int *vptr = &arr[0][0];
    
    /* Pattern: Store to 2D array with post-increment */
    for (int i = 0; i < 16 * 16; i++) {
        *vptr++ = value ^ i;
    }
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE int test_mixed_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *vptr1 = arr1;
    volatile float *vptr2 = arr2;
    volatile double *vptr3 = arr3;
    
    int isum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Mixed pattern: Different types in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        isum += *vptr1++;
        fsum += *vptr2++;
        dsum += *vptr3++;
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_arithmetic(int *base) {
    int sum = 0;
    
    /* Pattern: Base + constant offset that changes */
    for (int offset = 0; offset < ARRAY_SIZE; offset++) {
        sum += *(base + offset); /* May be converted to pointer increment */
    }
    
    /* Pattern: Multiple increments in loop */
    int *ptr = base;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *ptr;
        ptr += 2; /* Constant stride of 2 */
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_array = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int matrix[16][16];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to prevent dead code elimination */
    uint32_t checksum = CHECKSUM_SEED;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = checksum;
        float_array[i] = (float)checksum / 1000.0f;
        double_array[i] = (double)checksum / 10000.0;
        struct_array[i].val = checksum;
        struct_array[i].fval = (float)checksum / 500.0f;
        struct_array[i].dval = (double)checksum / 5000.0;
        checksum = checksum * 1103515245 + 12345; /* Simple PRNG */
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Execute all test patterns */
    int total = 0;
    
    total += test_int_postinc_load(int_array);
    test_int_postinc_store(int_array, 42);
    total += test_int_postdec_load(int_array);
    test_int_postdec_store(int_array, 99);
    
    total += (int)test_float_postinc_load(float_array);
    test_float_postinc_store(float_array, 3.14f);
    
    total += (int)test_double_postinc_load(double_array);
    test_double_postinc_store(double_array, 2.71828);
    
    total += test_struct_postinc_load(struct_array);
    test_struct_postinc_store(struct_array, 100, 1.5f, 2.5);
    
    total += test_2d_array_postinc(matrix);
    test_2d_array_postinc_store(matrix, 0xABCD);
    
    total += test_mixed_patterns(int_array, float_array, double_array);
    total += test_complex_arithmetic(int_array);
    
    /* Use results to prevent optimization */
    printf("Final checksum: %d (0x%08X)\n", total, total);
    
    /* Verify some values to ensure correctness */
    if (int_array[0] != 42 && int_array[ARRAY_SIZE-1] != 99 + (ARRAY_SIZE-1)) {
        printf("Warning: Integer array values may be incorrect\n");
    }
    
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
