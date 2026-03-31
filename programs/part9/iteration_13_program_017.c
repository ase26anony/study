/* auto_inc_dec_coverage.c
 * Comprehensive test for GCC auto-increment/decrement optimization coverage
 * Targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop patterns */
#define NOINLINE __attribute__((noinline, noipa))

/* Target architecture attributes for ARM */
#ifdef __ARM_ARCH
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* ========== INTEGER OPERATIONS ========== */

NOINLINE TARGET_ARM
int test_int_postinc_load(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Simple post-increment load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr++;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_int_postinc_store(int *arr, int value) {
    volatile int *vptr = arr;
    
    /* Pattern 1: Post-increment store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-increment */
    int *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - i;
    }
}

NOINLINE TARGET_ARM
int test_int_postdec_load(int *arr) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    int sum = 0;
    
    /* Pattern 1: Post-decrement load with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr--;
    }
    
    /* Pattern 2: Non-volatile pointer with post-decrement */
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr--;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_int_postdec_store(int *arr, int value) {
    volatile int *vptr = &arr[ARRAY_SIZE - 1];
    
    /* Pattern 1: Post-decrement store with volatile */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr-- = value + i;
    }
    
    /* Pattern 2: Non-volatile pointer with post-decrement */
    int *ptr = &arr[ARRAY_SIZE - 1];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr-- = value - i;
    }
}

/* ========== FLOAT OPERATIONS ========== */

NOINLINE TARGET_ARM
float test_float_postinc_load(float *arr) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    /* Pattern with constant stride */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *vptr;
        vptr = vptr + 1;  /* Different pattern: ptr = ptr + 1 */
    }
    
    /* Classic post-increment */
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_float_postinc_store(float *arr, float value) {
    volatile float *vptr = arr;
    
    /* Mixed offset patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *(vptr + 0) = value;  /* Constant zero offset */
        vptr++;
    }
    
    float *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value * i;
    }
}

/* ========== DOUBLE OPERATIONS ========== */

NOINLINE TARGET_ARM
double test_double_postinc_load(double *arr) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    /* Pattern with pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr + 0);  /* Explicit zero offset */
        vptr = &vptr[1];     /* Array indexing pattern */
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_double_postinc_store(double *arr, double value) {
    volatile double *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *vptr++ = value + sin(i * 0.1);
    }
    
    double *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        *ptr++ = value - cos(i * 0.1);
    }
}

/* ========== STRUCTURE OPERATIONS ========== */

typedef struct {
    int id;
    float value;
    double data;
    char tag[4];
} TestStruct;

NOINLINE TARGET_ARM
double test_struct_traversal(TestStruct *arr) {
    volatile TestStruct *vptr = arr;
    double sum = 0.0;
    
    /* Access different struct members with post-increment */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += vptr->id;
        sum += vptr->value;
        sum += vptr->data;
        vptr++;  /* Large constant offset (sizeof(TestStruct)) */
    }
    
    /* Access single member with post-increment */
    TestStruct *ptr = arr;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += ptr->data;
        ptr++;
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_struct_store(TestStruct *arr) {
    volatile TestStruct *vptr = arr;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        vptr->id = i;
        vptr->value = i * 0.5f;
        vptr->data = i * 1.5;
        memcpy(vptr->tag, "TAG", 4);
        vptr++;
    }
}

/* ========== MULTI-DIMENSIONAL ARRAYS ========== */

#define ROWS 16
#define COLS 16

NOINLINE TARGET_ARM
int test_2d_array_traversal(int arr[ROWS][COLS]) {
    volatile int *vptr = &arr[0][0];
    int sum = 0;
    
    /* Traverse 2D array as 1D with pointer */
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += *vptr++;
    }
    
    /* Nested loops with pointer reset */
    for (int i = 0; i < ROWS; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < COLS; j++) {
            sum += *ptr++;
        }
    }
    
    return sum;
}

NOINLINE TARGET_ARM
void test_2d_array_store(int arr[ROWS][COLS]) {
    volatile int *vptr = &arr[0][0];
    
    for (int i = 0; i < ROWS * COLS; i++) {
        *vptr++ = i * 2;
    }
    
    for (int i = 0; i < ROWS; i++) {
        int *ptr = arr[i];
        for (int j = 0; j < COLS; j++) {
            *ptr++ = i * COLS + j;
        }
    }
}

/* ========== MIXED PATTERNS WITH DIFFERENT OFFSETS ========== */

NOINLINE TARGET_ARM
int test_mixed_offset_patterns(int *arr) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern 1: Base + 0 offset (should trigger uncovered lines) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += *(vptr + 0);
        vptr++;
    }
    
    /* Pattern 2: Base + small constant offset */
    vptr = arr;
    for (int i = 0; i < ARRAY_SIZE - 4; i++) {
        sum += *(vptr + 4);
        vptr++;
    }
    
    /* Pattern 3: Array indexing that should become pointer arithmetic */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    TestStruct *struct_arr = (TestStruct*)malloc(ARRAY_SIZE * sizeof(TestStruct));
    int multi_arr[ROWS][COLS];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i + 1;
        float_arr[i] = (i + 1) * 0.1f;
        double_arr[i] = (i + 1) * 0.01;
        struct_arr[i].id = i;
        struct_arr[i].value = i * 0.5f;
        struct_arr[i].data = i * 1.5;
        memcpy(struct_arr[i].tag, "TAG", 4);
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            multi_arr[i][j] = i * COLS + j;
        }
    }
    
    int total_checksum = 0;
    
    /* Run tests multiple times to ensure patterns are exercised */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Integer tests */
        total_checksum += test_int_postinc_load(int_arr);
        test_int_postinc_store(int_arr, iter);
        total_checksum += test_int_postdec_load(int_arr);
        test_int_postdec_store(int_arr, iter);
        
        /* Float tests */
        total_checksum += (int)test_float_postinc_load(float_arr);
        test_float_postinc_store(float_arr, iter * 0.5f);
        
        /* Double tests */
        total_checksum += (int)test_double_postinc_load(double_arr);
        test_double_postinc_store(double_arr, iter * 0.25);
        
        /* Struct tests */
        total_checksum += (int)test_struct_traversal(struct_arr);
        test_struct_store(struct_arr);
        
        /* Multi-dimensional tests */
        total_checksum += test_2d_array_traversal(multi_arr);
        test_2d_array_store(multi_arr);
        
        /* Mixed pattern tests */
        total_checksum += test_mixed_offset_patterns(int_arr);
    }
    
    /* Verify some results to prevent dead code elimination */
    printf("Final checksum: %d\n", total_checksum);
    printf("Sample values:\n");
    printf("  int_arr[0] = %d\n", int_arr[0]);
    printf("  float_arr[0] = %f\n", float_arr[0]);
    printf("  double_arr[0] = %f\n", double_arr[0]);
    printf("  struct_arr[0].id = %d\n", struct_arr[0].id);
    printf("  multi_arr[0][0] = %d\n", multi_arr[0][0]);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(struct_arr);
    
    return 0;
}
