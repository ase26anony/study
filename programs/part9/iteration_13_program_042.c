/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit strides */
struct TestStruct {
    int val;
    float fval;
    double dval;
    char padding[32];  /* Force larger stride */
};

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
        sum += *(ptr + 0);  /* Zero offset pattern */
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
    
    /* Pattern: Mixed volatile and non-volatile accesses */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *vptr++;
    }
    
    double *ptr = &arr[ARRAY_SIZE/2];
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value / (i + 1);
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE int test_struct_postinc_load(struct TestStruct *arr) {
    volatile struct TestStruct *vptr = arr;
    int sum = 0;
    
    /* Pattern: Structure array traversal with member access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        vptr++;  /* Large stride */
    }
    
    return sum;
}

NOINLINE void test_struct_postinc_store(struct TestStruct *arr, int value) {
    volatile struct TestStruct *vptr = arr;
    
    /* Pattern: Structure member store with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->val = value + i;
        vptr->fval = (float)(value + i);
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_postinc(int arr[16][16]) {
    volatile int *vptr = &arr[0][0];
    int sum = 0;
    
    /* Pattern: 2D array as 1D pointer traversal */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE int test_nested_loop_postinc(int arr[16][16]) {
    int sum = 0;
    
    /* Pattern: Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        volatile int *vptr = arr[i];
        for (int j = 0; j < 16; j++) {
            sum += *vptr++;
        }
    }
    
    return sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE int test_mixed_patterns(int *arr1, float *arr2, double *arr3) {
    volatile int *vptr1 = arr1;
    volatile float *vptr2 = arr2;
    volatile double *vptr3 = arr3;
    int sum = 0;
    
    /* Pattern: Interleaved accesses to different arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr1++;
        sum += (int)(*vptr2++);
        sum += (int)(*vptr3++);
    }
    
    return sum;
}

/* ========== POINTER ARITHMETIC VARIATIONS ========== */

NOINLINE int test_pointer_arithmetic(int *arr) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern: Explicit pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 0);  /* Zero offset - should trigger the uncovered block */
        ptr += 1;
    }
    
    /* Pattern: Array index notation (compiler should convert to pointer arithmetic) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_arr = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int multi_arr[16][16];
    
    if (!int_arr || !float_arr || !double_arr || !struct_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].val = i * 3;
        struct_arr[i].fval = i * 4.0f;
        struct_arr[i].dval = i * 5.0;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_arr[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run all test patterns multiple times to ensure optimization triggers */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_arr);
        test_int_postinc_store(int_arr, iter);
        total_sum += test_int_postdec_load(int_arr);
        test_int_postdec_store(int_arr, iter);
        
        total_sum += (int)test_float_postinc_load(float_arr);
        test_float_postinc_store(float_arr, iter * 1.0f);
        
        total_sum += (int)test_double_postinc_load(double_arr);
        test_double_postdec_store(double_arr, iter * 1.0);
        
        total_sum += test_struct_postinc_load(struct_arr);
        test_struct_postinc_store(struct_arr, iter);
        
        total_sum += test_2d_array_postinc(multi_arr);
        total_sum += test_nested_loop_postinc(multi_arr);
        total_sum += test_mixed_patterns(int_arr, float_arr, double_arr);
        total_sum += test_pointer_arithmetic(int_arr);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    /* Verify array contents weren't corrupted */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += int_arr[i];
    }
    printf("Array verification sum: %d\n", verify_sum);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
