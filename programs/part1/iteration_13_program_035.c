/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86 -march and -mtune options to exercise
 * cache descriptor switch cases in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache-aware optimizations */
#define ARRAY_SIZE 10000
#define MATRIX_SIZE 256

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = ARRAY_SIZE;
volatile int matrix_dim = MATRIX_SIZE;

/* Different data types to exercise various cache line patterns */
static int int_array1[ARRAY_SIZE];
static int int_array2[ARRAY_SIZE];
static double double_array1[ARRAY_SIZE/2];
static double double_array2[ARRAY_SIZE/2];
static char char_array[ARRAY_SIZE * 4];

/* Matrix for multiplication test */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes to force different optimization contexts */
void matrix_multiply(int size);
void stride_access_pattern(int stride);
void cache_line_aliasing_test(void);
void mixed_data_type_operations(void);

int main(int argc, char **argv) {
    int i, j;
    double result = 0.0;
    clock_t start, end;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array1[i] = rand() % 100;
        int_array2[i] = rand() % 100;
    }
    
    for (i = 0; i < ARRAY_SIZE/2; i++) {
        double_array1[i] = (double)rand() / RAND_MAX;
        double_array2[i] = (double)rand() / RAND_MAX;
    }
    
    for (i = 0; i < ARRAY_SIZE * 4; i++) {
        char_array[i] = (char)(rand() % 256);
    }
    
    /* Initialize matrices */
    for (i = 0; i < matrix_dim; i++) {
        for (j = 0; j < matrix_dim; j++) {
            matrix_a[i][j] = (double)rand() / RAND_MAX;
            matrix_b[i][j] = (double)rand() / RAND_MAX;
            matrix_c[i][j] = 0.0;
        }
    }
    
    printf("Starting cache-sensitive computations...\n");
    start = clock();
    
    /* Execute different cache-sensitive patterns */
    
    /* Pattern 1: Matrix multiplication (triple nested loop) */
    matrix_multiply(matrix_dim);
    
    /* Pattern 2: Non-unit stride access */
    stride_access_pattern(17);  /* Prime number stride */
    stride_access_pattern(32);  /* Power of two stride */
    stride_access_pattern(63);  /* Cache line boundary testing */
    
    /* Pattern 3: Cache line aliasing */
    cache_line_aliasing_test();
    
    /* Pattern 4: Mixed data type operations */
    mixed_data_type_operations();
    
    /* Pattern 5: Blocked matrix multiplication (cache-aware) */
    int block_size = 32;  /* Typical cache block size */
    for (int ii = 0; ii < matrix_dim; ii += block_size) {
        for (int jj = 0; jj < matrix_dim; jj += block_size) {
            for (int kk = 0; kk < matrix_dim; kk += block_size) {
                for (int i = ii; i < ii + block_size && i < matrix_dim; i++) {
                    for (int j = jj; j < jj + block_size && j < matrix_dim; j++) {
                        for (int k = kk; k < kk + block_size && k < matrix_dim; k++) {
                            matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
    
    /* Calculate final result to prevent dead code elimination */
    for (i = 0; i < matrix_dim; i++) {
        for (j = 0; j < matrix_dim; j++) {
            result += matrix_c[i][j];
        }
    }
    
    /* Additional integer array operations */
    long long int_sum = 0;
    for (i = 0; i < outer_limit; i += 8) {
        /* Unrolled loop with multiple array accesses */
        int_sum += int_array1[i] * int_array2[i];
        int_sum += int_array1[i+1] * int_array2[i+1];
        int_sum += int_array1[i+2] * int_array2[i+2];
        int_sum += int_array1[i+3] * int_array2[i+3];
        int_sum += int_array1[i+4] * int_array2[i+4];
        int_sum += int_array1[i+5] * int_array2[i+5];
        int_sum += int_array1[i+6] * int_array2[i+6];
        int_sum += int_array1[i+7] * int_array2[i+7];
    }
    
    end = clock();
    
    printf("Final results:\n");
    printf("  Matrix sum: %f\n", result);
    printf("  Integer sum: %lld\n", int_sum);
    printf("  Time elapsed: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Architecture-specific code blocks to encourage multiple compilations */
#ifdef __x86_64__
    /* x86-64 specific optimizations */
    printf("x86-64 architecture detected\n");
    
    /* Use some x86-64 specific operations */
    unsigned long long rdtsc_val;
    asm volatile ("rdtsc" : "=A" (rdtsc_val));
    printf("RDTSC value: %llu\n", rdtsc_val);
#endif
    
#ifdef __i386__
    /* i386 specific code path */
    printf("i386 architecture detected\n");
#endif
    
    return 0;
}

void matrix_multiply(int size) {
    int i, j, k;
    
    /* Classic matrix multiplication - highly cache sensitive */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            double sum = 0.0;
            for (k = 0; k < size; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

void stride_access_pattern(int stride) {
    int i;
    double sum = 0.0;
    
    /* Access array with non-unit stride */
    for (i = 0; i < ARRAY_SIZE/2; i += stride) {
        sum += double_array1[i] * double_array2[i];
    }
    
    /* Prevent dead code elimination */
    if (sum < 0) {
        printf("Impossible condition\n");
    }
}

void cache_line_aliasing_test(void) {
    int i;
    
    /* Copy between arrays with potential cache line conflicts */
    for (i = 0; i < ARRAY_SIZE - 64; i++) {
        int_array2[i] = int_array1[i] + int_array1[i + 64];
    }
    
    /* Reverse copy to test write-back behavior */
    for (i = ARRAY_SIZE - 1; i >= 64; i--) {
        int_array1[i] = int_array2[i] + int_array2[i - 64];
    }
}

void mixed_data_type_operations(void) {
    int i;
    double float_sum = 0.0;
    int int_sum = 0;
    
    /* Mix operations on different data types in same loop */
    for (i = 0; i < ARRAY_SIZE/2; i++) {
        float_sum += double_array1[i];
        int_sum += int_array1[i * 2];
        
        /* Conditional that depends on both data types */
        if (float_sum > int_sum) {
            char_array[i] = (char)(int_sum % 256);
        } else {
            char_array[i + ARRAY_SIZE/2] = (char)(float_sum);
        }
    }
}

/* Additional function with different access pattern */
void transposed_access(void) {
    int i, j;
    
    /* Access matrix in column-major order (poor locality) */
    for (j = 0; j < matrix_dim; j++) {
        for (i = 0; i < matrix_dim; i++) {
            matrix_c[i][j] = matrix_a[j][i] + matrix_b[i][j];
        }
    }
}
