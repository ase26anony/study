/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
struct TestStruct {
    int id;
    float value;
    double data;
    char tag;
    int padding[3]; /* Ensure non-power-of-two size */
};

/* ========== INTEGER ARRAY TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr, int n) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 2: Post-increment store with volatile */
    for (int i = 0; i < n; i++) {
        *vptr++ = value + i;
    }
}

NOINLINE int test_int_postdec_load(int *arr, int n) {
    volatile int *vptr = &arr[n-1];
    int sum = 0;
    
    /* Pattern 3: Post-decrement load */
    for (int i = 0; i < n; i++) {
        sum += *vptr--;
    }
    
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    volatile int *vptr = &arr[n-1];
    
    /* Pattern 4: Post-decrement store */
    for (int i = 0; i < n; i++) {
        *vptr-- = value - i;
    }
}

NOINLINE int test_int_pointer_arithmetic(int *arr, int n) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern 5: Pointer arithmetic with constant stride */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 4);  /* Non-unit stride */
        ptr += 1;           /* But ptr increments by 1 */
    }
    
    return sum;
}

/* ========== FLOATING POINT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int n) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    volatile float *vptr = arr;
    
    for (int i = 0; i < n; i++) {
        *vptr++ = value * i;
    }
}

NOINLINE double test_double_postinc_load(double *arr, int n) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < n; i++) {
        *vptr++ = value / (i + 1);
    }
}

/* ========== STRUCTURE ARRAY TESTS ========== */

NOINLINE int test_struct_array_traversal(struct TestStruct *arr, int n) {
    struct TestStruct *ptr = arr;
    int sum = 0;
    
    /* Access different members to create varied offset patterns */
    for (int i = 0; i < n; i++) {
        sum += ptr->id;        /* Offset 0 */
        sum += (int)ptr->value; /* Offset 4 (assuming 4-byte int) */
        ptr++;                 /* Post-increment by structure size */
    }
    
    return sum;
}

NOINLINE void test_struct_member_store(struct TestStruct *arr, int n) {
    struct TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->id = i;
        ptr->value = i * 1.5f;
        ptr->data = i * 2.5;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

NOINLINE int test_2d_array_row_major(int arr[][16], int rows) {
    int *ptr = &arr[0][0];
    int sum = 0;
    
    /* Traverse 2D array as 1D with pointer */
    for (int i = 0; i < rows * 16; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE int test_nested_loop_pointer_reset(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Nested loops with pointer reset each iteration */
    for (int i = 0; i < rows; i++) {
        volatile int *vptr = &arr[i * cols];
        
        for (int j = 0; j < cols; j++) {
            sum += *vptr++;
        }
    }
    
    return sum;
}

/* ========== MIXED PATTERN TESTS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr1, float *arr2, int n) {
    volatile int *iptr = arr1;
    volatile float *fptr = arr2;
    
    /* Mixed loads and stores with different types */
    for (int i = 0; i < n; i++) {
        int val = *iptr++;
        *fptr++ = (float)val * 0.5f;
    }
}

NOINLINE int test_pointer_with_constant_offset(int *arr, int n) {
    int *base_ptr = arr;
    int sum = 0;
    
    /* Access with constant offset that might be folded */
    for (int i = 0; i < n; i++) {
        sum += base_ptr[0];  /* Should become *(base_ptr + 0) */
        base_ptr++;
    }
    
    return sum;
}

/* ========== COMPLEX LOOP PATTERNS ========== */

NOINLINE int test_unrolled_loop_pattern(int *arr, int n) {
    int *ptr = arr;
    int sum = 0;
    
    /* Manually unrolled to create multiple memory ops */
    for (int i = 0; i < n; i += 4) {
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE void test_volatile_nonvolatile_mix(int *arr, int n) {
    volatile int *vptr = arr;
    int *reg_ptr = arr + n/2;
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < n/2; i++) {
        *vptr++ = i;                    /* Volatile store */
        int val = *reg_ptr++;           /* Non-volatile load */
        *vptr++ = val * 2;              /* Volatile store */
    }
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize test arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct TestStruct *struct_array = 
        (struct TestStruct*)malloc(ARRAY_SIZE * sizeof(struct TestStruct));
    
    int int_2d[16][16];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (float)(i + 1) * 1.1f;
        double_array[i] = (double)(i + 1) * 1.5;
        struct_array[i].id = i;
        struct_array[i].value = (float)i * 2.0f;
        struct_array[i].data = (double)i * 3.0;
        struct_array[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int_2d[i][j] = i * 16 + j;
        }
    }
    
    int total_sum = 0;
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array, ARRAY_SIZE);
        test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        
        total_sum += test_int_postdec_load(int_array, ARRAY_SIZE);
        test_int_postdec_store(int_array, ARRAY_SIZE, iter);
        
        total_sum += test_int_pointer_arithmetic(int_array, ARRAY_SIZE);
        
        total_sum += (int)test_float_postinc_load(float_array, ARRAY_SIZE);
        test_float_postinc_store(float_array, ARRAY_SIZE, iter * 1.0f);
        
        total_sum += (int)test_double_postinc_load(double_array, ARRAY_SIZE);
        test_double_postinc_store(double_array, ARRAY_SIZE, iter * 1.0);
        
        total_sum += test_struct_array_traversal(struct_array, ARRAY_SIZE);
        test_struct_member_store(struct_array, ARRAY_SIZE);
        
        total_sum += test_2d_array_row_major(int_2d, 16);
        total_sum += test_nested_loop_pointer_reset(int_array, 16, 16);
        
        test_mixed_increment_patterns(int_array, float_array, ARRAY_SIZE);
        total_sum += test_pointer_with_constant_offset(int_array, ARRAY_SIZE);
        
        total_sum += test_unrolled_loop_pattern(int_array, ARRAY_SIZE);
        test_volatile_nonvolatile_mix(int_array, ARRAY_SIZE);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Accumulated checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
