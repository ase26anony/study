/* cache_detection_test.c
 * Designed to trigger GCC driver's CPU cache detection logic
 * for x86 cache descriptor values: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24, 0x2c,
 * 0x39-0x3e, 0x41-0x45, 0x48-0x49, 0x4e, 0x60, 0x66-0x68, 0x78-0x80, 0x82-0x87
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force compiler to consider cache-aware optimizations */
#define ARRAY_SIZE 10000
#define BLOCK_SIZE 64

/* Volatile variables to prevent compile-time optimization */
volatile int outer_limit = ARRAY_SIZE;
volatile int stride = 8;

/* Different array types to test various cache line behaviors */
static int matrix_a[ARRAY_SIZE][ARRAY_SIZE];
static double matrix_b[ARRAY_SIZE][ARRAY_SIZE];
static char char_array[ARRAY_SIZE * 4];
static int result_array[ARRAY_SIZE];

/* Matrix multiplication kernel - benefits from cache blocking */
void matrix_multiply(int n, volatile int limit) {
    int i, j, k;
    double sum;
    
    /* Triple nested loop - compiler may apply cache-aware transformations */
    for (i = 0; i < limit; i++) {
        for (j = 0; j < n; j++) {
            sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_b[i][j] = sum;
        }
    }
}

/* Non-unit stride access pattern - tests cache line utilization */
int stride_access(int size, int step) {
    int i, total = 0;
    
    /* Access every 'step'th element - may trigger prefetching logic */
    for (i = 0; i < size; i += step) {
        total += char_array[i];
        char_array[i] = (char)(total & 0xFF);
    }
    
    return total;
}

/* Cache line aliasing test - copying with potential conflicts */
void cache_line_copy(int* dest, int* src, int size) {
    int i, j;
    
    /* Copy in blocks to potentially trigger cache-aware optimizations */
    for (i = 0; i < size; i += BLOCK_SIZE) {
        for (j = 0; j < BLOCK_SIZE && (i + j) < size; j++) {
            dest[i + j] = src[i + j] * 2 - 1;
        }
    }
}

/* Mixed data type operations */
double mixed_operations(int iterations) {
    double sum = 0.0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < ARRAY_SIZE; j++) {
            /* Mix int and double operations */
            sum += (double)matrix_a[i % 100][j % 100] * 0.5;
            result_array[j] = (int)(sum * 100.0);
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int i, j;
    double total_sum = 0.0;
    clock_t start, end;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = 0; j < ARRAY_SIZE; j++) {
            matrix_a[i][j] = rand() % 100;
            matrix_b[i][j] = (double)(rand() % 100) / 100.0;
        }
        char_array[i] = (char)(rand() % 256);
        result_array[i] = 0;
    }
    
    start = clock();
    
    /* Execute different loop patterns to trigger various cache optimizations */
    
    /* Pattern 1: Matrix multiplication (benefits from cache blocking) */
    #ifdef __x86_64__
    printf("x86-64 architecture detected - using cache-aware optimizations\n");
    matrix_multiply(500, outer_limit / 20);
    #else
    matrix_multiply(100, outer_limit / 50);
    #endif
    
    /* Pattern 2: Non-unit stride access */
    int stride_result = stride_access(ARRAY_SIZE * 4, stride);
    printf("Stride access result: %d\n", stride_result);
    
    /* Pattern 3: Cache line copying */
    int* heap_array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* heap_array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (heap_array1 && heap_array2) {
        for (i = 0; i < ARRAY_SIZE; i++) {
            heap_array1[i] = rand() % 1000;
        }
        
        cache_line_copy(heap_array2, heap_array1, ARRAY_SIZE);
        
        free(heap_array1);
        free(heap_array2);
    }
    
    /* Pattern 4: Mixed operations */
    total_sum = mixed_operations(10);
    
    end = clock();
    
    printf("Total computation result: %f\n", total_sum);
    printf("Execution time: %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Explicit CPU feature detection (if available) */
    #ifdef __GNUC__
    /* This may prompt the driver to initialize CPU detection */
    __builtin_cpu_init();
    #endif
    
    return 0;
}

/* Additional architecture-specific code blocks */
#ifdef __i386__
/* 32-bit x86 specific optimizations */
void i386_specific_loop(int* arr, int size) {
    int i;
    /* Different loop pattern for 32-bit */
    for (i = 0; i < size; i += 4) {
        arr[i] = arr[i] * 3 + 1;
    }
}
#endif

#ifdef __x86_64__
/* 64-bit x86 specific optimizations */
void x86_64_specific_loop(double* arr, int size) {
    int i;
    /* Vector-friendly pattern for 64-bit */
    for (i = 0; i < size; i += 8) {
        arr[i] = arr[i] * 2.5 - 1.0;
    }
}
#endif
