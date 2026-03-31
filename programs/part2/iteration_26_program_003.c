/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#pragma GCC optimize ("O2")
#pragma GCC optimize ("fsel-sched-pipelining")

#define N 256
#define M 128
#define ITERATIONS 1000

/* Mixed data type structure for complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long timestamp;
};

/* Global arrays to prevent optimization */
struct MixedData data_array[N];
double matrix_a[N][M];
double matrix_b[M][N];
int results[N];
__m128i simd_buffer[N/4];

/* Helper function with varying arguments */
static double __attribute__((always_inline)) compute_transform(double x, double y, int scale) {
    return (sin(x) * cos(y)) * scale;
}

/* Function with complex control flow */
__attribute__((optimize("O3"))) 
static int process_mixed_data(struct MixedData* arr, int size) {
    int sum = 0;
    
    /* Loop with varying trip count */
    for (int i = 0; i < size; ++i) {
        /* Inner loop with i-dependent bounds */
        for (int j = 0; j < i % 16; ++j) {
            /* Data-dependent operation with mixed types */
            arr[i].value = arr[i].value * 0.97 + (arr[j].id * 0.03);
            
            /* Conditional move operation */
            arr[i].weight = (j % 3 == 0) ? arr[i].weight * 1.1f : arr[i].weight * 0.9f;
        }
        
        /* Non-contiguous memory access */
        if (i % 2 == 0) {
            arr[i].tag = 'A' + (i % 26);
        } else {
            arr[i].tag = 'a' + (i % 26);
        }
        
        /* Function call with varying arguments */
        arr[i].value = compute_transform(arr[i].value, i * 0.1, arr[i].id);
        
        sum += arr[i].id;
    }
    
    return sum;
}

/* SIMD processing function */
__attribute__((optimize("O3")))
static void simd_operations(int* dest, const int* src, int len) {
    /* Process with SSE intrinsics */
    for (int i = 0; i < len; i += 4) {
        __m128i a = _mm_loadu_si128((__m128i*)(src + i));
        __m128i b = _mm_add_epi32(a, a);
        __m128i c = _mm_mullo_epi16(b, _mm_set1_epi32(0x00010001));
        
        /* Inline assembly with clobbered registers */
        asm volatile (
            "movdqa %0, %%xmm0\n\t"
            "psrld $1, %%xmm0\n\t"
            "movdqa %%xmm0, %1\n\t"
            : "=m" (simd_buffer[i/4])
            : "m" (c)
            : "xmm0", "memory"
        );
        
        _mm_storeu_si128((__m128i*)(dest + i), simd_buffer[i/4]);
    }
}

/* Function with computed goto */
__attribute__((noinline))
static int jump_table_operation(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int idx = x & 7;
    int result = 0;
    
    goto *jump_table[idx];
    
case_0:
    result = x * 2;
    goto end;
case_1:
    result = x + 100;
    goto end;
case_2:
    result = x / 2;
    goto end;
case_3:
    result = x ^ 0xFF;
    goto end;
case_4:
    result = x << 2;
    goto end;
case_5:
    result = x >> 1;
    goto end;
case_6:
    result = ~x;
    goto end;
case_7:
    result = x * x;
    goto end;
    
end:
    return result;
}

/* Matrix multiplication with complex loop structure */
__attribute__((optimize("O3")))
static void matrix_multiply(double dest[N][N], double a[N][M], double b[M][N]) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        /* Outer loop with pipelining potential */
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            
            /* Inner loop with varying access patterns */
            for (int k = 0; k < M; ++k) {
                /* Mixed array access patterns */
                sum += a[i][k] * b[k][j];
                
                /* Conditional operation to create data dependencies */
                if (k % 8 == 0) {
                    sum = fabs(sum);
                }
            }
            
            dest[i][j] = sum;
            
            /* Additional operation to increase instruction mix */
            dest[i][j] = (dest[i][j] > 1000.0) ? 1000.0 : dest[i][j];
        }
    }
}

/* Function with switch statement testing different patterns */
__attribute__((optimize("O2")))
static double switch_operations(int mode, double x) {
    double result = x;
    
    switch (mode) {
        /* Dense case range */
        case 0: result = sin(x); break;
        case 1: result = cos(x); break;
        case 2: result = tan(x); break;
        case 3: result = exp(x); break;
        case 4: result = log(x + 1.0); break;
        
        /* Sparse case range */
        case 10: result = pow(x, 2.0); break;
        case 20: result = pow(x, 3.0); break;
        case 30: result = sqrt(x); break;
        case 100: result = x * M_PI; break;
        case 200: result = x / M_PI; break;
        
        default:
            /* Complex default case with loop */
            for (int i = 0; i < 5; ++i) {
                result = result * 0.9 + 0.1 * i;
            }
            break;
    }
    
    return result;
}

/* Main computational kernel */
__attribute__((optimize("O3")))
static unsigned long benchmark_kernel(void) {
    unsigned long checksum = 0;
    double temp_matrix[N][N];
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data_array[i].id = i;
        data_array[i].value = i * 0.5;
        data_array[i].tag = 'A' + (i % 26);
        data_array[i].weight = (i % 10) * 0.1f;
        data_array[i].timestamp = i * 1000;
        
        for (int j = 0; j < M; ++j) {
            matrix_a[i][j] = (i + j) * 0.1;
            matrix_b[j][i] = (i - j) * 0.2;
        }
    }
    
    /* Perform multiple operations to create scheduling pressure */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* 1. Process mixed data with complex loops */
        int data_sum = process_mixed_data(data_array, N);
        checksum ^= data_sum;
        
        /* 2. Matrix multiplication */
        matrix_multiply(temp_matrix, matrix_a, matrix_b);
        
        /* Use result to prevent elimination */
        for (int i = 0; i < 10; ++i) {
            checksum += (unsigned long)(temp_matrix[i][i] * 1000);
        }
        
        /* 3. SIMD operations */
        simd_operations(results, (int*)data_array, N);
        
        for (int i = 0; i < N; i += 8) {
            checksum += results[i];
        }
        
        /* 4. Jump table operations */
        for (int i = 0; i < 32; ++i) {
            checksum += jump_table_operation(i + iter);
        }
        
        /* 5. Switch operations */
        for (int i = 0; i < 20; ++i) {
            double val = switch_operations(i % 15, i * 0.1);
            checksum += (unsigned long)(val * 100);
        }
        
        /* Additional inline assembly to force scheduling constraints */
        asm volatile (
            "mov $0, %%eax\n\t"
            "cpuid\n\t"
            : : : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    
    return checksum;
}

int main(void) {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run the benchmark kernel */
    unsigned long final_checksum = benchmark_kernel();
    
    printf("Final checksum: %lu\n", final_checksum);
    printf("Program completed. Check compiler debug output for sel-sched dumps.\n");
    
    return 0;
}
