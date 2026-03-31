/* auto_inc_dec_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 100

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* ========== INTEGER OPERATIONS ========== */

/* Post-increment load with simple pointer */
NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern: *ptr++ in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Post-increment store with simple pointer */
NOINLINE void test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
}

/* Post-decrement load */
NOINLINE int test_int_postdec_load(int *arr) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern: *ptr-- in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

/* Post-decrement store */
NOINLINE void test_int_postdec_store(int *arr, int value) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
}

/* Pointer arithmetic with constant stride */
NOINLINE int test_int_stride_load(int *arr) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern: *(ptr + 4) with ptr increment */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    return sum;
}

/* ========== FLOAT OPERATIONS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (float)i;
    }
}

/* ========== DOUBLE OPERATIONS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (double)i;
    }
}

/* ========== STRUCTURE OPERATIONS ========== */

/* Access struct members with non-one offsets */
NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    double sum = 0.0;
    struct TestStruct *ptr = arr;
    
    /* Access different members to create varying offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->id;
        sum += ptr->value;
        sum += ptr->data;
        ptr++;
    }
    return sum;
}

NOINLINE void test_struct_store(struct TestStruct *arr) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = (float)i * 0.5f;
        ptr->data = (double)i * 1.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int *ptr = &matrix[0][0];
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_2d_array_store(int matrix[ROWS][COLS]) {
    int *ptr = &matrix[0][0];
    
    for (int i = 0; i < ROWS * COLS; i++) {
        *ptr++ = i;
    }
}

/* ========== NESTED LOOPS ========== */

NOINLINE int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Outer loop resets pointer each iteration */
    for (int r = 0; r < rows; r++) {
        int *ptr = &arr[r * cols];
        
        /* Inner loop with pointer increment */
        for (int c = 0; c < cols; c++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== MIXED ACCESS PATTERNS ========== */

NOINLINE int test_mixed_patterns(int *arr1, float *arr2, double *arr3) {
    int *iptr = arr1;
    float *fptr = arr2;
    double *dptr = arr3;
    int result = 0;
    
    /* Mixed pointer types in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result += *iptr++;
        result += (int)(*fptr++);
        result += (int)(*dptr++);
    }
    return result;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_array = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int matrix[ROWS][COLS];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i * 0.1f;
        double_array[i] = (double)i * 0.01;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 0.5f;
        struct_array[i].data = (double)i * 1.5;
    }
    
    int total_checksum = 0;
    
    /* Run tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer operations */
        total_checksum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        total_checksum += test_int_stride_load(int_array);
        
        /* Float operations */
        total_checksum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, (float)iter);
        
        /* Double operations */
        total_checksum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, (double)iter);
        
        /* Structure operations */
        total_checksum += (int)test_struct_traversal(struct_array);
        test_struct_store(struct_array);
        
        /* Multi-dimensional array */
        total_checksum += test_2d_array_traversal(matrix);
        test_2d_array_store(matrix);
        
        /* Nested loops */
        total_checksum += test_nested_loops(int_array, 16, 16);
        
        /* Mixed patterns */
        total_checksum += test_mixed_patterns(int_array, float_array, double_array);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
