/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets specific uncovered lines in auto-inc-dec.cc (lines 1352-1358)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Prevent unwanted optimizations that might obscure patterns */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_ACCESS volatile
#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* ========== Basic Type Array Traversal ========== */

/* Integer array post-increment load */
NOINLINE int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern: *ptr++ - should trigger auto-inc recognition */
        sum += *ptr++;
    }
    return sum;
}

/* Integer array post-increment store */
NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Pattern: *ptr++ = value */
        *ptr++ = value + i;
    }
}

/* Integer array post-decrement load */
NOINLINE int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n-1];  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Pattern: *ptr-- */
        sum += *ptr--;
    }
    return sum;
}

/* Float array post-increment operations */
NOINLINE float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Should generate similar pattern for floats */
        sum += *ptr++;
    }
    return sum;
}

/* Double array with mixed increment patterns */
NOINLINE double test_double_mixed(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    /* Mix of loads and stores */
    for (int i = 0; i < n; i++) {
        double temp = *ptr++;      /* Post-increment load */
        *ptr = temp * 2.0;         /* Store to next location */
        ptr++;                     /* Manual increment - different pattern */
    }
    return sum;
}

/* ========== Volatile Access Patterns ========== */

/* Volatile pointer with post-increment */
NOINLINE int test_volatile_postinc(VOLATILE_ACCESS int *arr, int n) {
    int sum = 0;
    VOLATILE_ACCESS int *vptr = arr;
    
    /* Volatile prevents reordering/elimination */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Mixed volatile and non-volatile in same loop */
NOINLINE int test_mixed_volatile(int *arr, VOLATILE_ACCESS int *varr, int n) {
    int sum = 0;
    int *ptr = arr;
    VOLATILE_ACCESS int *vptr = varr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;      /* Non-volatile */
        *vptr++ = sum;      /* Volatile store */
    }
    return sum;
}

/* ========== Structure Array Traversal ========== */

typedef struct {
    int id;
    float value;
    double data;
    char tag[4];
} TestStruct;

/* Structure array with member access */
NOINLINE float test_struct_traversal(TestStruct *arr, int n) {
    float total = 0.0f;
    TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Accessing struct members with constant offsets */
        total += ptr->value;
        ptr->data = total;
        ptr++;  /* Pointer increment by struct size */
    }
    return total;
}

/* Structure array with pointer arithmetic */
NOINLINE int test_struct_pointer_arithmetic(TestStruct *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Using array indexing - compiler may convert to pointer arithmetic */
        sum += arr[i].id;
        arr[i].value = sum * 0.5f;
    }
    return sum;
}

/* ========== Multi-Dimensional Array Access ========== */

#define ROWS 16
#define COLS 16

/* 2D array traversal with single pointer */
NOINLINE int test_2d_array_single_pointer(int matrix[ROWS][COLS]) {
    int sum = 0;
    int *ptr = &matrix[0][0];
    
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* 2D array with nested loops */
NOINLINE int test_2d_array_nested(int matrix[ROWS][COLS]) {
    int sum = 0;
    
    for (int i = 0; i < ROWS; i++) {
        int *row_ptr = matrix[i];
        for (int j = 0; j < COLS; j++) {
            /* Inner loop pointer gets reset each outer iteration */
            sum += *row_ptr++;
        }
    }
    return sum;
}

/* ========== Complex Patterns with Constant Stride ========== */

/* Pointer arithmetic with constant stride */
NOINLINE int test_constant_stride(int *arr, int n, int stride) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 'stride' elements */
    for (int i = 0; i < n; i += stride) {
        sum += *(ptr + i);  /* Constant offset pattern */
    }
    return sum;
}

/* Mixed increment patterns in same function */
NOINLINE void test_mixed_patterns(int *arr1, int *arr2, int n) {
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple memory operations with pointer updates */
        int val1 = *p1++;
        int val2 = *p2++;
        *p1 = val1 + val2;
        p1++;
        *p2 = val1 - val2;
        p2++;
    }
}

/* ========== Loop Unrolling Candidates ========== */

/* Manual unrolling to expose patterns */
NOINLINE int test_unrolled_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i;
    
    /* Process 4 elements per iteration */
    for (i = 0; i < n - 3; i += 4) {
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
    }
    
    /* Handle remainder */
    for (; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== Main Driver ========== */

int main() {
    /* Allocate and initialize test arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    VOLATILE_ACCESS int volatile_array[ARRAY_SIZE];
    TestStruct struct_array[ARRAY_SIZE / 4];  /* Fewer structs due to size */
    int matrix[ROWS][COLS];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (i + 1) * 1.5f;
        double_array[i] = (i + 1) * 2.5;
        volatile_array[i] = i * 3;
    }
    
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        struct_array[i].id = i;
        struct_array[i].value = i * 10.0f;
        struct_array[i].data = i * 100.0;
        snprintf(struct_array[i].tag, 4, "T%d", i);
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
    
    int total_sum = 0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_sum += test_int_postinc_load(int_array, ARRAY_SIZE);
        test_int_postinc_store(int_array, ARRAY_SIZE, iter);
        total_sum += test_int_postdec_load(int_array, ARRAY_SIZE);
        
        total_sum += (int)test_float_postinc_load(float_array, ARRAY_SIZE);
        total_sum += (int)test_double_mixed(double_array, ARRAY_SIZE);
        
        total_sum += test_volatile_postinc(volatile_array, ARRAY_SIZE);
        total_sum += test_mixed_volatile(int_array, volatile_array, ARRAY_SIZE / 2);
        
        total_sum += (int)test_struct_traversal(struct_array, ARRAY_SIZE / 4);
        total_sum += test_struct_pointer_arithmetic(struct_array, ARRAY_SIZE / 4);
        
        total_sum += test_2d_array_single_pointer(matrix);
        total_sum += test_2d_array_nested(matrix);
        
        total_sum += test_constant_stride(int_array, ARRAY_SIZE, 4);
        test_mixed_patterns(int_array, volatile_array, ARRAY_SIZE / 2);
        
        total_sum += test_unrolled_loop(int_array, ARRAY_SIZE);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    printf("Array element 0: %d\n", int_array[0]);
    printf("Volatile element 0: %d\n", volatile_array[0]);
    
    return 0;
}
