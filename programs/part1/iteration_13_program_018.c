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
volatile int inner_limit = 500;

/* Large arrays that exceed typical L1 cache sizes */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

static int array1[LARGE_SIZE];
static double array2[MEDIUM_SIZE];
static char array3[LARGE_SIZE];
static int matrix_a[SMALL_SIZE][SMALL_SIZE];
static int matrix_b[SMALL_SIZE][SMALL_SIZE];
static int matrix_c[SMALL_SIZE][SMALL_SIZE];

/* Function prototypes to prevent inlining */
int __attribute__((noinline)) process_matrix(int size);
double __attribute__((noinline)) stride_access(int stride, int iterations);
void __attribute__((noinline)) cache_line_test(int size);

int main(int argc, char *argv[]) {
    int i, j, k;
    double result = 0.0;
    int matrix_result = 0;
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < LARGE_SIZE; i++) {
        array1[i] = rand() % 100;
        array3[i] = (char)(rand() % 256);
    }
    
    for (i = 0; i < MEDIUM_SIZE; i++) {
        array2[i] = (double)rand() / RAND_MAX;
    }
    
    /* Initialize matrices */
    for (i = 0; i < SMALL_SIZE; i++) {
        for (j = 0; j < SMALL_SIZE; j++) {
            matrix_a[i][j] = rand() % 100;
            matrix_b[i][j] = rand() % 100;
            matrix_c[i][j] = 0;
        }
    }
    
    /* Test 1: Matrix multiplication-like triple nested loop
     * This benefits from cache-aware optimizations */
    printf("Starting matrix multiplication test...\n");
    for (i = 0; i < outer_limit % SMALL_SIZE; i++) {
        for (j = 0; j < inner_limit % SMALL_SIZE; j++) {
            int sum = 0;
            for (k = 0; k < SMALL_SIZE; k++) {
                /* Use volatile index to prevent optimization */
                volatile int idx_a = i * SMALL_SIZE + k;
                volatile int idx_b = k * SMALL_SIZE + j;
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
            matrix_result += sum;
        }
    }
    
    /* Test 2: Non-unit stride access pattern
     * Forces compiler to consider cache line effects */
    printf("Starting stride access test...\n");
    for (i = 0; i < LARGE_SIZE; i += 7) {  /* Non-power-of-two stride */
        array1[i] = array1[i] * 2 + 1;
        result += (double)array1[i];
    }
    
    /* Test 3: Array copy with potential cache line aliasing */
    printf("Starting array copy test...\n");
    for (i = 0; i < LARGE_SIZE - 64; i++) {
        /* Copy with offset that might cause cache conflicts */
        array3[i + 32] = array3[i];
    }
    
    /* Test 4: Double precision floating point operations
     * Different access pattern for FPU optimization */
    printf("Starting floating point test...\n");
    for (i = 0; i < MEDIUM_SIZE; i++) {
        for (j = 0; j < 10; j++) {
            array2[i] = array2[i] * 1.01 + (double)j;
        }
        result += array2[i];
    }
    
    /* Test 5: Mixed data type access pattern */
    printf("Starting mixed data type test...\n");
    for (i = 0; i < LARGE_SIZE; i++) {
        if (i % 2 == 0) {
            array1[i] = (int)(array2[i % MEDIUM_SIZE] * 100);
        } else {
            array3[i] = (char)(array1[i] % 256);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Matrix result: %d\n", matrix_result);
    printf("Floating point result: %f\n", result);
    printf("Final array1[100] = %d\n", array1[100]);
    printf("Final array3[200] = %d\n", (int)array3[200]);
    
    /* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
    printf("Compiled for x86_64 architecture\n");
    
    /* Additional architecture-specific tests */
    #ifdef __AVX__
    printf("AVX instructions available\n");
    /* AVX-optimized loop */
    for (i = 0; i < LARGE_SIZE; i += 8) {
        array1[i] *= 2;
    }
    #endif
    
    #ifdef __SSE4_2__
    printf("SSE4.2 instructions available\n");
    #endif
#endif

#ifdef __i386__
    printf("Compiled for i386 architecture\n");
    /* 32-bit specific optimizations */
    for (i = 0; i < LARGE_SIZE; i += 4) {
        array1[i] = array1[i] / 2;
    }
#endif

    return 0;
}

/* Additional functions to create more complex control flow */
int __attribute__((noinline)) process_matrix(int size) {
    int i, j, total = 0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            total += matrix_a[i][j] + matrix_b[j][i];
        }
    }
    return total;
}

double __attribute__((noinline)) stride_access(int stride, int iterations) {
    double sum = 0.0;
    int i;
    for (i = 0; i < iterations; i += stride) {
        sum += array2[i % MEDIUM_SIZE];
    }
    return sum;
}

void __attribute__((noinline)) cache_line_test(int size) {
    int i, j;
    /* Access pattern designed to test cache line size detection */
    for (i = 0; i < size; i++) {
        for (j = 0; j < 64; j++) {  /* Typical cache line size */
            array1[(i * 64 + j) % LARGE_SIZE] += j;
        }
    }
}
