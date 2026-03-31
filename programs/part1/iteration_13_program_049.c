/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 
 * 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60, 0x66-0x68, 
 * 0x78-0x80, 0x82-0x87
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
static int matrix_a[LARGE_SIZE][LARGE_SIZE/100];
static int matrix_b[LARGE_SIZE/100][LARGE_SIZE];
static int matrix_c[LARGE_SIZE/10][LARGE_SIZE/10];
static double double_array[5000][500];
static char char_array[20000][100];

/* Function prototypes to force different optimization contexts */
void matrix_multiply_optimized(int n, int m, int p);
void stride_access_pattern(int size, int step);
void cache_line_aliasing_test(int iterations);
void mixed_data_type_operations(void);

/* Matrix multiplication - triple nested loop */
void matrix_multiply_optimized(int n, int m, int p) {
    volatile int i, j, k;
    int sum;
    
    /* Force compiler to consider cache blocking */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum = 0;
            for (k = 0; k < p; k++) {
                /* Access with different strides */
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int step) {
    volatile int i;
    long long accumulator = 0;
    
    /* Access every 'step' element - tests cache line utilization */
    for (i = 0; i < size; i += step) {
        accumulator += double_array[i % 5000][i % 500];
        accumulator += char_array[i % 20000][i % 100] * 3;
    }
    
    /* Prevent dead code elimination */
    if (accumulator > 1000000) {
        printf("Stride pattern result: %lld\n", accumulator % 1000);
    }
}

/* Potential cache line aliasing */
void cache_line_aliasing_test(int iterations) {
    volatile int i, j;
    int temp[64]; /* Typical cache line size */
    
    for (i = 0; i < iterations; i++) {
        /* Copy with potential aliasing */
        for (j = 0; j < 64; j++) {
            temp[j] = matrix_a[i % LARGE_SIZE][j % (LARGE_SIZE/100)];
        }
        
        /* Use the data to prevent elimination */
        for (j = 0; j < 63; j++) {
            temp[j] = temp[j] + temp[j+1];
        }
    }
}

/* Mixed data type operations */
void mixed_data_type_operations(void) {
    volatile int i, j;
    double d_acc = 0.0;
    int i_acc = 0;
    
    /* Mix int and double operations */
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 500; j++) {
            d_acc += (double)matrix_a[i][j % 100] * 0.5;
            i_acc += char_array[i * 2][j % 100];
            
            /* Cross-type assignment */
            if ((i * j) % 7 == 0) {
                double_array[i][j % 500] = (double)i_acc;
            }
        }
    }
    
    printf("Mixed ops - double: %.2f, int: %d\n", d_acc, i_acc);
}

/* Main function with architecture-specific code blocks */
int main(int argc, char *argv[]) {
    volatile int n = 500, m = 500, p = 500;
    volatile int test_size = 10000;
    volatile int test_stride = 32;
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < LARGE_SIZE; i++) {
        for (int j = 0; j < LARGE_SIZE/100; j++) {
            matrix_a[i][j] = rand() % 100;
        }
    }
    
    for (int i = 0; i < LARGE_SIZE/100; i++) {
        for (int j = 0; j < LARGE_SIZE; j++) {
            matrix_b[i][j] = rand() % 100;
        }
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Architecture-specific code blocks to encourage multiple -march tests */
#ifdef __x86_64__
    /* Code that benefits from x86-64 optimizations */
    printf("x86-64 architecture detected\n");
    
    /* Try to use CPUID intrinsics if available */
    #ifdef __GNUC__
        /* This may prompt CPU feature detection */
        __builtin_cpu_init();
    #endif
    
#elif defined(__i386__)
    printf("i386 architecture detected\n");
#endif
    
    /* Execute various cache-sensitive patterns */
    matrix_multiply_optimized(n, m, p);
    
    stride_access_pattern(test_size, test_stride);
    
    cache_line_aliasing_test(1000);
    
    mixed_data_type_operations();
    
    /* Additional loop with variable bounds */
    volatile int dynamic_limit = (argc > 1) ? atoi(argv[1]) : 800;
    for (int i = 0; i < dynamic_limit; i++) {
        for (int j = i; j < dynamic_limit; j += 8) {
            matrix_c[i % 100][j % 100] += 
                matrix_a[i % LARGE_SIZE][j % (LARGE_SIZE/100)] * 
                matrix_b[j % (LARGE_SIZE/100)][i % LARGE_SIZE];
        }
    }
    
    printf("Computation complete.\n");
    
    /* Final result to prevent entire program elimination */
    int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += matrix_c[i][i];
    }
    printf("Final checksum: %d\n", final_result % 1000);
    
    return 0;
}
