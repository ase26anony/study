/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86-specific flags to exercise cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Use volatile to prevent compile-time optimization */
volatile int outer_limit = 1000;
volatile int inner_limit = 1000;
volatile int stride = 16;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 2000

/* Different data types to test various access patterns */
static int int_array1[LARGE_SIZE];
static int int_array2[LARGE_SIZE];
static double double_array1[MEDIUM_SIZE];
static double double_array2[MEDIUM_SIZE];
static char char_array[LARGE_SIZE * 4]; /* Larger byte array */

/* Matrix for multiplication-like operations */
#define MATRIX_SIZE 128
static int matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static int matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Initialize arrays with pseudo-random but deterministic values */
void initialize_arrays(void) {
    int i, j;
    
    /* Simple deterministic initialization */
    for (i = 0; i < LARGE_SIZE; i++) {
        int_array1[i] = i % 256;
        int_array2[i] = (i * 3) % 256;
    }
    
    for (i = 0; i < MEDIUM_SIZE; i++) {
        double_array1[i] = (double)(i % 100) / 3.0;
        double_array2[i] = (double)((i * 7) % 100) / 5.0;
    }
    
    for (i = 0; i < LARGE_SIZE * 4; i++) {
        char_array[i] = (char)(i % 128);
    }
    
    /* Initialize matrices */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) % 256;
            matrix_b[i][j] = (i * j) % 256;
            matrix_c[i][j] = 0;
        }
    }
}

/* Pattern 1: Matrix multiplication-like triple nested loop */
/* This pattern benefits from cache blocking optimizations */
int matrix_multiply_accumulate(void) {
    int i, j, k;
    int accum = 0;
    
    /* Classic matrix multiplication */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            int sum = 0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
            accum += sum;
        }
    }
    
    return accum;
}

/* Pattern 2: Non-unit stride access pattern */
/* Tests prefetching and cache line utilization */
double stride_access_pattern(void) {
    int i;
    double sum = 0.0;
    volatile int s = stride; /* Prevent constant propagation */
    
    /* Access every Nth element to test stride patterns */
    for (i = 0; i < MEDIUM_SIZE; i += s) {
        sum += double_array1[i] * double_array2[MEDIUM_SIZE - i - 1];
    }
    
    /* Also test backward stride */
    for (i = MEDIUM_SIZE - 1; i >= 0; i -= s) {
        sum += double_array2[i] * double_array1[MEDIUM_SIZE - i - 1];
    }
    
    return sum;
}

/* Pattern 3: Copy with potential cache line aliasing */
/* Tests cache line size optimizations */
void copy_with_aliasing(void) {
    int i, j;
    volatile int limit = inner_limit;
    
    /* Copy with offset that might cause cache line conflicts */
    for (i = 0; i < limit; i++) {
        for (j = 0; j < 64; j++) { /* 64-byte cache line sized inner loop */
            int idx = (i * 64 + j) % LARGE_SIZE;
            int_array2[idx] = int_array1[idx] + char_array[idx % (LARGE_SIZE * 4)];
        }
    }
}

/* Pattern 4: Mixed data type operations */
/* Tests various access sizes and alignments */
long mixed_data_operations(void) {
    int i;
    long total = 0;
    volatile int limit = outer_limit;
    
    for (i = 0; i < limit; i++) {
        /* Mix operations on different data types */
        int int_idx = i % LARGE_SIZE;
        int dbl_idx = i % MEDIUM_SIZE;
        int char_idx = i % (LARGE_SIZE * 4);
        
        /* Operations that might benefit from cache-aware optimizations */
        double temp = double_array1[dbl_idx] * 2.5;
        int_array1[int_idx] = (int)temp + char_array[char_idx];
        total += int_array1[int_idx] + (long)(double_array2[dbl_idx] * 100);
    }
    
    return total;
}

/* Pattern 5: Reduction operation on large array */
/* Tests vectorization and cache usage */
double reduction_operation(void) {
    int i;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    
    /* Multiple accumulators to break dependency chain */
    for (i = 0; i < MEDIUM_SIZE; i += 4) {
        sum1 += double_array1[i];
        sum2 += double_array1[i + 1];
        sum3 += double_array1[i + 2];
        sum1 += double_array1[i + 3];
    }
    
    return sum1 + sum2 + sum3;
}

int main(int argc, char *argv[]) {
    long total_result = 0;
    double fp_result = 0.0;
    int matrix_result = 0;
    
    /* Initialize with deterministic values */
    initialize_arrays();
    
    /* Execute different cache-sensitive patterns */
    matrix_result = matrix_multiply_accumulate();
    printf("Matrix accumulate result: %d\n", matrix_result);
    
    fp_result = stride_access_pattern();
    printf("Stride access result: %f\n", fp_result);
    
    copy_with_aliasing();
    printf("Copy with aliasing completed\n");
    
    total_result = mixed_data_operations();
    printf("Mixed operations result: %ld\n", total_result);
    
    fp_result = reduction_operation();
    printf("Reduction result: %f\n", fp_result);
    
    /* Final validation sum */
    {
        int i;
        long final_check = 0;
        for (i = 0; i < 1000; i++) {
            final_check += int_array1[i % LARGE_SIZE];
            final_check += (long)double_array1[i % MEDIUM_SIZE];
        }
        printf("Final validation: %ld\n", final_check);
    }
    
    return 0;
}

/* Conditional compilation for different x86 architectures */
/* This encourages testing with different -march flags */
#ifdef __x86_64__
/* x86-64 specific optimizations */
void x86_64_specific_operations(void) {
    /* Operations that might benefit from 64-bit specific optimizations */
    volatile long long large_sum = 0;
    int i;
    
    for (i = 0; i < 1000; i++) {
        large_sum += (long long)int_array1[i] * int_array2[i];
    }
    
    printf("x86-64 specific sum: %lld\n", large_sum);
}
#endif

#ifdef __i386__
/* i386 specific operations */
void i386_specific_operations(void) {
    /* 32-bit specific patterns */
    volatile int i;
    for (i = 0; i < 500; i++) {
        int_array1[i] = int_array1[i] * 3 - int_array2[i];
    }
}
#endif
