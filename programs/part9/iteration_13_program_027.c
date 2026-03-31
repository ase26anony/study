/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
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

NOINLINE void test_int_postinc_load(int *arr, int n, int *result) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Simple post-increment load with volatile */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Non-volatile version */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        *result += *ptr++;
    }
    
    *result += sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Post-increment store with constant stride */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
    
    /* Store with pointer arithmetic */
    ptr = arr;
    for (int i = 0; i < n; i++) {
        *(ptr + i) = value - i;
    }
}

NOINLINE void test_int_postdec_load(int *arr, int n, int *result) {
    int *ptr = &arr[n-1];
    int sum = 0;
    
    /* Post-decrement load */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    
    *result = sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Post-decrement store */
    for (int i = 0; i < n; i++) {
        *ptr-- = value + (i * 2);
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE void test_float_postinc_load(float *arr, int n, float *result) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Mixed volatile and non-volatile accesses */
    for (int i = 0; i < n; i += 2) {
        sum += *vptr++;
        *result += arr[i + 1];
    }
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    /* Store with constant offset pattern */
    for (int i = 0; i < n; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE void test_double_postinc_load(double *arr, int n, double *result) {
    double *ptr = arr;
    double sum = 0.0;
    
    /* Simple post-increment with known iteration count */
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    
    *result = sum;
}

NOINLINE void test_double_postdec_store(double *arr, int n, double value) {
    double *ptr = &arr[n-1];
    
    /* Post-decrement with computation */
    for (int i = 0; i < n; i++) {
        *ptr-- = value / (i + 1);
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE void test_struct_traversal(struct TestStruct *arr, int n, double *result) {
    struct TestStruct *ptr = arr;
    double sum = 0.0;
    
    /* Access different struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->value + ptr->data;
        ptr->id = i;
        ptr++;
    }
    
    *result = sum;
}

NOINLINE void test_struct_member_access(struct TestStruct *arr, int n, int *ids) {
    /* Access only one member across array */
    for (int i = 0; i < n; i++) {
        ids[i] = arr[i].id;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE void test_2d_array_traversal(int *matrix, int rows, int cols, int *result) {
    int *ptr = matrix;
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;
        }
    }
    
    *result = sum;
}

NOINLINE void test_nested_loop_reset(int *arr, int outer, int inner, int *result) {
    /* Inner loop pointer gets reset each iteration */
    for (int i = 0; i < outer; i++) {
        int *ptr = &arr[i * inner];
        int local_sum = 0;
        
        for (int j = 0; j < inner; j++) {
            local_sum += *ptr++;
        }
        
        result[i] = local_sum;
    }
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr, int n, int *results) {
    int *ptr1 = arr;
    int *ptr2 = &arr[n/2];
    volatile int *vptr = arr;
    
    /* Multiple pointers with different increment patterns */
    for (int i = 0; i < n/2; i++) {
        results[0] += *ptr1++;      /* Post-increment */
        results[1] += *ptr2--;      /* Post-decrement */
        results[2] += *vptr++;      /* Volatile post-increment */
    }
}

NOINLINE void test_pointer_arithmetic_const_stride(int *arr, int n, int stride, int *result) {
    int *ptr = arr;
    
    /* Constant stride larger than 1 */
    for (int i = 0; i < n; i++) {
        *result += *ptr;
        ptr += stride;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 16;
    
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(SIZE * sizeof(int));
    float *float_arr = (float*)malloc(SIZE * sizeof(float));
    double *double_arr = (double*)malloc(SIZE * sizeof(double));
    struct TestStruct *struct_arr = (struct TestStruct*)malloc(SIZE * sizeof(struct TestStruct));
    int *matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    int *results = (int*)calloc(10, sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
        struct_arr[i].id = i;
        struct_arr[i].value = i * 1.5f;
        struct_arr[i].data = i * 2.5;
        struct_arr[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = i;
    }
    
    /* Execute all test functions */
    test_int_postinc_load(int_arr, SIZE, &results[0]);
    test_int_postinc_store(int_arr, SIZE, 42);
    test_int_postdec_load(int_arr, SIZE, &results[1]);
    test_int_postdec_store(int_arr, SIZE, 100);
    
    test_float_postinc_load(float_arr, SIZE, (float*)&results[2]);
    test_float_postinc_store(float_arr, SIZE, 3.14f);
    
    test_double_postinc_load(double_arr, SIZE, (double*)&results[3]);
    test_double_postdec_store(double_arr, SIZE, 6.28);
    
    test_struct_traversal(struct_arr, SIZE, (double*)&results[4]);
    
    int *struct_ids = (int*)malloc(SIZE * sizeof(int));
    test_struct_member_access(struct_arr, SIZE, struct_ids);
    
    test_2d_array_traversal(matrix, MATRIX_SIZE, MATRIX_SIZE, &results[5]);
    
    int *nested_results = (int*)malloc(4 * sizeof(int));
    test_nested_loop_reset(int_arr, 4, SIZE/4, nested_results);
    
    test_mixed_increment_patterns(int_arr, SIZE, &results[6]);
    test_pointer_arithmetic_const_stride(int_arr, SIZE/4, 4, &results[7]);
    
    /* Verify results aren't optimized away */
    int final_sum = 0;
    for (int i = 0; i < 10; i++) {
        final_sum += results[i];
    }
    
    for (int i = 0; i < 4; i++) {
        final_sum += nested_results[i];
    }
    
    for (int i = 0; i < SIZE; i++) {
        final_sum += struct_ids[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    free(matrix);
    free(results);
    free(struct_ids);
    free(nested_results);
    
    return 0;
}
