/* auto_inc_dec_test.c - Comprehensive test for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-unit strides */
struct Data {
    int val;
    float fval;
    double dval;
    char padding[32];  /* Force larger stride */
};

/* Global arrays to prevent constant propagation */
int global_int_array[1024];
float global_float_array[1024];
double global_double_array[1024];
struct Data global_struct_array[256];

/* ========== INTEGER TESTS ========== */

NOINLINE int test_int_postinc_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Pattern: *ptr++ in loop - should trigger auto-inc */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: *ptr++ = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;
    }
}

NOINLINE int test_int_postdec_load(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n-1];  /* Start from end */
    
    /* Pattern: *ptr-- in loop - should trigger auto-dec */
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: *ptr-- = value in loop */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;
    }
}

/* Test with volatile to prevent reordering */
NOINLINE int test_int_volatile_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Test with constant stride */
NOINLINE int test_int_stride4_load(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Access every 4th element */
    for (int i = 0; i < n/4; i++) {
        sum += *ptr;
        ptr += 4;
    }
    return sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE float test_float_postinc_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (float)i;
    }
}

NOINLINE float test_float_postdec_load(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

/* ========== DOUBLE TESTS ========== */

NOINLINE double test_double_postinc_load(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (double)i;
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE int test_struct_member_load(struct Data *arr, int n) {
    int sum = 0;
    struct Data *ptr = arr;
    
    /* Access struct member with non-unit stride */
    for (int i = 0; i < n; i++) {
        sum += ptr->val;
        ptr++;  /* Large stride due to struct size */
    }
    return sum;
}

NOINLINE void test_struct_member_store(struct Data *arr, int n, int value) {
    struct Data *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        ptr->val = value + i;
        ptr++;
    }
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE int test_2d_array_row_major(int arr[16][16], int rows, int cols) {
    int sum = 0;
    int *ptr = &arr[0][0];  /* Treat as 1D array */
    
    /* Row-major traversal with single pointer */
    for (int i = 0; i < rows * cols; i++) {
        sum += *ptr++;
    }
    return sum;
}

NOINLINE int test_nested_loop_pointer(int arr[8][8]) {
    int sum = 0;
    
    /* Nested loops with pointer reset each iteration */
    for (int i = 0; i < 8; i++) {
        int *ptr = arr[i];  /* Reset pointer each outer iteration */
        for (int j = 0; j < 8; j++) {
            sum += *ptr++;
        }
    }
    return sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr1, float *arr2, double *arr3, int n) {
    int *ip = arr1;
    float *fp = arr2;
    double *dp = arr3;
    
    /* Mixed types in same loop */
    for (int i = 0; i < n; i++) {
        *ip++ = i;
        *fp++ = (float)i * 0.5f;
        *dp++ = (double)i * 0.25;
    }
}

/* ========== ARCHITECTURE-SPECIFIC TESTS ========== */

#ifdef __ARM_ARCH
__attribute__((target("arch=armv7-a")))
#endif
NOINLINE int test_arm_optimized_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    /* Simple pattern that should work well on ARM */
    for (int i = 0; i < n; i += 4) {
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
        sum += *ptr++;
    }
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    const int INT_SIZE = 256;
    const int FLOAT_SIZE = 256;
    const int DOUBLE_SIZE = 256;
    const int STRUCT_SIZE = 64;
    
    /* Initialize arrays */
    for (int i = 0; i < INT_SIZE; i++) {
        global_int_array[i] = i;
    }
    
    for (int i = 0; i < FLOAT_SIZE; i++) {
        global_float_array[i] = i * 1.5f;
    }
    
    for (int i = 0; i < DOUBLE_SIZE; i++) {
        global_double_array[i] = i * 2.5;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        global_struct_array[i].val = i * 10;
        global_struct_array[i].fval = i * 3.14f;
        global_struct_array[i].dval = i * 6.28;
    }
    
    int result = 0;
    
    /* Run integer tests */
    result += test_int_postinc_load(global_int_array, INT_SIZE);
    test_int_postinc_store(global_int_array, INT_SIZE, 42);
    result += test_int_postdec_load(global_int_array, INT_SIZE);
    test_int_postdec_store(global_int_array, INT_SIZE, 100);
    result += test_int_volatile_postinc(global_int_array, INT_SIZE);
    result += test_int_stride4_load(global_int_array, INT_SIZE);
    
    /* Run float tests */
    float fresult = test_float_postinc_load(global_float_array, FLOAT_SIZE);
    test_float_postinc_store(global_float_array, FLOAT_SIZE, 10.5f);
    fresult += test_float_postdec_load(global_float_array, FLOAT_SIZE);
    result += (int)fresult;
    
    /* Run double tests */
    double dresult = test_double_postinc_load(global_double_array, DOUBLE_SIZE);
    test_double_postinc_store(global_double_array, DOUBLE_SIZE, 20.25);
    result += (int)dresult;
    
    /* Run struct tests */
    result += test_struct_member_load(global_struct_array, STRUCT_SIZE);
    test_struct_member_store(global_struct_array, STRUCT_SIZE, 500);
    
    /* 2D array test */
    int arr_2d[16][16];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr_2d[i][j] = i * 16 + j;
        }
    }
    result += test_2d_array_row_major(arr_2d, 16, 16);
    result += test_nested_loop_pointer(arr_2d);
    
    /* Mixed pattern test */
    test_mixed_increment_patterns(global_int_array, global_float_array, 
                                  global_double_array, 64);
    
    /* Architecture-specific test */
    result += test_arm_optimized_postinc(global_int_array, INT_SIZE);
    
    /* Verify results aren't optimized away */
    printf("Result checksum: %d\n", result);
    printf("Float result: %f\n", fresult);
    printf("Double result: %f\n", dresult);
    
    /* Additional verification */
    volatile int verify = 0;
    for (int i = 0; i < INT_SIZE; i++) {
        verify += global_int_array[i];
    }
    printf("Array sum: %d\n", verify);
    
    return 0;
}
