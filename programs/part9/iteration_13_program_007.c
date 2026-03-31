/* test_auto_inc_dec.c - Comprehensive test for GCC auto-increment/decrement optimization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char tag;
};

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
    return n;
}

NOINLINE int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n-1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
    return n;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float base) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double base) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base / (i + 1);
    }
}

/* ========== VOLATILE TESTS ========== */

NOINLINE int test_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *ptr = arr;
    
    /* Volatile access should still trigger auto-inc pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_volatile_postdec_store(volatile int *arr, int n) {
    volatile int *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        *ptr-- = i * 2;
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_array_load(struct TestStruct *arr, int n) {
    double sum = 0.0;
    struct TestStruct *ptr = arr;
    
    /* Accessing struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->data + ptr->value;
        ptr++;  /* Increment by struct size */
    }
    return sum;
}

NOINLINE void test_struct_array_store(struct TestStruct *arr, int n) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->data = i * 2.5;
        ptr->tag = 'A' + (i % 26);
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traverse(int *matrix, int rows, int cols) {
    int sum = 0;
    int *ptr = matrix;
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

NOINLINE void test_nested_loop_reset(int *arr, int outer, int inner) {
    /* Inner loop pointer gets reset each outer iteration */
    for (int i = 0; i < outer; i++) {
        int *ptr = &arr[i * inner];
        for (int j = 0; j < inner; j++) {
            *ptr++ = i * 100 + j;
        }
    }
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_constant_stride(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 4th element */
    for (int i = 0; i < n; i += 4) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    return sum;
}

NOINLINE void test_mixed_offsets(int *arr, int n) {
    /* Mix of different access patterns */
    int *ptr1 = arr;
    int *ptr2 = &arr[n/2];
    
    for (int i = 0; i < n/2; i++) {
        *ptr1++ = i;
        *ptr2++ = i * 2;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    const int ARRAY_SIZE = 256;
    const int MATRIX_ROWS = 16;
    const int MATRIX_COLS = 16;
    
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile int *volatile_arr = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    struct TestStruct *struct_arr = (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int *matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i + 1;
        float_arr[i] = (i + 1) * 1.1f;
        double_arr[i] = (i + 1) * 1.5;
        volatile_arr[i] = i * 2;
        struct_arr[i].id = i;
        struct_arr[i].value = i * 0.5f;
        struct_arr[i].data = i * 0.25;
        struct_arr[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = i;
    }
    
    /* Execute all test functions */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    printf("Running auto-increment/decrement pattern tests...\n");
    
    /* Integer tests */
    int_sum += test_int_postinc_load(int_arr, ARRAY_SIZE);
    test_int_postinc_store(int_arr, ARRAY_SIZE, 42);
    int_sum += test_int_postdec_load(int_arr, ARRAY_SIZE);
    test_int_postdec_store(int_arr, ARRAY_SIZE, 100);
    
    /* Float tests */
    float_sum += test_float_postinc_load(float_arr, ARRAY_SIZE);
    test_float_postinc_store(float_arr, ARRAY_SIZE, 2.5f);
    
    /* Double tests */
    double_sum += test_double_postinc_load(double_arr, ARRAY_SIZE);
    test_double_postinc_store(double_arr, ARRAY_SIZE, 3.14);
    
    /* Volatile tests */
    int_sum += test_volatile_postinc(volatile_arr, ARRAY_SIZE);
    test_volatile_postdec_store(volatile_arr, ARRAY_SIZE);
    
    /* Struct tests */
    double_sum += test_struct_array_load(struct_arr, ARRAY_SIZE);
    test_struct_array_store(struct_arr, ARRAY_SIZE);
    
    /* Multi-dimensional tests */
    int_sum += test_2d_array_traverse(matrix, MATRIX_ROWS, MATRIX_COLS);
    test_nested_loop_reset(int_arr, 8, 32);
    
    /* Constant stride tests */
    int_sum += test_constant_stride(int_arr, ARRAY_SIZE);
    test_mixed_offsets(int_arr, ARRAY_SIZE);
    
    /* Verification */
    printf("Integer checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    printf("Double checksum: %f\n", double_sum);
    
    /* Simple runtime verification */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += int_arr[i];
    }
    printf("Final array sum: %d\n", verify_sum);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free((void*)volatile_arr);
    free(struct_arr);
    free(matrix);
    
    return 0;
}
