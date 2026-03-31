/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline,noipa))
#define VOLATILE_ACCESS volatile

/* Test structure for complex offset patterns */
struct test_struct {
    int id;
    float value;
    double data;
    char padding[8];
};

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    return arr[ARRAY_SIZE/2];
}

NOINLINE int test_int_postdec_load(int *arr) {
    int sum = 0;
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int value) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    return arr[0];
}

/* Integer with volatile pointer */
NOINLINE int test_int_volatile_postinc(VOLATILE_ACCESS int *arr) {
    int sum = 0;
    VOLATILE_ACCESS int *vptr = arr;
    
    /* Volatile access pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Integer with constant stride */
NOINLINE int test_int_stride4_load(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 4th element with pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        sum += *ptr;
        ptr += 4;  /* Constant stride - may convert to auto-inc */
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(struct test_struct *arr) {
    double sum = 0.0;
    struct test_struct *ptr = arr;
    
    /* Accessing struct members with pointer increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data + ptr->value;
        ptr->id = i;
        ptr++;
    }
    return sum;
}

/* Struct with specific member access pattern */
NOINLINE float test_struct_value_only(struct test_struct *arr) {
    float sum = 0.0f;
    struct test_struct *ptr = arr;
    
    /* Only access one member - different offset pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->value;
        ptr++;
    }
    return sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr[16][16]) {
    int sum = 0;
    int *ptr = &arr[0][0];
    
    /* Linear traversal of 2D array */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_pointer(int arr[16][16]) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < 16; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < 16; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== COMPLEX PATTERNS ========== */

/* Mixed increment/decrement pattern */
NOINLINE int test_mixed_inc_dec(int *arr) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = &arr[ARRAY_SIZE - 1];
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr1++;
        sum -= *ptr2--;
    }
    return sum;
}

/* Pointer arithmetic with base + offset */
NOINLINE int test_base_plus_offset(int *arr) {
    int sum = 0;
    
    /* arr[i] pattern that should become *(arr + i) then auto-inc */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Multiple pointers in same loop */
NOINLINE void test_multiple_pointers(int *src, int *dst) {
    int *sptr = src;
    int *dptr = dst;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *dptr++ = *sptr++ * 2;
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
    int multi_array[16][16];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 0.75f;
        struct_array[i].data = i * 1.25;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run all tests multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_array);
        total_checksum += test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        total_checksum += test_int_postdec_store(int_array, iter);
        total_checksum += test_int_volatile_postinc(int_array);
        total_checksum += test_int_stride4_load(int_array);
        
        /* Float tests */
        total_checksum += (int)test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 0.5f);
        
        /* Double tests */
        total_checksum += (int)test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, iter * 0.75);
        
        /* Struct tests */
        total_checksum += (int)test_struct_traversal(struct_array);
        total_checksum += (int)test_struct_value_only(struct_array);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(multi_array);
        total_checksum += test_nested_loop_pointer(multi_array);
        
        /* Complex patterns */
        total_checksum += test_mixed_inc_dec(int_array);
        total_checksum += test_base_plus_offset(int_array);
        
        /* Multiple pointers */
        int *dst_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
        test_multiple_pointers(int_array, dst_array);
        total_checksum += dst_array[ARRAY_SIZE/2];
        free(dst_array);
        
        /* Modify arrays slightly each iteration */
        int_array[iter % ARRAY_SIZE] = iter;
        float_array[iter % ARRAY_SIZE] = iter * 0.1f;
    }
    
    /* Final validation */
    printf("Total checksum: %d\n", total_checksum);
    printf("Sample values for verification:\n");
    printf("  int_array[100] = %d\n", int_array[100]);
    printf("  float_array[200] = %f\n", float_array[200]);
    printf("  struct_array[50].data = %f\n", struct_array[50].data);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return (total_checksum != 0) ? 0 : 1;
}
