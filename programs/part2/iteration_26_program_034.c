/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 sel-sched-trigger.c -lm -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

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

/* Function with optimization attribute to ensure selective scheduling */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double process_mixed_data(struct MixedData* data, int size) {
    double sum = 0.0;
    
    /* Loop with varying trip count and data-dependent operations */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < i % 16; ++j) {
            /* Complex data-dependent computation */
            data[i].value = data[i].value * 0.97 + 
                           sin(data[j % size].value) * 0.03;
            
            /* Conditional move operation */
            data[i].weight = (data[i].id > j) ? data[i].weight * 1.1f : 
                                               data[i].weight * 0.9f;
            
            /* Inline assembly with clobbered registers */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "memory"
            );
        }
        sum += data[i].value * data[i].weight;
    }
    return sum;
}

/* SIMD-intensive function with vector intrinsics */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void simd_process(int* src, int* dst, int len) {
    /* Unroll pragma to create large basic blocks */
    #pragma GCC unroll 4
    for (int i = 0; i < len; i += 4) {
        if (i + 4 <= len) {
            /* SSE vector operations */
            __m128i vec_a = _mm_loadu_si128((__m128i*)(src + i));
            __m128i vec_b = _mm_add_epi32(vec_a, vec_a);
            __m128i vec_c = _mm_mullo_epi32(vec_b, _mm_set1_epi32(3));
            _mm_storeu_si128((__m128i*)(dst + i), vec_c);
            
            /* Another inline asm with different clobbers */
            asm volatile ("nop" : : : "xmm0", "xmm1", "memory");
        } else {
            /* Scalar fallback with ternary operator */
            for (int j = i; j < len; ++j) {
                dst[j] = (src[j] > 0) ? src[j] * 3 : -src[j] * 3;
            }
        }
    }
}

/* Function with computed goto for complex control flow */
__attribute__((noinline))
static int computed_goto_demo(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int idx = x & 0x7;
    int result = 0;
    
    /* Computed goto */
    goto *jump_table[idx];
    
case_0:
    result = x * 2;
    /* Function call within scheduled region */
    result += (int)sin(result * 0.01);
    goto end;
    
case_1:
    result = x * x;
    asm volatile ("nop" : : : "rcx", "rdx");
    goto end;
    
case_2:
    result = x / 2;
    /* Mixed floating point operation */
    result += (int)(cos(result * 0.01) * 100);
    goto end;
    
case_3:
    result = x + 100;
    /* Another inline asm */
    asm volatile (
        "addl $5, %0\n\t"
        : "+r" (result) : : "cc"
    );
    goto end;
    
case_4:
case_5:
case_6:
case_7:
    result = x - (idx * 10);
    /* Complex expression with multiple operations */
    result = (result > 0) ? result : (result * -1) + (int)(log(fabs(result) + 1) * 10);
    break;
    
end:
    return result;
}

/* Matrix multiplication with complex loop structure */
__attribute__((optimize("O2", "funroll-loops")))
static void matrix_multiply(double A[N][M], double B[M][N], double C[N][N]) {
    /* Nested loops with dependencies */
    for (int i = 0; i < N; ++i) {
        #pragma GCC unroll 2
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            /* Inner loop with varying access pattern */
            for (int k = 0; k < M; ++k) {
                /* Non-contiguous memory access */
                sum += A[i][k] * B[k][(j + i) % N];
                
                /* Conditional operation based on loop indices */
                if ((i + j + k) % 8 == 0) {
                    sum *= 0.999;
                    asm volatile ("nop" : : : "r8", "r9", "r10");
                }
            }
            C[i][j] = sum;
            
            /* Periodic function call */
            if (j % 16 == 0) {
                C[i][j] += pow(fabs(sum), 0.5) * 0.01;
            }
        }
    }
}

/* Switch statement with mixed case patterns */
static int switch_demo(int val) {
    int result = 0;
    
    /* Dense and sparse switch cases */
    switch (val % 20) {
        case 0:
        case 1:
        case 2:
            result = val + 1;
            /* SIMD operation in switch case */
            {
                __m128 v = _mm_set1_ps(val * 0.1f);
                float f[4];
                _mm_storeu_ps(f, v);
                result += (int)(f[0] * 10);
            }
            break;
            
        case 3:
            result = val * 2;
            asm volatile ("nop" : : : "r11", "r12");
            break;
            
        case 7:  /* Sparse case */
            result = val / 2;
            /* Complex floating point */
            result += (int)(tan(val * 0.01) * 50);
            break;
            
        case 15: /* Another sparse case */
            result = val - 100;
            /* Memory barrier asm */
            asm volatile ("mfence" : : : "memory");
            break;
            
        default:
            result = val % 10;
            /* Conditional move idiom */
            result = (val > 0) ? result : -result;
    }
    
    return result;
}

/* Main benchmark function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
int main() {
    /* Initialize data structures */
    struct MixedData mixed_array[N];
    int src_array[N], dst_array[N];
    double matrixA[N][M], matrixB[M][N], matrixC[N][N];
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        mixed_array[i].id = i;
        mixed_array[i].value = sin(i * 0.1) * 100.0;
        mixed_array[i].tag = 'A' + (i % 26);
        mixed_array[i].weight = (i % 10) * 0.1f;
        mixed_array[i].timestamp = i * 1000L;
        
        src_array[i] = i * 3 - 150;
        dst_array[i] = 0;
        
        for (int j = 0; j < M; ++j) {
            matrixA[i][j] = (i + j) * 0.01;
        }
    }
    
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            matrixB[i][j] = (i * j) * 0.005;
        }
    }
    
    long long total_checksum = 0;
    
    /* Run multiple iterations to ensure scheduler activity */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* 1. Process mixed data with complex loops */
        double sum1 = process_mixed_data(mixed_array, N);
        total_checksum += (long long)(sum1 * 1000);
        
        /* 2. SIMD processing */
        simd_process(src_array, dst_array, N);
        for (int i = 0; i < N; ++i) {
            total_checksum += dst_array[i];
        }
        
        /* 3. Computed goto patterns */
        for (int i = 0; i < 32; ++i) {
            int r = computed_goto_demo(i + iter);
            total_checksum += r;
        }
        
        /* 4. Matrix multiplication (every 10th iteration) */
        if (iter % 10 == 0) {
            matrix_multiply(matrixA, matrixB, matrixC);
            for (int i = 0; i < N; i += 8) {
                total_checksum += (long long)(matrixC[i][i] * 10000);
            }
        }
        
        /* 5. Switch statement variations */
        for (int i = 0; i < 50; ++i) {
            int r = switch_demo(i * 7 + iter);
            total_checksum += r;
        }
        
        /* Modify data for next iteration */
        for (int i = 0; i < N; ++i) {
            mixed_array[i].value *= 0.99;
            src_array[i] += i % 7;
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    return 0;
}
