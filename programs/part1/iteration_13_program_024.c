/* cache_detection_test.c
 * Designed to trigger GCC driver cache detection logic for x86 targets
 * Compile with various -march and -mtune options to exercise different
 * cache descriptor cases in driver-i386.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Large arrays to exceed typical L1/L2 cache sizes */
#define SIZE_A 10000
#define SIZE_B 5000
#define SIZE_C 2000

/* Volatile variables to prevent compile-time optimization */
volatile int v_size = SIZE_A;
volatile int v_stride = 8;

/* Function prototypes to force different optimization contexts */
void matrix_multiply_pattern(int n, double * restrict a, double * restrict b, double * restrict c);
void stride_access_pattern(int size, int stride, int *arr);
void cache_line_copy_pattern(int size, char *src, char *dst);
double compute_checksum(double *arr, int size);

int main(int argc, char *argv[]) {
    /* Use command line args to vary behavior and prevent dead code elimination */
    int use_heap = (argc > 1) ? atoi(argv[1]) : 1;
    int iter_count = (argc > 2) ? atoi(argv[2]) : 3;
    
    /* Allocate arrays - mix of stack and heap to test different access patterns */
    double *A, *B, *C;
    int *int_array;
    char *char_src, *char_dst;
    
    if (use_heap) {
        A = (double*)malloc(SIZE_A * sizeof(double));
        B = (double*)malloc(SIZE_B * sizeof(double));
        C = (double*)malloc(SIZE_C * sizeof(double));
        int_array = (int*)malloc(SIZE_A * sizeof(int));
        char_src = (char*)malloc(SIZE_A * sizeof(char));
        char_dst = (char*)malloc(SIZE_A * sizeof(char));
    } else {
        /* Force stack allocation for some cases */
        double A_stack[SIZE_A];
        double B_stack[SIZE_B];
        double C_stack[SIZE_C];
        A = A_stack;
        B = B_stack;
        C = C_stack;
        int int_array_stack[SIZE_A];
        char char_src_stack[SIZE_A];
        char char_dst_stack[SIZE_A];
        int_array = int_array_stack;
        char_src = char_src_stack;
        char_dst = char_dst_stack;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE_A; i++) {
        A[i] = (double)rand() / RAND_MAX;
        int_array[i] = rand() % 100;
        char_src[i] = (char)(rand() % 256);
    }
    for (int i = 0; i < SIZE_B; i++) {
        B[i] = (double)rand() / RAND_MAX;
    }
    
    double total_checksum = 0.0;
    
    /* Execute multiple loop patterns that benefit from cache-aware optimizations */
    for (int iter = 0; iter < iter_count; iter++) {
        /* Pattern 1: Matrix multiplication-like triple nested loop */
        /* This benefits from cache blocking/tiling optimizations */
        matrix_multiply_pattern(v_size / 10, A, B, C);
        
        /* Pattern 2: Non-unit stride access - tests prefetching decisions */
        stride_access_pattern(v_size, v_stride + iter, int_array);
        
        /* Pattern 3: Cache line copying with potential aliasing */
        cache_line_copy_pattern(v_size, char_src, char_dst);
        
        /* Mix data types and access patterns */
        if (iter % 2 == 0) {
            /* Column-major like access */
            for (int j = 0; j < 100; j++) {
                for (int i = 0; i < SIZE_A; i += 64) { /* Cache line sized jumps */
                    A[i] = A[i] * 1.0001 + B[j % SIZE_B];
                }
            }
        } else {
            /* Row-major like access with temporal locality */
            for (int i = 0; i < SIZE_A - 64; i += 1) {
                double sum = 0.0;
                for (int k = 0; k < 64; k++) {
                    sum += A[i + k];
                }
                C[i % SIZE_C] = sum / 64.0;
            }
        }
        
        /* Compute checksum to ensure computations aren't optimized away */
        total_checksum += compute_checksum(A, SIZE_A) +
                         compute_checksum(B, SIZE_B) +
                         compute_checksum(C, SIZE_C);
    }
    
    printf("Result checksum: %f\n", total_checksum);
    
    /* Conditional compilation for different x86 architectures */
    /* This encourages testing with different -march flags */
#ifdef __x86_64__
    #ifdef __AVX__
        printf("Compiled for AVX-capable x86-64\n");
    #else
        printf("Compiled for x86-64\n");
    #endif
#elif defined(__i386__)
    printf("Compiled for i386\n");
#endif
    
    /* Explicit CPU feature detection if available */
#ifdef __GNUC__
    /* These builtins may trigger CPUID queries */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 supported\n");
    }
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
    }
#endif
    
    if (use_heap) {
        free(A);
        free(B);
        free(C);
        free(int_array);
        free(char_src);
        free(char_dst);
    }
    
    return 0;
}

/* Matrix multiplication pattern - triple nested loop */
void matrix_multiply_pattern(int n, double * restrict a, double * restrict b, double * restrict c) {
    /* Simulate matrix multiplication access pattern */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Access with different strides to test cache behavior */
                sum += a[i * n + k] * b[k * n + j];
            }
            c[i * n + j] = sum;
        }
    }
}

/* Non-unit stride access pattern */
void stride_access_pattern(int size, int stride, int *arr) {
    int sum = 0;
    /* Large stride access - tests hardware prefetcher effectiveness */
    for (int i = 0; i < size; i += stride) {
        sum += arr[i];
        /* Mix in some dependent operations */
        arr[i] = (arr[i] * 13 + 7) % 97;
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = sum;
    (void)dummy;
}

/* Cache line copy with potential aliasing */
void cache_line_copy_pattern(int size, char *src, char *dst) {
    /* Copy with potential cache line conflicts */
    for (int i = 0; i < size - 64; i += 64) {
        /* Unrolled copy of cache line sized chunks */
        for (int j = 0; j < 64; j++) {
            dst[i + j] = src[i + ((j + 31) % 64)]; /* Offset to cause potential aliasing */
        }
    }
    
    /* Additional pattern: reverse copy every other cache line */
    for (int i = 64; i < size - 128; i += 128) {
        for (int j = 0; j < 64; j++) {
            dst[i + 63 - j] = src[i + j];
        }
    }
}

/* Compute simple checksum */
double compute_checksum(double *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * (i % 7 + 1);
    }
    return sum;
}
