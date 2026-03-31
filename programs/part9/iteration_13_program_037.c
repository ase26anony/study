/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit strides */
struct DataStruct {
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
        ptr += 1;
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
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
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

NOINLINE int test_struct_postinc_load(struct DataStruct *arr) {
    volatile struct DataStruct *vptr = arr;
    int sum = 0;
    
    /* Pattern: Structure member access with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->val;
        vptr++;  /* Large stride */
    }
    
    return sum;
}

NOINLINE void test_struct_postinc_store(struct DataStruct *arr, int val, float fval, double dval) {
    volatile struct DataStruct *vptr = arr;
    
    /* Pattern: Multiple member stores with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->val = val + i;
        vptr->fval = fval * i;
        vptr->dval = dval / (i + 1);
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_postinc(int arr[16][16]) {
    volatile int *vptr = &arr[0][0];
    int sum = 0;
    
    /* Pattern: 2D array traversal with single pointer */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE void test_nested_loop_postinc(int arr[16][16], int value) {
    /* Pattern: Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        volatile int *vptr = &arr[i][0];
        for (int j = 0; j < 16; j++) {
            *vptr++ = value + i * 16 + j;
        }
    }
}

/* ========== MIXED PATTERN TESTS ========== */

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

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_pointer_arithmetic(int *base) {
    int sum = 0;
    
    /* Pattern: Base + constant offset in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* This should create (mem (plus (reg) (const_int))) patterns */
        sum += *(base + i);
    }
    
    /* Pattern: Explicit pointer increment with stride */
    int *ptr = base;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += *ptr;
        ptr += 2;  /* Non-unit stride */
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataStruct *struct_arr = (struct DataStruct*)malloc(ARRAY_SIZE * sizeof(struct DataStruct));
    int matrix[16][16];
    
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
            matrix[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run all test functions multiple times to ensure optimization triggers */
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
        test_struct_postinc_store(struct_arr, iter, iter * 2.0f, iter * 3.0);
        
        total_sum += test_2d_array_postinc(matrix);
        test_nested_loop_postinc(matrix, iter);
        
        total_sum += test_mixed_patterns(int_arr, float_arr, double_arr);
        total_sum += test_pointer_arithmetic(int_arr);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
