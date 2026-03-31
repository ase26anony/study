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
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[8];
} TestStruct;

/* ========== INTEGER TESTS ========== */

NOINLINE void test_int_postinc_load(int *arr, int *result) {
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
    
    *result = sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 1: Simple post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE void test_int_postdec_load(int *arr, int *result) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    int sum = 0;
    
    /* Pattern: Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    *result = sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = arr + ARRAY_SIZE - 1;
    
    /* Pattern: Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

NOINLINE void test_int_constant_stride(int *arr, int *result) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: Pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    
    *result = sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE void test_float_postinc_load(float *arr, float *result) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    *result = sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (float)i;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - (float)i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE void test_double_postinc_load(double *arr, double *result) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    *result = sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (double)i;
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - (double)i;
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE void test_struct_traversal(TestStruct *arr, double *result) {
    volatile TestStruct *vptr = arr;
    double sum = 0.0;
    
    /* Access different struct members with pointer increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        sum += vptr->fval;
        sum += vptr->dval;
        vptr++;
    }
    
    TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->val;
        sum += ptr->fval;
        sum += ptr->dval;
        ptr++;
    }
    
    *result = sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE void test_2d_array_traversal(int arr2d[16][16], int *result) {
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    int *ptr = &arr2d[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        int *row_ptr = arr2d[i];
        for (int j = 0; j < 16; j++) {
            sum += *row_ptr++;
        }
    }
    
    *result = sum;
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr, int *result) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Mix of increments and direct accesses */
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *vptr;      /* Direct access */
        vptr++;
        sum += *vptr++;    /* Post-increment access */
    }
    
    *result = sum;
}

NOINLINE void test_pointer_arithmetic_loop(int *arr, int *result) {
    int sum = 0;
    int *ptr = arr;
    
    /* Complex pointer arithmetic that should simplify */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[0];
        sum += ptr[1];
        ptr = ptr + 2;
    }
    
    *result = sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)aligned_alloc(16, ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)aligned_alloc(16, ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)aligned_alloc(16, ARRAY_SIZE * sizeof(double));
    TestStruct *struct_arr = (TestStruct*)aligned_alloc(16, ARRAY_SIZE * sizeof(TestStruct));
    int arr2d[16][16];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        float_arr[i] = sinf(i * 0.1f);
        double_arr[i] = cos(i * 0.05);
        struct_arr[i].val = i;
        struct_arr[i].fval = i * 0.5f;
        struct_arr[i].dval = i * 0.25;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    /* Run tests multiple times to ensure patterns are optimized */
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    double struct_result = 0.0;
    int array2d_result = 0;
    int mixed_result = 0;
    int ptr_arith_result = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_postinc_load(int_arr, &int_result);
        test_int_postinc_store(int_arr, iter);
        test_int_postdec_load(int_arr, &int_result);
        test_int_postdec_store(int_arr, iter);
        test_int_constant_stride(int_arr, &int_result);
        
        test_float_postinc_load(float_arr, &float_result);
        test_float_postinc_store(float_arr, iter * 0.1f);
        
        test_double_postinc_load(double_arr, &double_result);
        test_double_postinc_store(double_arr, iter * 0.05);
        
        test_struct_traversal(struct_arr, &struct_result);
        
        test_2d_array_traversal(arr2d, &array2d_result);
        
        test_mixed_increment_patterns(int_arr, &mixed_result);
        test_pointer_arithmetic_loop(int_arr, &ptr_arith_result);
        
        /* Prevent compiler from optimizing everything away */
        asm volatile("" : : "r"(int_result), "r"(float_result), 
                       "r"(double_result), "r"(struct_result));
    }
    
    /* Verify results */
    printf("Test Results:\n");
    printf("Integer operations: %d\n", int_result);
    printf("Float operations: %f\n", float_result);
    printf("Double operations: %f\n", double_result);
    printf("Struct operations: %f\n", struct_result);
    printf("2D array operations: %d\n", array2d_result);
    printf("Mixed patterns: %d\n", mixed_result);
    printf("Pointer arithmetic: %d\n", ptr_arith_result);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
