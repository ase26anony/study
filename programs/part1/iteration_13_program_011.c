/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86/x86-64 targets, covering specific cache descriptor values
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

/* Large arrays that exceed typical L1 cache sizes */
static int large_array1[ARRAY_SIZE];
static int large_array2[ARRAY_SIZE];
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static char char_buffer[ARRAY_SIZE * 4];

/* Function prototypes to force different optimization contexts */
void matrix_multiply(int size, double (*a)[MATRIX_SIZE], 
                     double (*b)[MATRIX_SIZE], double (*c)[MATRIX_SIZE]);
void stride_access(int *array, int size, int stride);
void cache_line_test(char *buffer, int size);
int compute_checksum(int *array, int size);

int main(int argc, char *argv[]) {
    int i, j, result = 0;
    int limit = outer_limit;
    int dim = matrix_dim;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < limit; i++) {
        large_array1[i] = rand() % 100;
        large_array2[i] = rand() % 100;
    }
    
    /* Initialize matrices */
    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            matrix_a[i][j] = (double)(rand() % 100) / 10.0;
            matrix_b[i][j] = (double)(rand() % 100) / 10.0;
        }
    }
    
    /* Pattern 1: Matrix multiplication (triple nested loop) */
    matrix_multiply(dim, matrix_a, matrix_b, matrix_c);
    
    /* Pattern 2: Non-unit stride access */
    stride_access(large_array1, limit, 7);  /* Prime number stride */
    stride_access(large_array1, limit, 16); /* Power of two stride */
    stride_access(large_array1, limit, 13); /* Another prime stride */
    
    /* Pattern 3: Cache line aliasing test */
    cache_line_test(char_buffer, limit * 4);
    
    /* Pattern 4: Copy between arrays with potential cache conflicts */
    for (i = 0; i < limit; i += 8) {
        /* Unrolled copy with offset to potentially cause cache line conflicts */
        large_array2[i] = large_array1[i];
        large_array2[i + 1] = large_array1[i + 1];
        large_array2[i + 2] = large_array1[i + 2];
        large_array2[i + 3] = large_array1[i + 3];
        large_array2[i + 4] = large_array1[i + 4];
        large_array2[i + 5] = large_array1[i + 5];
        large_array2[i + 6] = large_array1[i + 6];
        large_array2[i + 7] = large_array1[i + 7];
    }
    
    /* Pattern 5: Mixed data type operations */
    for (i = 0; i < limit; i++) {
        /* Mix int and double operations */
        double temp = (double)large_array1[i];
        temp = temp * 1.5;
        large_array2[i] = (int)temp;
        
        /* Also use char buffer */
        char_buffer[i % (limit * 4)] = (char)(large_array1[i] % 256);
    }
    
    /* Compute final result to prevent dead code elimination */
    result = compute_checksum(large_array1, limit);
    result += compute_checksum((int*)matrix_c, dim * dim);
    
    printf("Result: %d\n", result);
    
    /* Conditional compilation for different architectures */
#ifdef __x86_64__
    /* x86-64 specific code path */
    printf("x86-64 architecture detected\n");
    
    /* Additional x86-64 specific operations */
    unsigned long long big_array[5000];
    for (i = 0; i < 5000; i++) {
        big_array[i] = (unsigned long long)large_array1[i % limit];
    }
    
    /* Another cache-intensive pattern for x86-64 */
    for (i = 0; i < 4999; i++) {
        big_array[i] = big_array[i] * big_array[i + 1];
    }
#endif
    
#ifdef __i386__
    /* i386 specific code path */
    printf("i386 architecture detected\n");
    
    /* Different access pattern for 32-bit */
    for (i = 0; i < limit; i += 4) {
        int sum = large_array1[i] + large_array1[i + 1] + 
                  large_array1[i + 2] + large_array1[i + 3];
        large_array2[i] = sum / 4;
    }
#endif
    
    return result % 1000;
}

/* Matrix multiplication - classic cache-sensitive algorithm */
void matrix_multiply(int size, double (*a)[MATRIX_SIZE], 
                     double (*b)[MATRIX_SIZE], double (*c)[MATRIX_SIZE]) {
    int i, j, k;
    volatile int n = size;  /* Prevent optimization */
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

/* Access array with different strides to test prefetching */
void stride_access(int *array, int size, int stride) {
    int i, sum = 0;
    volatile int s = stride;
    
    for (i = 0; i < size; i += s) {
        sum += array[i];
        /* Prevent compiler from optimizing away the access */
        array[i] = sum % 100;
    }
    
    /* Use the result to prevent dead code elimination */
    if (sum < 0) {
        printf("Impossible\n");
    }
}

/* Test cache line behavior with byte accesses */
void cache_line_test(char *buffer, int size) {
    int i, j;
    volatile int block = 64;  /* Typical cache line size */
    
    /* Fill buffer */
    for (i = 0; i < size; i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Access in cache-line sized blocks with offsets */
    for (j = 0; j < 4; j++) {
        for (i = j; i < size; i += block) {
            buffer[i] = buffer[i] + 1;
        }
    }
}

/* Compute checksum to ensure all computations are used */
int compute_checksum(int *array, int size) {
    int i, sum = 0;
    for (i = 0; i < size; i++) {
        sum = (sum * 31 + array[i]) % 1000000;
    }
    return sum;
}

/* Additional function with different access pattern */
void alternate_pattern(void) {
    static int alt_array[5000];
    int i, j;
    
    /* Create a different access pattern */
    for (i = 0; i < 5000; i++) {
        alt_array[i] = i;
    }
    
    /* Reverse and process */
    for (i = 0, j = 4999; i < 2500; i++, j--) {
        int temp = alt_array[i];
        alt_array[i] = alt_array[j];
        alt_array[j] = temp;
    }
}
