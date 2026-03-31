/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 256
#define CHECKSUM_SEED 0xDEADBEEF

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific attributes for different architectures */
#ifdef __arm__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

#ifdef __powerpc__
#define TARGET_PPC __attribute__((target("cpu=powerpc")))
#else
#define TARGET_PPC
#endif

/* ========== INTEGER ARRAY TESTS ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postinc_store(int *arr, int n, int value) {
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

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n - 1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
int test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n - 1];
    
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

/* Test with volatile to prevent excessive optimization */
NOINLINE TARGET_ARM TARGET_PPC
int test_int_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *vptr = arr;
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    return sum;
}

/* Test with constant stride pointer arithmetic */
NOINLINE TARGET_ARM TARGET_PPC
int test_int_stride4_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 4th element with pointer arithmetic */
    for (int i = 0; i < n/4; i++) {
        sum += *ptr;
        ptr += 4;  /* Constant stride */
    }
    return sum;
}

/* ========== FLOATING POINT TESTS ========== */

NOINLINE TARGET_ARM TARGET_PPC
float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
float test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (float)i;
    }
    
    float sum = 0.0f;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== DOUBLE TESTS ========== */

NOINLINE TARGET_ARM TARGET_PPC
double test_double_postinc_load(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
double test_double_postdec_store(double *arr, int n, double value) {
    double *ptr = &arr[n - 1];
    
    for (int i = 0; i < n; i++) {
        *ptr-- = value - (double)i;
    }
    
    double sum = 0.0;
    ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* ========== STRUCTURE ARRAY TESTS ========== */

typedef struct {
    int a;
    float b;
    double c;
    char d[8];
} TestStruct;

NOINLINE TARGET_ARM TARGET_PPC
double test_struct_traversal(TestStruct *arr, int n) {
    double sum = 0.0;
    TestStruct *ptr = arr;
    
    /* Access struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b + ptr->c;
        ptr++;  /* Large stride (sizeof(TestStruct)) */
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
void test_struct_member_store(TestStruct *arr, int n, int base) {
    TestStruct *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->a = base + i;
        ptr->b = (float)(base + i) * 0.5f;
        ptr->c = (double)(base + i) * 0.25;
        snprintf(ptr->d, sizeof(ptr->d), "%d", base + i);
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAY TESTS ========== */

#define ROWS 16
#define COLS 16

NOINLINE TARGET_ARM TARGET_PPC
int test_2d_array_row_major(int arr[ROWS][COLS]) {
    int sum = 0;
    int *ptr = &arr[0][0];  /* Treat as 1D array */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE TARGET_ARM TARGET_PPC
int test_2d_array_nested_loops(int arr[ROWS][COLS]) {
    int sum = 0;
    
    /* Nested loops - inner loop pointer gets reset each iteration */
    for (int i = 0; i < ROWS; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < COLS; j++) {
            sum += *row_ptr++;
        }
    }
    return sum;
}

/* ========== MIXED PATTERNS IN SINGLE FUNCTION ========== */

NOINLINE TARGET_ARM TARGET_PPC
int test_mixed_patterns(int *arr1, float *arr2, double *arr3, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    int *ptr1 = arr1;
    float *ptr2 = arr2;
    double *ptr3 = arr3;
    
    /* Mixed pointer types with post-increment */
    for (int i = 0; i < n; i++) {
        sum_int += *ptr1++;
        sum_float += *ptr2++;
        sum_double += *ptr3++;
    }
    
    return (int)(sum_int + sum_float + sum_double);
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_array = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int matrix[ROWS][COLS];
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i + 1;
        float_array[i] = (float)(i + 1) * 1.5f;
        double_array[i] = (double)(i + 1) * 2.5;
        struct_array[i].a = i;
        struct_array[i].b = (float)i * 0.75f;
        struct_array[i].c = (double)i * 1.25;
        snprintf(struct_array[i].d, sizeof(struct_array[i].d), "val%d", i);
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j + 1;
        }
    }
    
    int checksum = CHECKSUM_SEED;
    
    /* Run all tests */
    checksum ^= test_int_postinc_load(int_array, ARRAY_SIZE);
    checksum ^= test_int_postinc_store(int_array, ARRAY_SIZE, 42);
    checksum ^= test_int_postdec_load(int_array, ARRAY_SIZE);
    checksum ^= test_int_postdec_store(int_array, ARRAY_SIZE, 100);
    checksum ^= test_int_volatile_postinc((volatile int*)int_array, ARRAY_SIZE);
    checksum ^= test_int_stride4_load(int_array, ARRAY_SIZE);
    
    checksum ^= (int)test_float_postinc_load(float_array, ARRAY_SIZE);
    checksum ^= (int)test_float_postinc_store(float_array, ARRAY_SIZE, 10.0f);
    
    checksum ^= (int)test_double_postinc_load(double_array, ARRAY_SIZE);
    checksum ^= (int)test_double_postdec_store(double_array, ARRAY_SIZE, 20.0);
    
    checksum ^= (int)test_struct_traversal(struct_array, ARRAY_SIZE);
    test_struct_member_store(struct_array, ARRAY_SIZE, 500);
    
    checksum ^= test_2d_array_row_major(matrix);
    checksum ^= test_2d_array_nested_loops(matrix);
    
    checksum ^= test_mixed_patterns(int_array, float_array, double_array, ARRAY_SIZE/2);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
