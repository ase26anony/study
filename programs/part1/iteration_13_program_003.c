/* cache_detection_test.c
 * Designed to trigger CPU cache detection logic in GCC driver
 * Compile with various x86-specific flags to exercise cache descriptor cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force compiler to consider cache characteristics for these arrays */
#define LARGE_SIZE 10000
#define MEDIUM_SIZE 5000
#define SMALL_SIZE 1000

/* Volatile variables to prevent compile-time optimization */
volatile int v_limit = LARGE_SIZE;
volatile int v_stride = 16;
volatile int v_block = 64;

/* Function prototypes to prevent inlining */
double matrix_multiply(int size, volatile int limit);
double stride_access(int size, volatile int stride);
double array_copy(int size, volatile int block);

int main(int argc, char *argv[]) {
    double total_result = 0.0;
    clock_t start, end;
    
    /* Use command line args to vary behavior */
    if (argc > 1) {
        v_limit = atoi(argv[1]);
        if (v_limit <= 0) v_limit = LARGE_SIZE;
    }
    if (argc > 2) {
        v_stride = atoi(argv[2]);
        if (v_stride <= 0) v_stride = 16;
    }
    if (argc > 3) {
        v_block = atoi(argv[3]);
        if (v_block <= 0) v_block = 64;
    }
    
    printf("Starting cache-sensitive computations...\n");
    printf("Parameters: limit=%d, stride=%d, block=%d\n", 
           v_limit, v_stride, v_block);
    
    start = clock();
    
    /* Execute different cache-sensitive patterns */
    
    /* Pattern 1: Matrix multiplication style - benefits from L1/L2 cache blocking */
    total_result += matrix_multiply(MEDIUM_SIZE, v_limit);
    
    /* Pattern 2: Non-unit stride access - tests cache line utilization */
    total_result += stride_access(LARGE_SIZE, v_stride);
    
    /* Pattern 3: Blocked array copy - tests cache line aliasing and prefetch */
    total_result += array_copy(LARGE_SIZE, v_block);
    
    /* Pattern 4: Mixed data type access pattern */
    {
        int* int_array = (int*)malloc(LARGE_SIZE * sizeof(int));
        double* double_array = (double*)malloc(MEDIUM_SIZE * sizeof(double));
        char* char_array = (char*)malloc(LARGE_SIZE * sizeof(char));
        
        if (int_array && double_array && char_array) {
            /* Initialize arrays with non-zero values */
            for (int i = 0; i < LARGE_SIZE; i++) {
                int_array[i] = i % 256;
                char_array[i] = (char)(i % 128);
                if (i < MEDIUM_SIZE) {
                    double_array[i] = (double)(i % 1024) * 0.1;
                }
            }
            
            /* Cross-type computation that forces different cache line sizes */
            for (int i = 0; i < MEDIUM_SIZE; i++) {
                for (int j = 0; j < 8; j++) {
                    int idx = i * 8 + j;
                    if (idx < LARGE_SIZE) {
                        double_array[i] += int_array[idx] * 0.5;
                        double_array[i] += char_array[idx];
                    }
                }
            }
            
            /* Accumulate results */
            for (int i = 0; i < MEDIUM_SIZE; i++) {
                total_result += double_array[i];
            }
            
            free(int_array);
            free(double_array);
            free(char_array);
        }
    }
    
    end = clock();
    
    printf("Total result: %f\n", total_result);
    printf("Computation time: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Explicit CPU feature detection if available */
#ifdef __x86_64__
    /* This may prompt driver to initialize CPU detection */
    asm volatile("" ::: "memory"); /* Memory barrier */
#endif
    
    return 0;
}

/* Matrix multiplication pattern - triple nested loop */
double matrix_multiply(int size, volatile int limit) {
    double result = 0.0;
    int actual_size = size;
    if (limit < size) actual_size = limit;
    
    /* Use static arrays to ensure they're in memory */
    static double matA[500][500];
    static double matB[500][500];
    static double matC[500][500];
    
    /* Initialize matrices */
    for (int i = 0; i < actual_size; i++) {
        for (int j = 0; j < actual_size; j++) {
            matA[i][j] = (i + j) * 0.1;
            matB[i][j] = (i - j) * 0.2;
            matC[i][j] = 0.0;
        }
    }
    
    /* Blocked matrix multiplication - compiler may optimize based on cache */
    int block_size = 32; /* Compiler may adjust based on cache line size */
    for (int ii = 0; ii < actual_size; ii += block_size) {
        for (int jj = 0; jj < actual_size; jj += block_size) {
            for (int kk = 0; kk < actual_size; kk += block_size) {
                int i_end = ii + block_size;
                if (i_end > actual_size) i_end = actual_size;
                int j_end = jj + block_size;
                if (j_end > actual_size) j_end = actual_size;
                int k_end = kk + block_size;
                if (k_end > actual_size) k_end = actual_size;
                
                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {
                        double sum = matC[i][j];
                        for (int k = kk; k < k_end; k++) {
                            sum += matA[i][k] * matB[k][j];
                        }
                        matC[i][j] = sum;
                    }
                }
            }
        }
    }
    
    /* Accumulate result */
    for (int i = 0; i < actual_size; i++) {
        for (int j = 0; j < actual_size; j++) {
            result += matC[i][j];
        }
    }
    
    return result;
}

