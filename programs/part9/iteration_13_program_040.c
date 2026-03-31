/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization
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
    
    /* Pattern 1: Simple post-increment load */
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
    
    /* Pattern 1: Simple post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Store with pointer arithmetic */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(ptr + i) = value * i;  /* Base + index pattern */
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
        *vptr-- = value - i;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Mixed volatile and non-volatile patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr[i];  /* Array index notation */
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    /* Post-increment store with computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value * sinf(i * 0.1f);
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
    
    /* Pointer with constant offset */
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(ptr + 1);  /* Constant offset of 1 element */
        ptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postdec_store(double *arr, double value) {
    volatile double *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value / (i + 1);
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE double test_struct_traversal(struct TestStruct *arr) {
    double sum = 0.0;
    
    /* Access different struct members with constant offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i].value;  /* Offset from base */
        sum += arr[i].data;   /* Larger offset */
    }
    
    /* Pointer arithmetic with struct stride */
    struct TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data;
        ptr++;  /* Increment by struct size */
    }
    
    return sum;
}

NOINLINE void test_struct_store(struct TestStruct *arr) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = i * 0.5f;
        ptr->data = i * 1.5;
        ptr++;  /* Post-increment of struct pointer */
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int matrix[16][16]) {
    int sum = 0;
    
    /* Row-major traversal with single pointer */
    int *ptr = &matrix[0][0];
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int row = 0; row < 16; row++) {
        int *row_ptr = matrix[row];
        for (int col = 0; col < 16; col++) {
            sum += *row_ptr++;
        }
    }
    
    return sum;
}

NOINLINE int test_complex_pattern(int *arr1, int *arr2, int *arr3) {
    volatile int *vptr1 = arr1;
    volatile int *vptr2 = arr2;
    int *ptr3 = arr3;
    int sum = 0;
    
    /* Multiple pointers with different access patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int val1 = *vptr1++;
        int val2 = *vptr2--;
        *ptr3++ = val1 + val2;
        sum += val1 * val2;
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_arr = (int*)aligned_alloc(16, ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)aligned_alloc(16, ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)aligned_alloc(16, ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_arr = (struct TestStruct*)aligned_alloc(16, 
        ARRAY_SIZE * sizeof(struct TestStruct));
    
    int matrix[16][16];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 1.5;
        struct_arr[i].id = i;
        struct_arr[i].value = i * 0.25f;
        struct_arr[i].data = i * 2.5;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Run tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_arr);
        test_int_postinc_store(int_arr, iter);
        total_sum += test_int_postdec_load(int_arr);
        test_int_postdec_store(int_arr, iter);
        
        total_sum += (int)test_float_postinc_load(float_arr);
        test_float_postinc_store(float_arr, iter * 0.1f);
        
        total_sum += (int)test_double_postinc_load(double_arr);
        test_double_postdec_store(double_arr, iter * 0.5);
        
        total_sum += (int)test_struct_traversal(struct_arr);
        test_struct_store(struct_arr);
        
        total_sum += test_2d_array_traversal(matrix);
        
        /* Create additional arrays for complex pattern test */
        int *arr2 = (int*)aligned_alloc(16, ARRAY_SIZE * sizeof(int));
        int *arr3 = (int*)aligned_alloc(16, ARRAY_SIZE * sizeof(int));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr2[i] = ARRAY_SIZE - i;
        }
        
        total_sum += test_complex_pattern(int_arr, arr2, arr3);
        
        free(arr2);
        free(arr3);
    }
    
    /* Verification and output */
    printf("Total checksum: %d\n", total_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
