/* auto_inc_dec_test.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 100

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-unit stride accesses */
struct TestStruct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* Volatile pointer variations */
volatile int volatile_array[ARRAY_SIZE];
volatile float volatile_floats[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc recognition */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Post-increment load */
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;  /* Post-increment store */
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
    
    /* Pattern: *ptr-- in loop */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;  /* Post-decrement load */
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;  /* Post-decrement store */
    }
    
    /* Verify */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Float post-increment load */
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float base) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base + (float)i * 0.5f;  /* Float post-increment store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;  /* Double post-increment load */
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double base) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = base + (double)i * 0.25;  /* Double post-increment store */
    }
}

/* ========== VOLATILE ACCESS TESTS ========== */

NOINLINE int test_volatile_postinc(void) {
    int sum = 0;
    volatile int *vptr = volatile_array;
    
    /* Volatile access with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;  /* Volatile post-increment load */
    }
    return sum;
}

NOINLINE void test_volatile_postinc_store(int value) {
    volatile int *vptr = volatile_array;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;  /* Volatile post-increment store */
    }
}

/* ========== STRUCTURE ARRAY TESTS ========== */

NOINLINE double test_struct_traversal(struct TestStruct *arr, int n) {
    double sum = 0.0;
    struct TestStruct *ptr = arr;
    
    /* Access multiple members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->value + ptr->data;  /* Mixed type access */
        ptr++;  /* Explicit increment - compiler should recognize pattern */
    }
    return sum;
}

NOINLINE void test_struct_init(struct TestStruct *arr, int n) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = i;
        ptr->value = (float)i * 1.5f;
        ptr->data = (double)i * 2.5;
        ptr++;  /* Explicit increment */
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_row_major(int arr[][16], int rows) {
    int sum = 0;
    int *ptr = &arr[0][0];  /* Treat as 1D array */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < rows * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loops(int arr[][8], int rows) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int r = 0; r < rows; r++) {
        int *ptr = &arr[r][0];
        for (int c = 0; c < 8; c++) {
            sum += *ptr++;  /* Inner loop pointer increment */
        }
    }
    return sum;
}

/* ========== CONSTANT STRIDE TESTS ========== */

NOINLINE int test_constant_stride(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access with constant stride */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  /* Constant stride increment */
    }
    return sum;
}

NOINLINE int test_pointer_arithmetic(int *arr, int n) {
    int sum = 0;
    
    /* Explicit pointer arithmetic in loop */
    for (int i = 0; i < n; i++) {
        sum += *(arr + i);  /* May be converted to auto-inc */
    }
    return sum;
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE int test_mixed_load_store(int *src, int *dst, int n) {
    int *src_ptr = src;
    int *dst_ptr = dst;
    
    /* Mixed load/store with post-increment */
    for (int i = 0; i < n; i++) {
        int val = *src_ptr++;  /* Load with post-inc */
        *dst_ptr++ = val * 2;  /* Store with post-inc */
    }
    
    /* Verify */
    int sum = 0;
    dst_ptr = dst;
    for (int i = 0; i < n; i++) {
        sum += *dst_ptr++;
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main(void) {
    /* Initialize arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    struct TestStruct struct_array[ARRAY_SIZE/4];
    int array_2d[16][16];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = (float)i * 0.1f;
        double_array[i] = (double)i * 0.01;
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 1.1f;
        struct_array[i].data = (double)i * 2.2;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run all tests multiple times to ensure pattern recognition */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_array, ARRAY_SIZE);
        total_checksum += test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        total_checksum += test_int_postdec_load(int_array, ARRAY_SIZE);
        total_checksum += test_int_postdec_store(int_array, ARRAY_SIZE, iter);
        
        /* Float tests */
        float float_sum = test_float_postinc_load(float_array, ARRAY_SIZE);
        total_checksum += (int)float_sum;
        test_float_postinc_store(float_array, ARRAY_SIZE, (float)iter);
        
        /* Double tests */
        double double_sum = test_double_postinc_load(double_array, ARRAY_SIZE);
        total_checksum += (int)double_sum;
        test_double_postinc_store(double_array, ARRAY_SIZE, (double)iter);
        
        /* Volatile tests */
        total_checksum += test_volatile_postinc();
        test_volatile_postinc_store(iter);
        
        /* Structure tests */
        double struct_sum = test_struct_traversal(struct_array, ARRAY_SIZE/4);
        total_checksum += (int)struct_sum;
        test_struct_init(struct_array, ARRAY_SIZE/4);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_row_major(array_2d, 16);
        total_checksum += test_nested_loops(array_2d, 16);
        
        /* Constant stride tests */
        total_checksum += test_constant_stride(int_array, ARRAY_SIZE/2, 2);
        total_checksum += test_pointer_arithmetic(int_array, ARRAY_SIZE);
        
        /* Mixed pattern test */
        int dest_array[ARRAY_SIZE];
        total_checksum += test_mixed_load_store(int_array, dest_array, ARRAY_SIZE);
    }
    
    /* Final validation */
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed. Compile with appropriate flags to trigger auto-inc-dec optimization.\n");
    
    return 0;
}
