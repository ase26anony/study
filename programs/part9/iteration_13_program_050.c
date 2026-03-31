/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for complex access patterns */
struct DataStruct {
    int id;
    float value;
    double data;
    char tag;
};

/* Volatile helper to prevent elimination */
volatile int volatile_sink;

/* ========== INTEGER ARRAY OPERATIONS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: post-increment load with zero offset */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;  /* Should trigger mem_insn.reg1_val = 0 pattern */
    }
    
    volatile_sink = sum; /* Prevent dead code elimination */
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern: post-increment store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    
    volatile_sink = arr[0];
}

NOINLINE int test_int_postdec_load(int *arr) {
    int sum = 0;
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: post-decrement load */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    volatile_sink = sum;
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int value) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: post-decrement store */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    
    volatile_sink = arr[ARRAY_SIZE - 1];
}

NOINLINE int test_int_volatile_postinc(volatile int *arr) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Volatile access pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE int test_int_stride4_postinc(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: pointer arithmetic with constant stride */
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    
    volatile_sink = sum;
    return sum;
}

/* ========== FLOAT ARRAY OPERATIONS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    volatile_sink = (int)sum;
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (float)i;
    }
    
    volatile_sink = (int)arr[0];
}

/* ========== DOUBLE ARRAY OPERATIONS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    volatile_sink = (int)sum;
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + (double)i;
    }
    
    volatile_sink = (int)arr[0];
}

/* ========== STRUCTURE ARRAY OPERATIONS ========== */

NOINLINE int test_struct_postinc_load(struct DataStruct *arr) {
    int sum = 0;
    struct DataStruct *ptr = arr;
    
    /* Accessing struct members with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->id;
        ptr++;  /* Large offset due to struct size */
    }
    
    volatile_sink = sum;
    return sum;
}

NOINLINE void test_struct_postinc_store(struct DataStruct *arr, int base) {
    struct DataStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = base + i;
        ptr->value = (float)(base + i) * 0.5f;
        ptr->data = (double)(base + i) * 0.25;
        ptr->tag = 'A' + (i % 26);
        ptr++;
    }
    
    volatile_sink = arr[0].id;
}

/* ========== MULTI-DIMENSIONAL ARRAY ACCESS ========== */

#define ROWS 16
#define COLS 16

NOINLINE int test_2d_array_row_major(int arr[ROWS][COLS]) {
    int sum = 0;
    int *ptr = &arr[0][0];
    
    /* Single pointer traversing 2D array in row-major order */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    
    volatile_sink = sum;
    return sum;
}

NOINLINE int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int r = 0; r < rows; r++) {
        int *ptr = &arr[r * cols];
        
        for (int c = 0; c < cols; c++) {
            sum += *ptr++;
        }
    }
    
    volatile_sink = sum;
    return sum;
}

/* ========== MIXED ACCESS PATTERNS ========== */

NOINLINE int test_mixed_access_patterns(int *arr1, float *arr2, double *arr3) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    int *ptr1 = arr1;
    float *ptr2 = arr2;
    double *ptr3 = arr3;
    
    /* Mixed type access in same loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum_int += *ptr1++;
        sum_float += *ptr2++;
        sum_double += *ptr3++;
    }
    
    volatile_sink = sum_int + (int)sum_float + (int)sum_double;
    return sum_int;
}

/* ========== COMPLEX POINTER ARITHMETIC ========== */

NOINLINE int test_complex_pointer_arithmetic(int *base) {
    int sum = 0;
    
    /* Multiple pointer operations that might still optimize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int *ptr = base + i;
        sum += *ptr;
        
        /* Additional computation that doesn't break the pattern */
        if (sum & 1) {
            ptr = base + (i ^ 1);
            sum += *ptr;
        }
    }
    
    volatile_sink = sum;
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct DataStruct *struct_array = 
        (struct DataStruct*)malloc(ARRAY_SIZE * sizeof(struct DataStruct));
    
    int matrix[ROWS][COLS];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        double_array[i] = i * 0.25;
        struct_array[i].id = i;
        struct_array[i].value = i * 1.5f;
        struct_array[i].data = i * 2.5;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            matrix[r][c] = r * COLS + c;
        }
    }
    
    int total_sum = 0;
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array);
        test_int_postinc_store(int_array, iter);
        total_sum += test_int_postdec_load(int_array);
        test_int_postdec_store(int_array, iter);
        total_sum += test_int_volatile_postinc((volatile int*)int_array);
        total_sum += test_int_stride4_postinc(int_array);
        
        total_sum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 0.7f);
        
        total_sum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, iter * 0.3);
        
        total_sum += test_struct_postinc_load(struct_array);
        test_struct_postinc_store(struct_array, iter);
        
        total_sum += test_2d_array_row_major(matrix);
        total_sum += test_nested_loops(int_array, 16, 16);
        total_sum += test_mixed_access_patterns(int_array, float_array, double_array);
        total_sum += test_complex_pointer_arithmetic(int_array);
    }
    
    /* Verification and output */
    printf("Total checksum: %d\n", total_sum);
    printf("Array[0] = %d\n", int_array[0]);
    printf("Array[%d] = %d\n", ARRAY_SIZE-1, int_array[ARRAY_SIZE-1]);
    printf("Struct[0].id = %d\n", struct_array[0].id);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return (total_sum != 0) ? 0 : 1;
}