/* Non-unit stride access pattern */
double stride_access(int size, volatile int stride) {
    double result = 0.0;
    int actual_stride = stride;
    if (actual_stride < 1) actual_stride = 1;
    
    /* Large array to exceed L1 cache */
    double* array = (double*)malloc(size * sizeof(double));
    if (!array) return 0.0;
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        array[i] = i * 0.01;
    }
    
    /* Access with variable stride - tests cache line efficiency */
    for (int base = 0; base < actual_stride; base++) {
        for (int i = base; i < size; i += actual_stride) {
            array[i] = array[i] * 1.1 + 0.5;
            result += array[i];
        }
    }
    
    /* Reverse stride access */
    for (int base = actual_stride - 1; base >= 0; base--) {
        for (int i = size - 1 - base; i >= 0; i -= actual_stride) {
            array[i] = array[i] * 0.9 - 0.3;
            result += array[i];
        }
    }
    
    free(array);
    return result;
}

/* Blocked array copy with potential cache line issues */
double array_copy(int size, volatile int block) {
    double result = 0.0;
    int block_size = block;
    if (block_size < 8) block_size = 8;
    if (block_size > 256) block_size = 256;
    
    /* Two large arrays that may alias in cache */
    int* src = (int*)malloc(size * sizeof(int));
    int* dst = (int*)malloc(size * sizeof(int));
    
    if (!src || !dst) {
        if (src) free(src);
        if (dst) free(dst);
        return 0.0;
    }
    
    /* Initialize source with pattern */
    for (int i = 0; i < size; i++) {
        src[i] = i ^ (i >> 4); /* Some non-linear pattern */
    }
    
    /* Blocked copy - compiler may optimize block size based on cache */
    for (int i = 0; i < size; i += block_size) {
        int end = i + block_size;
        if (end > size) end = size;
        
        /* Copy block */
        for (int j = i; j < end; j++) {
            dst[j] = src[j] + src[size - j - 1];
        }
        
        /* Process block */
        for (int j = i; j < end; j++) {
            dst[j] = (dst[j] * 1103515245 + 12345) & 0x7fffffff;
            result += dst[j] * 0.000001;
        }
    }
    
    /* Verify by reading back in different order */
    for (int i = size - 1; i >= 0; i -= block_size) {
        int start = i - block_size + 1;
        if (start < 0) start = 0;
        
        for (int j = start; j <= i; j++) {
            result += dst[j] * 0.0000001;
        }
    }
    
    free(src);
    free(dst);
    return result;
}

/* Conditional compilation for different x86 architectures */
#ifdef __x86_64__
/* Code that might benefit from specific x86 cache optimizations */
void x86_specific_optimizations() {
    /* Using different data types and operations */
    __attribute__((unused)) long long big_array[4096];
    __attribute__((unused)) float float_array[8192];
    
    /* Loop with potential for SIMD and cache optimization */
    for (int i = 0; i < 4096; i++) {
        big_array[i] = i * i;
        if (i < 8192) {
            float_array[i] = big_array[i] * 0.5f;
        }
    }
}
#endif

/* i386 specific path */
#ifdef __i386__
void i386_specific_optimizations() {
    /* Different access pattern for 32-bit */
    int array32[32768];
    for (int i = 0; i < 32768; i += 8) {
        for (int j = 0; j < 8; j++) {
            array32[i + j] = (i + j) * 3;
        }
    }
}
#endif
