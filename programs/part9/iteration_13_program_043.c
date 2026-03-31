/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets lines 1352-1358 in auto-inc-dec.cc
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
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
    int32_t checksum;
};

/* Global arrays to prevent complete optimization */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
struct DataPoint global_struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE void test_int_postinc_load(int *arr, int n) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Prevent dead code elimination */
    global_int_array[0] = sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 1: Simple post-increment store */
    for (int i = 0; i < n; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE void test_int_postdec_load(int *arr, int n) {
    volatile int *vptr = &arr[n-1];
    int sum = 0;
    
    /* Pattern 1: Simple post-decrement load */
    for (int i = 0; i < n; i++) {
        sum += *vptr--;
    }
    
    /* Pattern 2: Non-volatile pointer with post-decrement */
    int *ptr = &arr[n-1];
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    
    global_int_array[1] = sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    volatile int *vptr = &arr[n-1];
    
    /* Pattern 1: Simple post-decrement store */
    for (int i = 0; i < n; i++) {
        *vptr-- = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-decrement */
    int *ptr = &arr[n-1];
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
}

/* ========== FLOAT TESTS ========== */

NOINLINE void test_float_postinc_load(float *arr, int n) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Pattern with constant stride */
    for (int i = 0; i < n; i++) {
        sum += *vptr;
        vptr = vptr + 1;  /* Alternative form */
    }
    
    /* Direct post-increment */
    float *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    global_float_array[0] = sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    volatile float *vptr = arr;
    
    /* Mixed offset patterns */
    for (int i = 0; i < n; i++) {
        *(vptr + 0) = value + i;
        vptr++;
    }
    
    /* Simple post-increment */
    float *ptr = arr;
    for (int i = 0; i < n; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE void test_double_postinc_load(double *arr, int n) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Multiple access patterns in same loop */
    for (int i = 0; i < n; i += 2) {
        sum += *vptr++;
        sum += *vptr++;
    }
    
    global_double_array[0] = sum;
}

NOINLINE void test_double_postdec_store(double *arr, int n, double value) {
    volatile double *vptr = &arr[n-1];
    
    /* Post-decrement with computation */
    for (int i = 0; i < n; i++) {
        *vptr-- = value / (i + 1);
    }
}

/* ========== STRUCTURE TESTS ========== */

NOINLINE void test_struct_traversal(struct DataPoint *arr, int n) {
    volatile struct DataPoint *vptr = arr;
    int total = 0;
    
    /* Access structure members with pointer arithmetic */
    for (int i = 0; i < n; i++) {
        total += vptr->id;
        total += (int)vptr->value;
        vptr++;
    }
    
    /* Alternative: pointer to member */
    int *id_ptr = &arr[0].id;
    for (int i = 0; i < n; i++) {
        total += *id_ptr;
        id_ptr = (int*)((char*)id_ptr + sizeof(struct DataPoint));
    }
    
    global_struct_array[0].checksum = total;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE void test_2d_array_traversal(int arr[][16], int rows, int cols) {
    volatile int *ptr = &arr[0][0];
    int sum = 0;
    
    /* Single pointer traversing 2D array in row-major order */
    for (int i = 0; i < rows * cols; i++) {
        sum += *ptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < rows; i++) {
        volatile int *row_ptr = &arr[i][0];
        for (int j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
    }
    
    global_int_array[2] = sum;
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr, int n) {
    volatile int *vptr1 = arr;
    volatile int *vptr2 = &arr[n/2];
    int sum = 0;
    
    /* Two pointers incrementing in same loop */
    for (int i = 0; i < n/2; i++) {
        sum += *vptr1++;
        sum += *vptr2++;
    }
    
    /* Pointer with stride */
    int *stride_ptr = arr;
    for (int i = 0; i < n/4; i++) {
        sum += *stride_ptr;
        stride_ptr += 4;  /* Constant stride */
    }
    
    global_int_array[3] = sum;
}

NOINLINE void test_pointer_arithmetic_with_offset(int *base, int n) {
    volatile int *ptr = base;
    int sum = 0;
    
    /* Access with explicit offset that should become zero offset */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);  /* Should match mem_insn.reg1_val = 0 pattern */
        ptr++;
    }
    
    /* Multiple offsets */
    ptr = base;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Array notation */
        ptr++;
    }
    
    global_int_array[4] = sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 3.0f;
        global_struct_array[i].precision = i * 4.0;
        global_struct_array[i].tag = 'A' + (i % 26);
        global_struct_array[i].checksum = 0;
    }
    
    int local_int_array[ARRAY_SIZE];
    float local_float_array[ARRAY_SIZE];
    double local_double_array[ARRAY_SIZE];
    struct DataPoint local_struct_array[ARRAY_SIZE];
    int matrix[16][16];
    
    /* Initialize local arrays */
    memcpy(local_int_array, global_int_array, sizeof(local_int_array));
    memcpy(local_float_array, global_float_array, sizeof(local_float_array));
    memcpy(local_double_array, global_double_array, sizeof(local_double_array));
    memcpy(local_struct_array, global_struct_array, sizeof(local_struct_array));
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Run all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_postinc_load(local_int_array, ARRAY_SIZE);
        test_int_postinc_store(local_int_array, ARRAY_SIZE, iter);
        test_int_postdec_load(local_int_array, ARRAY_SIZE);
        test_int_postdec_store(local_int_array, ARRAY_SIZE, iter);
        
        test_float_postinc_load(local_float_array, ARRAY_SIZE);
        test_float_postinc_store(local_float_array, ARRAY_SIZE, iter * 1.0f);
        
        test_double_postinc_load(local_double_array, ARRAY_SIZE);
        test_double_postdec_store(local_double_array, ARRAY_SIZE, iter * 2.0);
        
        test_struct_traversal(local_struct_array, ARRAY_SIZE);
        test_2d_array_traversal(matrix, 16, 16);
        test_mixed_increment_patterns(local_int_array, ARRAY_SIZE);
        test_pointer_arithmetic_with_offset(local_int_array, ARRAY_SIZE);
    }
    
    /* Verify results to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += global_int_array[i];
        final_sum += (int)global_float_array[i];
        final_sum += (int)global_double_array[i];
        final_sum += global_struct_array[i].checksum;
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
