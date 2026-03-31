/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit stride access */
struct DataStruct {
    int id;
    float value;
    double data;
    char tag;
    int64_t timestamp;
};

/* Global arrays to prevent complete optimization */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
struct DataStruct global_struct_array[ARRAY_SIZE];

/* ========== INTEGER OPERATIONS ========== */

NOINLINE void test_int_postinc_load(int *arr, int n) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Prevent dead code elimination */
    global_int_array[0] = sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern 2: Post-increment store */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
}

NOINLINE void test_int_postdec_load(int *arr, int n) {
    volatile int *vptr = &arr[n-1];
    int sum = 0;
    
    /* Pattern 3: Post-decrement load */
    for (int i = 0; i < n; i++) {
        sum += *vptr--;
    }
    
    global_int_array[1] = sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern 4: Post-decrement store */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
}

NOINLINE void test_int_pointer_arithmetic(int *arr, int n) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern 5: Pointer arithmetic with constant stride */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 4);  /* Non-unit stride */
        ptr += 4;
        if (ptr >= arr + n) break;
    }
    
    global_int_array[2] = sum;
}

/* ========== FLOAT OPERATIONS ========== */

NOINLINE void test_float_postinc_load(float *arr, int n) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    /* Use result to prevent elimination */
    global_float_array[0] = sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (float)i;
    }
}

NOINLINE void test_float_postdec_load(float *arr, int n) {
    volatile float *vptr = &arr[n-1];
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr--;
    }
    
    global_float_array[1] = sum;
}

/* ========== DOUBLE OPERATIONS ========== */

NOINLINE void test_double_postinc_load(double *arr, int n) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    global_double_array[0] = sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (double)i;
    }
}

NOINLINE void test_double_postdec_load(double *arr, int n) {
    volatile double *vptr = &arr[n-1];
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr--;
    }
    
    global_double_array[1] = sum;
}

/* ========== STRUCTURE OPERATIONS ========== */

NOINLINE void test_struct_postinc_load(struct DataStruct *arr, int n) {
    volatile struct DataStruct *vptr = arr;
    int64_t sum = 0;
    
    /* Access different members to test various offsets */
    for (int i = 0; i < n; i++) {
        sum += vptr->id + (int64_t)vptr->timestamp;
        vptr++;
    }
    
    global_struct_array[0].timestamp = sum;
}

NOINLINE void test_struct_postinc_store(struct DataStruct *arr, int n) {
    struct DataStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = i;
        ptr->value = (float)i * 1.5f;
        ptr->data = (double)i * 2.5;
        ptr->tag = 'A' + (i % 26);
        ptr->timestamp = i * 1000LL;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY ACCESS ========== */

#define ROWS 16
#define COLS 16

NOINLINE void test_2d_array_row_major(int arr[ROWS][COLS]) {
    int *ptr = &arr[0][0];
    int sum = 0;
    
    /* Traverse 2D array as 1D with single pointer */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    global_int_array[3] = sum;
}

NOINLINE void test_nested_loops(int *arr, int rows, int cols) {
    /* Nested loops with pointer reset */
    for (int i = 0; i < rows; i++) {
        int *ptr = &arr[i * cols];
        int row_sum = 0;
        
        for (int j = 0; j < cols; j++) {
            row_sum += *ptr++;
        }
        
        global_int_array[4 + i] = row_sum;
    }
}

/* ========== MIXED PATTERNS WITH VOLATILE ========== */

NOINLINE void test_mixed_volatile_patterns(int *arr, int n) {
    volatile int *vptr1 = arr;
    int *ptr2 = arr + n/2;
    volatile int *vptr3 = arr + n/4;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Mix volatile and non-volatile in same loop */
    for (int i = 0; i < n/4; i++) {
        sum1 += *vptr1++;      /* volatile load */
        *ptr2++ = sum1;        /* non-volatile store */
        sum3 += *vptr3++;      /* another volatile load */
    }
    
    global_int_array[5] = sum1 + sum2 + sum3;
}

/* ========== ARCHITECTURE-SPECIFIC TARGETS ========== */

/* Target ARM specifically */
#ifdef __ARM_ARCH
__attribute__((target("arch=armv7-a")))
#endif
NOINLINE void test_arm_optimized(int *arr, int n) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern designed for ARM auto-increment */
    for (int i = 0; i < n; i += 2) {
        sum += *ptr++;
        sum += *ptr++;
    }
    
    global_int_array[6] = sum;
}

/* Target PowerPC specifically */
#ifdef __powerpc__
__attribute__((target("cpu=powerpc")))
#endif
NOINLINE void test_ppc_optimized(int *arr, int n) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern for PowerPC update addressing */
    for (int i = 0; i < n; i++) {
        sum += *vptr;
        vptr += 1;  /* Separate increment to test pattern recognition */
    }
    
    global_int_array[7] = sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
        global_struct_array[i].id = i;
        global_struct_array[i].value = i * 3.5f;
        global_struct_array[i].data = i * 4.5;
        global_struct_array[i].tag = 'A' + (i % 26);
        global_struct_array[i].timestamp = i * 1000LL;
    }
    
    int local_int_array[ARRAY_SIZE];
    float local_float_array[ARRAY_SIZE];
    double local_double_array[ARRAY_SIZE];
    struct DataStruct local_struct_array[ARRAY_SIZE];
    int matrix[ROWS][COLS];
    
    /* Initialize local arrays */
    memcpy(local_int_array, global_int_array, sizeof(local_int_array));
    memcpy(local_float_array, global_float_array, sizeof(local_float_array));
    memcpy(local_double_array, global_double_array, sizeof(local_double_array));
    memcpy(local_struct_array, global_struct_array, sizeof(local_struct_array));
    
    /* Initialize matrix */
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    /* Run all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer operations */
        test_int_postinc_load(local_int_array, ARRAY_SIZE);
        test_int_postinc_store(local_int_array, ARRAY_SIZE, iter);
        test_int_postdec_load(local_int_array, ARRAY_SIZE);
        test_int_postdec_store(local_int_array, ARRAY_SIZE, iter);
        test_int_pointer_arithmetic(local_int_array, ARRAY_SIZE);
        
        /* Float operations */
        test_float_postinc_load(local_float_array, ARRAY_SIZE);
        test_float_postinc_store(local_float_array, ARRAY_SIZE, iter * 1.0f);
        test_float_postdec_load(local_float_array, ARRAY_SIZE);
        
        /* Double operations */
        test_double_postinc_load(local_double_array, ARRAY_SIZE);
        test_double_postinc_store(local_double_array, ARRAY_SIZE, iter * 1.0);
        test_double_postdec_load(local_double_array, ARRAY_SIZE);
        
        /* Structure operations */
        test_struct_postinc_load(local_struct_array, ARRAY_SIZE);
        test_struct_postinc_store(local_struct_array, ARRAY_SIZE);
        
        /* Multi-dimensional */
        test_2d_array_row_major(matrix);
        test_nested_loops(local_int_array, 8, 32);
        
        /* Mixed patterns */
        test_mixed_volatile_patterns(local_int_array, ARRAY_SIZE);
        
        /* Architecture-specific */
        test_arm_optimized(local_int_array, ARRAY_SIZE);
        test_ppc_optimized(local_int_array, ARRAY_SIZE);
    }
    
    /* Compute checksum to verify correctness */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += global_int_array[i];
        checksum += (int)global_float_array[i];
        checksum += (int)global_double_array[i];
        checksum += global_struct_array[i].id;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
