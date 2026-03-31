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

/* Prevent inlining to preserve loop structure for RTL analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Structure for testing non-trivial offsets */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[8];
} TestStruct;

/* Global arrays to prevent constant propagation */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
TestStruct global_struct_array[ARRAY_SIZE];

/* ========== POST-INCREMENT LOAD OPERATIONS ========== */

NOINLINE int test_postinc_load_int(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    
    /* Simple post-increment load pattern */
    for (int i = 0; i < size; i++) {
        sum += *ptr++;  /* Should trigger auto-inc recognition */
    }
    return sum;
}

NOINLINE float test_postinc_load_float(float *arr, int size) {
    float sum = 0.0f;
    volatile float *vptr = arr;  /* Volatile to prevent reordering */
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < size; i++) {
        sum += *vptr++;
    }
    return sum;
}

NOINLINE double test_postinc_load_double(double *arr, int size) {
    double sum = 0.0;
    double *ptr = arr;
    
    /* Fixed iteration count for predictable pattern */
    for (int i = 0; i < 256; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== POST-INCREMENT STORE OPERATIONS ========== */

NOINLINE void test_postinc_store_int(int *arr, int size, int value) {
    int *ptr = arr;
    
    /* Post-increment store with constant stride */
    for (int i = 0; i < size; i++) {
        *ptr++ = value + i;
    }
}

NOINLINE void test_postinc_store_float(float *arr, int size, float value) {
    volatile float *vptr = arr;
    
    /* Volatile store with post-increment */
    for (int i = 0; i < size; i++) {
        *vptr++ = value * i;
    }
}

NOINLINE void test_postinc_store_double(double *arr, int size, double value) {
    double *ptr = arr;
    
    /* Simple post-increment store */
    for (int i = 0; i < 256; i++) {
        *ptr++ = value + sin(i * 0.01);
    }
}

/* ========== POST-DECREMENT OPERATIONS ========== */

NOINLINE int test_postdec_load_int(int *arr, int size) {
    int sum = 0;
    int *ptr = &arr[size - 1];  /* Start from end */
    
    /* Post-decrement load */
    for (int i = 0; i < size; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE void test_postdec_store_int(int *arr, int size, int value) {
    int *ptr = &arr[size - 1];
    
    /* Post-decrement store */
    for (int i = 0; i < size; i++) {
        *ptr-- = value - i;
    }
}

/* ========== STRUCTURE ARRAY TRAVERSAL ========== */

NOINLINE double test_struct_array_traversal(TestStruct *arr, int size) {
    double sum = 0.0;
    TestStruct *ptr = arr;
    
    /* Access multiple members with pointer increment */
    for (int i = 0; i < size; i++) {
        sum += ptr->val + ptr->fval + ptr->dval;
        ptr++;  /* Non-trivial offset due to struct size */
    }
    return sum;
}

NOINLINE void test_struct_store(TestStruct *arr, int size) {
    TestStruct *ptr = arr;
    
    /* Store to struct members with pointer increment */
    for (int i = 0; i < size; i++) {
        ptr->val = i;
        ptr->fval = i * 1.5f;
        ptr->dval = i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TRAVERSAL ========== */

NOINLINE int test_2d_array_traversal(int arr[][16], int rows) {
    int sum = 0;
    int *ptr = &arr[0][0];
    
    /* Flattened 2D array traversal */
    for (int i = 0; i < rows * 16; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_traversal(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < rows; i++) {
        int *ptr = &arr[i * cols];
        for (int j = 0; j < cols; j++) {
            sum += *ptr++;  /* Inner loop pointer increment */
        }
    }
    return sum;
}

/* ========== POINTER ARITHMETIC WITH CONSTANT STRIDE ========== */

NOINLINE int test_pointer_arithmetic_stride(int *arr, int size, int stride) {
    int sum = 0;
    
    /* Explicit pointer arithmetic with constant stride */
    for (int i = 0; i < size; i += stride) {
        sum += *(arr + i);  /* May be converted to auto-inc with offset */
    }
    return sum;
}

NOINLINE void test_mixed_access_patterns(int *arr, int size) {
    int *ptr1 = arr;
    volatile int *ptr2 = &arr[size/2];
    
    /* Mixed volatile and non-volatile accesses */
    for (int i = 0; i < size/2; i++) {
        *ptr1++ = i;
        *ptr2++ = i * 2;
    }
}

/* ========== COMPLEX PATTERN WITH MULTIPLE POINTERS ========== */

NOINLINE int test_multiple_pointers(int *src, int *dst, int size) {
    int *src_ptr = src;
    int *dst_ptr = dst;
    int checksum = 0;
    
    /* Copy with post-increment on both pointers */
    for (int i = 0; i < size; i++) {
        int val = *src_ptr++;
        *dst_ptr++ = val;
        checksum += val;
    }
    return checksum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    TestStruct struct_array[ARRAY_SIZE];
    int multi_array[16][16];
    
    /* Initialize arrays with non-trivial values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 7;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.5 + sin(i * 0.1);
        struct_array[i].val = i;
        struct_array[i].fval = i * 0.75f;
        struct_array[i].dval = i * 1.25;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 16 + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Execute all test patterns */
    total_checksum += test_postinc_load_int(int_array, ARRAY_SIZE);
    total_checksum += (int)test_postinc_load_float(float_array, ARRAY_SIZE);
    total_checksum += (int)test_postinc_load_double(double_array, ARRAY_SIZE);
    
    test_postinc_store_int(int_array, ARRAY_SIZE, 42);
    test_postinc_store_float(float_array, ARRAY_SIZE, 3.14f);
    test_postinc_store_double(double_array, ARRAY_SIZE, 2.71828);
    
    total_checksum += test_postdec_load_int(int_array, ARRAY_SIZE);
    test_postdec_store_int(int_array, ARRAY_SIZE, 100);
    
    total_checksum += (int)test_struct_array_traversal(struct_array, ARRAY_SIZE);
    test_struct_store(struct_array, ARRAY_SIZE);
    
    total_checksum += test_2d_array_traversal(multi_array, 16);
    total_checksum += test_nested_loop_traversal(int_array, 16, 16);
    
    total_checksum += test_pointer_arithmetic_stride(int_array, ARRAY_SIZE, 4);
    test_mixed_access_patterns(int_array, ARRAY_SIZE);
    
    int dest_array[ARRAY_SIZE];
    total_checksum += test_multiple_pointers(int_array, dest_array, ARRAY_SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += int_array[i] + dest_array[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    return (total_checksum != 0 && verify_sum != 0) ? 0 : 1;
}
