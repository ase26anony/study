/* test_auto_inc_dec.c
 * Comprehensive test for GCC auto-increment/decrement optimization
 * Targets specific RTL patterns in auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to preserve loop structure */
#define NOINLINE __attribute__((noinline,noipa))

/* Structure for testing non-trivial offsets */
typedef struct {
    int val;
    float fval;
    double dval;
    char padding[16];  /* Force larger stride */
} TestStruct;

/* Global arrays to prevent constant propagation */
int global_int_array[ARRAY_SIZE];
float global_float_array[ARRAY_SIZE];
double global_double_array[ARRAY_SIZE];
TestStruct global_struct_array[ARRAY_SIZE];

/* ========== INTEGER TESTS ========== */

NOINLINE void test_int_postinc_load(int *arr, int n) {
    volatile int *vptr = arr;
    int sum = 0;
    
    /* Pattern: post-increment load with volatile */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;  /* Should generate mem_loc with reg0 = vptr, offset 0 */
    }
    
    /* Use result to prevent elimination */
    global_int_array[0] = sum;
}

NOINLINE void test_int_postinc_store(int *arr, int n, int value) {
    int *ptr = arr;
    
    /* Pattern: post-increment store */
    for (int i = 0; i < n; i++) {
        *ptr++ = value + i;  /* Store with post-increment */
    }
}

NOINLINE void test_int_postdec_load(int *arr, int n) {
    volatile int *vptr = &arr[n-1];
    int sum = 0;
    
    /* Pattern: post-decrement load */
    for (int i = 0; i < n; i++) {
        sum += *vptr--;  /* Load with post-decrement */
    }
    
    global_int_array[1] = sum;
}

NOINLINE void test_int_postdec_store(int *arr, int n, int value) {
    int *ptr = &arr[n-1];
    
    /* Pattern: post-decrement store */
    for (int i = 0; i < n; i++) {
        *ptr-- = value - i;  /* Store with post-decrement */
    }
}

NOINLINE void test_int_pointer_arithmetic(int *arr, int n) {
    int *ptr = arr;
    int sum = 0;
    
    /* Pattern: pointer arithmetic with constant stride */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 4);  /* Constant offset, ptr increments in loop */
        ptr += 4;           /* Explicit increment */
        if (ptr >= &arr[n]) break;
    }
    
    global_int_array[2] = sum;
}

/* ========== FLOAT TESTS ========== */

NOINLINE void test_float_postinc_load(float *arr, int n) {
    volatile float *vptr = arr;
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;  /* Float post-increment load */
    }
    
    global_float_array[0] = sum;
}

NOINLINE void test_float_postinc_store(float *arr, int n, float value) {
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (float)i;  /* Float post-increment store */
    }
}

/* ========== DOUBLE TESTS ========== */

NOINLINE void test_double_postinc_load(double *arr, int n) {
    volatile double *vptr = arr;
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum += *vptr++;  /* Double post-increment load */
    }
    
    global_double_array[0] = sum;
}

NOINLINE void test_double_postinc_store(double *arr, int n, double value) {
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        *ptr++ = value + (double)i;  /* Double post-increment store */
    }
}

/* ========== STRUCT TESTS ========== */

NOINLINE void test_struct_member_access(TestStruct *arr, int n) {
    volatile TestStruct *vptr = arr;
    int sum = 0;
    
    /* Access struct member with post-increment */
    for (int i = 0; i < n; i++) {
        sum += vptr->val;  /* Access member, then increment */
        vptr++;            /* Larger stride due to struct size */
    }
    
    global_int_array[3] = sum;
}

NOINLINE void test_struct_pointer_arithmetic(TestStruct *arr, int n) {
    TestStruct *ptr = arr;
    float sum = 0.0f;
    
    /* Access float member with pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += ptr->fval;  /* Access float member */
        ptr++;             /* Non-one stride */
    }
    
    global_float_array[1] = sum;
}

/* ========== MULTI-DIMENSIONAL TESTS ========== */

NOINLINE void test_2d_array_traversal(int arr2d[16][16]) {
    int *ptr = &arr2d[0][0];
    int sum = 0;
    
    /* Traverse 2D array as 1D with single pointer */
    for (int i = 0; i < 16 * 16; i++) {
        sum += *ptr++;  /* Row-major traversal */
    }
    
    global_int_array[4] = sum;
}

NOINLINE void test_nested_loop_reset(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Nested loop with pointer reset each iteration */
    for (int r = 0; r < rows; r++) {
        volatile int *vptr = &arr[r * cols];
        
        /* Inner loop should trigger auto-inc recognition */
        for (int c = 0; c < cols; c++) {
            sum += *vptr++;  /* Pointer reset each outer iteration */
        }
    }
    
    global_int_array[5] = sum;
}

/* ========== MIXED PATTERNS ========== */

NOINLINE void test_mixed_increment_patterns(int *arr, int n) {
    volatile int *vptr1 = arr;
    int *ptr2 = &arr[n/2];
    int sum = 0;
    
    /* Mix of increment patterns in same function */
    for (int i = 0; i < n/2; i++) {
        sum += *vptr1++;  /* Volatile post-inc */
        *ptr2++ = sum;    /* Non-volatile post-inc store */
    }
    
    global_int_array[6] = sum;
}

/* ========== MAIN DRIVER ========== */

int main() {
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = i + 1;
        global_float_array[i] = (float)(i + 1) * 1.5f;
        global_double_array[i] = (double)(i + 1) * 2.5;
        global_struct_array[i].val = i * 3;
        global_struct_array[i].fval = (float)i * 4.0f;
        global_struct_array[i].dval = (double)i * 5.0;
    }
    
    int local_int_array[ARRAY_SIZE];
    float local_float_array[ARRAY_SIZE];
    double local_double_array[ARRAY_SIZE];
    TestStruct local_struct_array[ARRAY_SIZE];
    int arr2d[16][16];
    
    /* Initialize local arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        local_int_array[i] = i * 2;
        local_float_array[i] = (float)i * 3.0f;
        local_double_array[i] = (double)i * 4.0;
        local_struct_array[i] = global_struct_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            arr2d[i][j] = i * 16 + j;
        }
    }
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_postinc_load(local_int_array, ARRAY_SIZE);
        test_int_postinc_store(local_int_array, ARRAY_SIZE, iter);
        test_int_postdec_load(local_int_array, ARRAY_SIZE);
        test_int_postdec_store(local_int_array, ARRAY_SIZE, iter);
        test_int_pointer_arithmetic(local_int_array, ARRAY_SIZE/4);
        
        test_float_postinc_load(local_float_array, ARRAY_SIZE);
        test_float_postinc_store(local_float_array, ARRAY_SIZE, (float)iter);
        
        test_double_postinc_load(local_double_array, ARRAY_SIZE);
        test_double_postinc_store(local_double_array, ARRAY_SIZE, (double)iter);
        
        test_struct_member_access(local_struct_array, ARRAY_SIZE);
        test_struct_pointer_arithmetic(local_struct_array, ARRAY_SIZE);
        
        test_2d_array_traversal(arr2d);
        test_nested_loop_reset(local_int_array, 8, 32);
        test_mixed_increment_patterns(local_int_array, ARRAY_SIZE);
    }
    
    /* Verify results to ensure correctness */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += global_int_array[i % 10];  /* Use modulo to avoid overflow */
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    printf("Sample values: %d, %f, %lf\n", 
           global_int_array[0], 
           global_float_array[0], 
           global_double_array[0]);
    
    return checksum != 0 ? 0 : 1;
}
