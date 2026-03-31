/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement optimization
 * Targets specific uncovered lines in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with stride */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 1: Post-increment store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Store with pointer arithmetic */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr + i) = value - i;  /* Base + offset pattern */
    }
}

NOINLINE int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Mix of volatile and non-volatile accesses */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        sum += ptr[0] + ptr[1];  /* Multiple loads per iteration */
        ptr += 2;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * i;
    }
    
    /* Alternative pattern with constant offset */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr++) = value + i;  /* Explicit post-increment */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Simple post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pointer with stride */
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        sum += *ptr;
        ptr += 8;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    double sum = 0.0;
    
    /* Access different members with non-one offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].value + arr[i].data;
    }
    
    /* Pointer-based traversal */
    struct TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data;
        ptr++;  /* Large stride (sizeof(struct TestStruct)) */
    }
    
    return sum;
}

NOINLINE void test_struct_store(struct TestStruct *arr, int base) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = base + i;
        ptr->value = (float)(base + i) / 10.0f;
        ptr->data = (double)(base + i) / 100.0;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_traversal(int matrix[ROWS][COLS]) {
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    int *ptr = &matrix[0][0];
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < ROWS; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < COLS; j++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

NOINLINE void test_2d_array_store(int matrix[ROWS][COLS], int value) {
    int *ptr = &matrix[0][0];
    
    for (int i = 0; i < ROWS * COLS; i++) {
        *ptr++ = value + i;
    }
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE int test_mixed_patterns(int *arr1, int *arr2, int *arr3) {
    int sum = 0;
    
    /* Multiple arrays in same loop */
    int *p1 = arr1;
    int *p2 = arr2;
    int *p3 = arr3;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *p1++ + *p2++ + *p3++;
    }
    
    /* Interleaved loads and stores */
    p1 = arr1;
    p2 = arr2;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int temp = *p1++;
        *p2++ = temp * 2;
        sum += temp;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_array = 
        (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    int matrix[ROWS][COLS];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 0.5f;
        struct_array[i].data = i * 0.25;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run tests multiple times to ensure loops are executed */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        
        total_sum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 0.1f);
        
        total_sum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, iter * 0.01);
        
        total_sum += (int)test_struct_traversal(struct_array);
        test_struct_store(struct_array, iter);
        
        total_sum += test_2d_array_traversal(matrix);
        test_2d_array_store(matrix, iter);
        
        int *arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
        int *arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
        total_sum += test_mixed_patterns(int_array, arr2, arr3);
        free(arr2);
        free(arr3);
    }
    
    /* Prevent dead code elimination */
    printf("Result checksum: %d\n", total_sum);
    
    /* Verify some values */
    printf("Sample values - int[0]=%d, float[10]=%.2f, double[20]=%.2f\n",
           int_array[0], float_array[10], double_array[20]);
    
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
