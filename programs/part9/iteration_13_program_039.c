/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 100
#define STRUCT_SIZE 16

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline,noipa))
#define VOLATILE_ACCESS volatile
#define FORCE_USE(x) __asm__ volatile("" : : "r"(x) : "memory")

/* Test structure for complex access patterns */
typedef struct {
    int id;
    float value;
    double data;
    char padding[STRUCT_SIZE - sizeof(int) - sizeof(float) - sizeof(double)];
} TestStruct;

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc recognition */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    FORCE_USE(sum);
    return sum;
}

NOINLINE int test_int_postinc_store(int *arr, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value + i;
    }
    
    /* Verify by reloading */
    int sum = 0;
    ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_int_postdec_load(int *arr) {
    int sum = 0;
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    FORCE_USE(sum);
    return sum;
}

NOINLINE int test_int_postdec_store(int *arr, int value) {
    int *ptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
    
    return 0;
}

NOINLINE int test_int_volatile_postinc(VOLATILE_ACCESS int *arr) {
    VOLATILE_ACCESS int *ptr = arr;
    int sum = 0;
    
    /* Volatile access pattern - prevents reordering/elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_int_pointer_arithmetic(int *arr) {
    int sum = 0;
    int *ptr = arr;
    
    /* Mixed pattern: ptr + constant offset */
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        sum += *(ptr + 0);
        sum += *(ptr + 1);
        sum += *(ptr + 2);
        sum += *(ptr + 3);
        ptr += 4;
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
    
    FORCE_USE(sum);
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
    
    FORCE_USE(sum);
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value / (i + 1);
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE double test_struct_traversal(TestStruct *arr) {
    double sum = 0.0;
    TestStruct *ptr = arr;
    
    /* Access struct members with non-one stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->id + ptr->value + ptr->data;
        ptr++;  /* Large stride = sizeof(TestStruct) */
    }
    
    return sum;
}

NOINLINE void test_struct_member_store(TestStruct *arr) {
    TestStruct *ptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->data = i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_traversal(int arr[][16]) {
    int sum = 0;
    int *ptr = &arr[0][0];
    
    /* Traverse 2D array as 1D with single pointer */
    for (int i = 0; i < ARRAY_SIZE * 16; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_nested_loop_reset(int arr[][8]) {
    int total = 0;
    
    /* Outer loop resets pointer each iteration */
    for (int row = 0; row < 32; row++) {
        int *ptr = arr[row];
        
        /* Inner loop with post-increment */
        for (int col = 0; col < 8; col++) {
            total += *ptr++;
        }
    }
    
    return total;
}

/* ========== COMPLEX PATTERNS ========== */

NOINLINE int test_mixed_increment_patterns(int *arr) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = &arr[ARRAY_SIZE/2];
    
    /* Multiple pointers with different increment patterns */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        sum += *ptr1++;      /* Post-increment */
        sum += *ptr2--;      /* Post-decrement */
    }
    
    return sum;
}

NOINLINE int test_pointer_with_constant_offset(int *base) {
    int sum = 0;
    
    /* Access with constant offset from base */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(base + i);  /* Compiler should convert to base increment */
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_array = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int (*md_array)[16] = (int(*)[16])malloc(ARRAY_SIZE * 16 * sizeof(int));
    int (*nested_array)[8] = (int(*)[8])malloc(32 * 8 * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (i + 1) * 1.1f;
        double_array[i] = (i + 1) * 1.5;
        struct_array[i].id = i;
        struct_array[i].value = i * 2.0f;
        struct_array[i].data = i * 3.0;
    }
    
    for (int i = 0; i < ARRAY_SIZE * 16; i++) {
        md_array[0][i] = i % 100;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i][j] = i * 8 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_checksum += test_int_postinc_load(int_array);
        total_checksum += test_int_postinc_store(int_array, iter);
        total_checksum += test_int_postdec_load(int_array);
        total_checksum += test_int_postdec_store(int_array, iter);
        total_checksum += test_int_volatile_postinc(int_array);
        total_checksum += test_int_pointer_arithmetic(int_array);
        
        /* Float tests */
        test_float_postinc_load(float_array);
        test_float_postinc_store(float_array, iter * 1.0f);
        
        /* Double tests */
        test_double_postinc_load(double_array);
        test_double_postinc_store(double_array, iter * 1.0);
        
        /* Struct tests */
        total_checksum += (int)test_struct_traversal(struct_array);
        test_struct_member_store(struct_array);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(md_array);
        total_checksum += test_nested_loop_reset(nested_array);
        
        /* Complex pattern tests */
        total_checksum += test_mixed_increment_patterns(int_array);
        total_checksum += test_pointer_with_constant_offset(int_array);
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    free(md_array);
    free(nested_array);
    
    return 0;
}
