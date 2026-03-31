/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21,
 * 0x24, 0x2c, 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60,
 * 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache characteristics by using
 * large arrays that exceed typical L1/L2 cache sizes */
#define SIZE_A 10000
#define SIZE_B 5000
#define SIZE_C 2000

/* Volatile variables to prevent compile-time optimization */
volatile int v_size = SIZE_A;
volatile int v_stride = 8;

/* Arrays with different data types to test various cache line behaviors */
static int array_int[SIZE_A][SIZE_A/100];
static double array_double[SIZE_B][SIZE_B/100];
static char array_char[SIZE_C][SIZE_C/10];

/* Function prototypes to prevent inlining */
void matrix_multiply_like(int n, volatile int limit);
void non_unit_stride_access(int stride, volatile int limit);
void cache_line_aliasing_copy(volatile int limit);
int compute_result(void);

/* Matrix multiplication-like triple nested loop */
void matrix_multiply_like(int n, volatile int limit) {
    int i, j, k;
    double sum;
    
    /* Triple nested loop - classic pattern for cache optimization */
    for (i = 0; i < n && i < limit; i++) {
        for (j = 0; j < n && j < limit; j++) {
            sum = 0.0;
            for (k = 0; k < n && k < limit; k++) {
                /* Access with different strides to test associativity */
                sum += array_double[i][k] * array_double[k][j];
            }
            array_double[i][j] = sum;
        }
    }
}

/* Loop with non-unit stride accessing pattern */
void non_unit_stride_access(int stride, volatile int limit) {
    int i, j;
    long long accum = 0;
    
    /* Access every 'stride' element to test cache line utilization */
    for (i = 0; i < SIZE_A && i < limit; i += stride) {
        for (j = 0; j < SIZE_A/100 && j < limit/100; j++) {
            accum += array_int[i][j];
            /* Mix operations to prevent simple optimizations */
            array_int[i][j] = (accum & 0xFF) + j;
        }
    }
    
    /* Use result to prevent dead code elimination */
    array_int[0][0] = (int)(accum % 1000);
}

/* Copy between arrays with potential cache line aliasing */
void cache_line_aliasing_copy(volatile int limit) {
    int i, j;
    
    /* Copy with potential cache conflicts */
    for (i = 0; i < SIZE_C && i < limit; i++) {
        for (j = 0; j < SIZE_C/10 && j < limit/10; j++) {
            /* Different access patterns to trigger various cache behaviors */
            array_char[(i * 17) % SIZE_C][j] = 
                array_char[i][(j * 13) % (SIZE_C/10)];
        }
    }
}

/* Compute final result using all arrays */
int compute_result(void) {
    int i, j;
    int result = 0;
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            result += array_int[i][j] + (int)array_double[i][j] + array_char[i][j];
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int i, j;
    int final_result = 0;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < SIZE_A; i++) {
        for (j = 0; j < SIZE_A/100; j++) {
            array_int[i][j] = rand() % 100;
        }
    }
    
    for (i = 0; i < SIZE_B; i++) {
        for (j = 0; j < SIZE_B/100; j++) {
            array_double[i][j] = (double)(rand() % 1000) / 10.0;
        }
    }
    
    for (i = 0; i < SIZE_C; i++) {
        for (j = 0; j < SIZE_C/10; j++) {
            array_char[i][j] = (char)((i + j) % 256);
        }
    }
    
    /* Use command line arguments to vary loop limits (prevents constant propagation) */
    volatile int limit1 = (argc > 1) ? atoi(argv[1]) : 500;
    volatile int limit2 = (argc > 2) ? atoi(argv[2]) : 300;
    volatile int stride = (argc > 3) ? atoi(argv[3]) : 16;
    
    /* Execute different cache-intensive patterns */
    
    /* Pattern 1: Matrix-like operations */
    matrix_multiply_like(200, limit1);
    
    /* Pattern 2: Non-unit stride access */
    non_unit_stride_access(stride, limit2);
    
    /* Pattern 3: Cache line aliasing */
    cache_line_aliasing_copy(limit1);
    
    /* Pattern 4: Mixed access patterns */
    for (i = 0; i < 1000 && i < limit1; i++) {
        for (j = 0; j < 100 && j < limit2; j++) {
            /* Complex addressing to defeat simple analysis */
            int idx = (i * 31 + j * 7) % SIZE_A;
            array_int[idx][j % (SIZE_A/100)] += 
                (int)(array_double[i % SIZE_B][j % (SIZE_B/100)] * 100);
        }
    }
    
    /* Compute and print final result */
    final_result = compute_result();
    printf("Final result: %d\n", final_result);
    
    /* Conditional compilation for different x86 architectures */
    #ifdef __x86_64__
        /* Code block for x86-64 with various march options */
        #if defined(__AVX__)
            printf("Compiled with AVX support\n");
        #elif defined(__SSE4_2__)
            printf("Compiled with SSE4.2 support\n");
        #else
            printf("Generic x86-64 compilation\n");
        #endif
    #endif
    
    #ifdef __i386__
        printf("32-bit x86 compilation\n");
    #endif
    
    /* Explicit CPU feature detection if available */
    #ifdef __GNUC__
        /* These builtins may trigger CPUID queries */
        __builtin_cpu_init();
        if (__builtin_cpu_supports("sse")) {
            printf("SSE supported\n");
        }
        if (__builtin_cpu_supports("avx")) {
            printf("AVX supported\n");
        }
    #endif
    
    return final_result % 100;
}
