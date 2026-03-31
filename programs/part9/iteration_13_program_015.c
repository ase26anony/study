/* test_auto_inc_dec.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit strides */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
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
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern 1: Simple post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern 1: Simple post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

NOINLINE int test_int_pointer_arithmetic(int *arr) {
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
    
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
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

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + (double)i;
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE double test_struct_traversal(struct DataPoint *arr) {
    double sum = 0.0;
    
    /* Pattern: Access specific member with non-unit stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].precision;
    }
    
    /* Pattern: Pointer to struct with post-increment */
    struct DataPoint *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value;
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_struct_store(struct DataPoint *arr) {
    struct DataPoint *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = (float)i * 0.5f;
        ptr->precision = (double)i * 0.25;
        ptr->tag = 'A' + (i % 26);
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int sum = 0;
    
    /* Pattern: Traverse 2D array with single pointer */
    int *ptr = &matrix[0][0];
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    /* Pattern: Nested loops with pointer reset */
    for (int i = 0; i < ROWS; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < COLS; j++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_2d_array_store(int matrix[ROWS][COLS]) {
    int *ptr = &matrix[0][0];
    int counter = 0;
    
    for (int i = 0; i < ROWS * COLS; i++) {
        *ptr++ = counter++;
    }
}

/* ========== COMPLEX PATTERN TESTS ========== */

NOINLINE int test_mixed_increment_decrement(int *arr) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = &arr[ARRAY_SIZE - 1];
    
    /* Mixed increment and decrement in same loop */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr1++;
        sum += *ptr2--;
    }
    
    return sum;
}

NOINLINE int test_offset_access(int *arr) {
    int sum = 0;
    int *base_ptr = arr;
    
    /* Access with constant offset from moving base */
    for (int i = 0; i < ARRAY_SIZE - 4; i++) {
        sum += base_ptr[0];  /* Should become *(base_ptr) */
        sum += base_ptr[2];  /* Should become *(base_ptr + 2) */
        sum += base_ptr[4];  /* Should become *(base_ptr + 4) */
        base_ptr++;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataPoint *struct_array = (struct DataPoint*)malloc(ARRAY_SIZE * sizeof(struct DataPoint));
    int matrix[ROWS][COLS];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i * 1.5f;
        double_array[i] = (double)i * 2.5;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 0.75f;
        struct_array[i].precision = (double)i * 1.25;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run tests multiple times to ensure optimization triggers */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        total_sum += test_int_pointer_arithmetic(int_array);
        total_sum += test_mixed_increment_decrement(int_array);
        total_sum += test_offset_access(int_array);
        
        /* Float tests */
        volatile float float_sum = test_float_postinc_load(float_array);
        total_sum += (int)float_sum;
        test_float_postinc_store(float_array, (float)iter);
        
        /* Double tests */
        volatile double double_sum = test_double_postinc_load(double_array);
        total_sum += (int)double_sum;
        test_double_postinc_store(double_array, (double)iter);
        
        /* Structure tests */
        volatile double struct_sum = test_struct_traversal(struct_array);
        total_sum += (int)struct_sum;
        test_struct_store(struct_array);
        
        /* 2D array tests */
        total_sum += test_2d_array_traversal(matrix);
        test_2d_array_store(matrix);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
