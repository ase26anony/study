/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets uncovered lines in auto-inc-dec.cc:1352-1358
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
typedef struct {
    int id;
    float value;
    double data;
    char padding[8];
} TestStruct;

/* Global arrays to prevent constant propagation */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
TestStruct global_struct_array[ARRAY_SIZE];

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
    
    /* Verify by reading back */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
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

NOINLINE int test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
    
    /* Verify */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Test with volatile to prevent reordering */
NOINLINE int test_int_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Test with constant stride */
NOINLINE int test_int_stride4(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 4th element with pointer arithmetic */
    for (int i = 0; i < n/4; i++) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
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
        *ptr++ = value * i;
    }
}

NOINLINE float test_float_postdec_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
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

NOINLINE double test_struct_traversal(TestStruct *arr, int n) {
    double sum = 0.0;
    TestStruct *ptr = arr;
    
    /* Access struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->value + ptr->data;
        ptr++;  /* Large offset between structs */
    }
    return sum;
}

NOINLINE void test_struct_member_store(TestStruct *arr, int n, int base) {
    TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = base + i;
        ptr->value = (float)i * 0.5f;
        ptr->data = (double)i * 1.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr[][16], int rows, int cols) {
    int sum = 0;
    int *ptr = &arr[0][0];  /* Flatten to single pointer */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < rows * cols; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_pointer(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Nested loops with pointer reset each iteration */
    for (int r = 0; r < rows; r++) {
        int *ptr = &arr[r * cols];
        for (int c = 0; c < cols; c++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== COMPLEX PATTERNS ========== */

/* Mix load and store in same loop */
NOINLINE void test_mixed_load_store(int *src, int *dst, int n) {
    int *sptr = src;
    int *dptr = dst;
    
    for (int i = 0; i < n; i++) {
        *dptr++ = *sptr++ * 2;
    }
}

/* Pointer arithmetic with constant offset */
NOINLINE int test_constant_offset(int *arr, int n) {
    int sum = 0;
    
    /* arr[i] pattern that should become pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Should become *(arr + i) then auto-inc */
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    TestStruct struct_array[ARRAY_SIZE];
    int array_2d[16][16];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        float_array[i] = (float)i * 0.25f;
        double_array[i] = (double)i * 0.125;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 0.5f;
        struct_array[i].data = (double)i * 0.75;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    float float_checksum = 0.0f;
    double double_checksum = 0.0;
    
    /* Run all tests multiple times to ensure loops execute */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_array, ARRAY_SIZE);
        total_checksum += test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        total_checksum += test_int_postdec_load(int_array, ARRAY_SIZE);
        total_checksum += test_int_postdec_store(int_array, ARRAY_SIZE, iter);
        total_checksum += test_int_volatile_postinc((volatile int*)int_array, ARRAY_SIZE);
        total_checksum += test_int_stride4(int_array, ARRAY_SIZE);
        
        /* Float tests */
        float_checksum += test_float_postinc_load(float_array, ARRAY_SIZE);
        test_float_postinc_store(float_array, ARRAY_SIZE, (float)iter);
        float_checksum += test_float_postdec_load(float_array, ARRAY_SIZE);
        
        /* Double tests */
        double_checksum += test_double_postinc_load(double_array, ARRAY_SIZE);
        test_double_postinc_store(double_array, ARRAY_SIZE, (double)iter);
        
        /* Struct tests */
        double_checksum += test_struct_traversal(struct_array, ARRAY_SIZE);
        test_struct_member_store(struct_array, ARRAY_SIZE, iter);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(array_2d, 16, 16);
        total_checksum += test_nested_loop_pointer(int_array, 16, 16);
        
        /* Complex patterns */
        int dest_array[ARRAY_SIZE];
        test_mixed_load_store(int_array, dest_array, ARRAY_SIZE);
        total_checksum += test_constant_offset(int_array, ARRAY_SIZE);
        
        /* Use results to prevent dead code elimination */
        if (iter % 100 == 0) {
            printf("Iteration %d: checksum = %d\n", iter, total_checksum);
        }
    }
    
    /* Final validation output */
    printf("Final integer checksum: %d\n", total_checksum);
    printf("Final float checksum: %f\n", float_checksum);
    printf("Final double checksum: %f\n", double_checksum);
    
    /* Use results to prevent optimization */
    if (total_checksum > 0 && float_checksum > 0 && double_checksum > 0) {
        printf("All tests completed successfully.\n");
    }
    
    return 0;
}
