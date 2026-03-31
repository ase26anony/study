/* test_auto_inc_dec.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag[8];
};

/* ========== INTEGER OPERATIONS ========== */

NOINLINE void test_int_postinc_load(int *arr, int *result) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with stride */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
    }
    
    *result = sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern 3: Post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    
    /* Pattern 4: Post-decrement store */
    ptr = arr + ARRAY_SIZE - 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
}

NOINLINE void test_int_postdec_load(int *arr, int *result) {
    int *ptr = arr + ARRAY_SIZE - 1;
    int sum = 0;
    
    /* Pattern 5: Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    *result = sum;
}

/* ========== FLOAT OPERATIONS ========== */

NOINLINE void test_float_postinc_load(float *arr, float *result) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Pattern 6: Float post-increment with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 7: Float with constant stride */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Alternative syntax */
    }
    
    *result = sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    /* Pattern 8: Float post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE OPERATIONS ========== */

NOINLINE void test_double_postinc_load(double *arr, double *result) {
    double *ptr = arr;
    double sum = 0.0;
    
    /* Pattern 9: Double with mixed increment patterns */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        sum += *ptr++;
        sum += *ptr++;
    }
    
    *result = sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    double *ptr = arr + ARRAY_SIZE - 1;
    
    /* Pattern 10: Double post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value / (i + 1);
    }
}

/* ========== STRUCTURE OPERATIONS ========== */

NOINLINE void test_struct_traversal(struct DataPoint *arr, double *result) {
    struct DataPoint *ptr = arr;
    double sum = 0.0;
    
    /* Pattern 11: Structure array traversal with member access */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value + ptr->precision;
        ptr++;  /* Non-trivial offset due to struct size */
    }
    
    /* Pattern 12: Direct pointer arithmetic with struct offset */
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->id;
        ptr = ptr + 1;  /* Explicit pointer arithmetic */
    }
    
    *result = sum;
}

NOINLINE void test_struct_member_store(struct DataPoint *arr) {
    struct DataPoint *ptr = arr;
    
    /* Pattern 13: Store to struct members with pointer increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->precision = i * 2.5;
        snprintf(ptr->tag, sizeof(ptr->tag), "ID%d", i);
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ACCESS ========== */

#define ROWS 16
#define COLS 16

NOINLINE void test_2d_array_traversal(int matrix[ROWS][COLS], int *result) {
    int *ptr = &matrix[0][0];
    int sum = 0;
    
    /* Pattern 14: 2D array as 1D pointer traversal */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    /* Pattern 15: Nested loops with pointer reset */
    for (int r = 0; r < ROWS; r++) {
        ptr = &matrix[r][0];
        for (int c = 0; c < COLS; c++) {
            sum += *ptr++;
        }
    }
    
    *result = sum;
}

NOINLINE void test_mixed_access_patterns(int *arr1, float *arr2, double *arr3, double *result) {
    int *iptr = arr1;
    float *fptr = arr2;
    double *dptr = arr3;
    double sum = 0.0;
    
    /* Pattern 16: Mixed type access in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *iptr++ + *fptr++ + *dptr++;
    }
    
    *result = sum;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE void test_pointer_arithmetic_with_constants(int *arr, int *result) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern 17: Explicit pointer arithmetic with constants */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        sum += *(ptr + 4);
        sum += *(ptr + 5);
        sum += *(ptr + 6);
        sum += *(ptr + 7);
        ptr += 8;
    }
    
    *result = sum;
}

NOINLINE void test_volatile_nonvolatile_mix(volatile int *varr, int *arr, int *result) {
    int sum = 0;
    
    /* Pattern 18: Mix of volatile and non-volatile accesses */
    volatile int *vptr = varr;
    int *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Volatile access */
        *ptr++ = sum;    /* Non-volatile store */
    }
    
    *result = sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataPoint *struct_array = (struct DataPoint*)malloc(ARRAY_SIZE * sizeof(struct DataPoint));
    int matrix[ROWS][COLS];
    volatile int *volatile_array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (i + 1) * 1.1f;
        double_array[i] = (i + 1) * 1.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 2.0f;
        struct_array[i].precision = i * 3.0;
        snprintf(struct_array[i].tag, sizeof(struct_array[i].tag), "TAG%d", i);
        volatile_array[i] = i * 2;
    }
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            matrix[r][c] = r * COLS + c + 1;
        }
    }
    
    /* Results storage */
    int int_result1 = 0, int_result2 = 0, int_result3 = 0;
    float float_result = 0.0f;
    double double_result1 = 0.0, double_result2 = 0.0;
    double struct_result = 0.0;
    int matrix_result = 0;
    double mixed_result = 0.0;
    int arithmetic_result = 0;
    int volatile_result = 0;
    
    /* Execute test functions multiple times to ensure optimization */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_postinc_load(int_array, &int_result1);
        test_int_postinc_store(int_array, iter);
        test_int_postdec_load(int_array, &int_result2);
        
        test_float_postinc_load(float_array, &float_result);
        test_float_postinc_store(float_array, iter * 1.5f);
        
        test_double_postinc_load(double_array, &double_result1);
        test_double_postdec_store(double_array, iter * 2.5);
        
        test_struct_traversal(struct_array, &struct_result);
        test_struct_member_store(struct_array);
        
        test_2d_array_traversal(matrix, &matrix_result);
        test_mixed_access_patterns(int_array, float_array, double_array, &mixed_result);
        test_pointer_arithmetic_with_constants(int_array, &arithmetic_result);
        test_volatile_nonvolatile_mix(volatile_array, int_array, &volatile_result);
    }
    
    /* Verify results to prevent dead code elimination */
    int final_sum = int_result1 + int_result2 + int_result3 + (int)float_result +
                   (int)double_result1 + (int)double_result2 + (int)struct_result +
                   matrix_result + (int)mixed_result + arithmetic_result + volatile_result;
    
    printf("Test completed. Final checksum: %d\n", final_sum);
    printf("Array element samples:\n");
    printf("  int_array[0] = %d, int_array[100] = %d\n", int_array[0], int_array[100]);
    printf("  float_array[50] = %f\n", float_array[50]);
    printf("  struct_array[10].id = %d\n", struct_array[10].id);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    free((void*)volatile_array);
    
    return (final_sum != 0) ? 0 : 1;
}
