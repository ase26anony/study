/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets uncovered lines in auto-inc-dec.cc (lines 1352-1358)
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
struct test_struct {
    int id;
    float value;
    double data;
    char padding[8];
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

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
}

NOINLINE int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n-1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
}

/* Test with volatile to prevent excessive optimization */
NOINLINE int test_int_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    return sum;
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

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value * (i + 1);
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

NOINLINE void test_double_postinc_store(double *arr, int n, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_postinc_load(struct test_struct *arr, int n) {
    double sum = 0.0;
    struct test_struct *ptr = arr;
    
    /* Accessing struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->data + ptr->value;
        ptr++;  /* Large offset (sizeof struct) */
    }
    return sum;
}

NOINLINE void test_struct_postinc_store(struct test_struct *arr, int n) {
    struct test_struct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->data = i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_postinc(int arr[][16], int rows, int cols) {
    int sum = 0;
    
    /* Row-major traversal with pointer arithmetic */
    for (int i = 0; i < rows; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

NOINLINE void test_2d_array_postinc_store(int arr[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < cols; j++) {
            *ptr++ = i * cols + j;
        }
    }
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_constant_stride(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access with constant stride via pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  /* Constant stride in pointer arithmetic */
    }
    return sum;
}

/* ========== NESTED LOOP WITH POINTER RESET ========== */

NOINLINE int test_nested_loop_reset(int *arr, int outer, int inner) {
    int total = 0;
    
    for (int i = 0; i < outer; i++) {
        int *ptr = arr;
        for (int j = 0; j < inner; j++) {
            total += *ptr++;
        }
        /* ptr gets reset each outer iteration */
    }
    return total;
}

/* ========== MIXED ACCESS PATTERNS ========== */

NOINLINE void test_mixed_access(int *arr1, float *arr2, double *arr3, int n) {
    int *ip = arr1;
    float *fp = arr2;
    double *dp = arr3;
    
    for (int i = 0; i < n; i++) {
        *ip++ = i;
        *fp++ = i * 1.1f;
        *dp++ = i * 2.2;
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct test_struct *struct_array = 
        (struct test_struct*)malloc(ARRAY_SIZE * sizeof(struct test_struct));
    
    int int_2d[16][16];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 3.5f;
        struct_array[i].data = i * 4.5;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int_2d[i][j] = i * 16 + j;
        }
    }
    
    int checksum = 0;
    
    /* Run all tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        checksum += test_int_postinc_load(int_array, ARRAY_SIZE);
        
        test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        checksum += int_array[ARRAY_SIZE/2];
        
        checksum += test_int_postdec_load(int_array, ARRAY_SIZE);
        
        test_int_postdec_store(int_array, ARRAY_SIZE, iter);
        checksum += int_array[ARRAY_SIZE/2];
        
        checksum += test_int_volatile_postinc(int_array, ARRAY_SIZE);
        
        checksum += (int)test_float_postinc_load(float_array, ARRAY_SIZE);
        test_float_postinc_store(float_array, ARRAY_SIZE, iter * 1.1f);
        
        checksum += (int)test_double_postinc_load(double_array, ARRAY_SIZE);
        test_double_postinc_store(double_array, ARRAY_SIZE, iter * 2.2);
        
        checksum += (int)test_struct_postinc_load(struct_array, ARRAY_SIZE);
        test_struct_postinc_store(struct_array, ARRAY_SIZE);
        
        checksum += test_2d_array_postinc(int_2d, 16, 16);
        test_2d_array_postinc_store(int_2d, 16, 16);
        
        checksum += test_constant_stride(int_array, ARRAY_SIZE/4, 4);
        checksum += test_nested_loop_reset(int_array, 4, ARRAY_SIZE/4);
        
        test_mixed_access(int_array, float_array, double_array, ARRAY_SIZE/2);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
