/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -march=haswell -mtune=haswell sel-sched-trigger.c -lm -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define N 256
#define M 128
#define ITERATIONS 1000

/* Mixed data type structure for complex memory access */
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
    
    /* Loop with varying trip count and complex dependencies */
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < i % 16; ++j) {
            /* Data-dependent operations with type mixing */
            data[i].value = data[i].id * 0.5 + sin(data[j % size].value);
            data[i].weight = (float)(data[i].value * 0.3);
            
            /* Conditional move operations */
            double temp = (j % 3 == 0) ? data[i].value * 2.0 : data[i].value / 2.0;
            sum += temp;
            
            /* Inline assembly with clobbers to force scheduler work */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "rcx", "memory"
            );
        }
    }
    return sum;
}

/* SIMD processing function */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void simd_process(float* src, float* dst, int len) {
    /* Unroll pragma to create large basic blocks */
    #pragma GCC unroll 4
    for (int i = 0; i < len; i += 4) {
        if (i + 3 < len) {
            /* SIMD operations using intrinsics */
            __m128 a = _mm_loadu_ps(&src[i]);
            __m128 b = _mm_loadu_ps(&src[(i + 1) % len]);
            __m128 c = _mm_add_ps(a, b);
            __m128 d = _mm_mul_ps(c, _mm_set1_ps(0.5f));
            _mm_storeu_ps(&dst[i], d);
            
            /* More complex SIMD operations */
            __m128 e = _mm_sqrt_ps(d);
            __m128 f = _mm_max_ps(d, e);
            _mm_storeu_ps(&dst[(i + 2) % len], f);
        }
    }
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O2")))
static int computed_goto_test(int x) {
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
    result = x / 3;
    goto end;
case_3:
    result = x - 50;
    goto end;
case_4:
    result = x * x;
    goto end;
case_5:
    result = x | 0xFF;
    goto end;
case_6:
    result = x ^ 0xAA;
    goto end;
case_7:
    result = ~x;
    goto end;
    
end:
    return result;
}

/* Switch statement with mixed dense/sparse cases */
__attribute__((optimize("O2")))
static int complex_switch(int val) {
    int result = 0;
    
    switch (val) {
        /* Dense cases */
        case 0:  result = val + 1; break;
        case 1:  result = val * 2; break;
        case 2:  result = val << 3; break;
        case 3:  result = val | 0xF0; break;
        case 4:  result = val & 0x0F; break;
        
        /* Sparse cases */
        case 10: result = val / 2; break;
        case 20: result = val - 15; break;
        case 50: result = val + 100; break;
        case 100: result = val * val; break;
        case 200: result = ~val; break;
        
        /* Default with computation */
        default: 
            result = (val % 2 == 0) ? pow(val, 2.0) : sqrt(fabs(val));
            break;
    }
    
    return result;
}

/* Matrix multiplication with loop-carried dependencies */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void matrix_multiply(double A[N][M], double B[M][N], double C[N][N]) {
    /* Triple nested loop with complex dependencies */
    for (int i = 0; i < N; ++i) {
        #pragma GCC unroll 2
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0.0;
            for (int k = 0; k < M; ++k) {
                /* Non-contiguous access patterns */
                C[i][j] += A[i][k] * B[k][j];
                
                /* Conditional operation to create varied instructions */
                if ((i + j + k) % 8 == 0) {
                    C[i][j] = fmod(C[i][j], 1000.0);
                }
            }
            
            /* Function call within loop */
            C[i][j] = sin(C[i][j]) + cos(C[i][j] * 0.5);
        }
    }
}

/* Main benchmark function */
int main(void) {
    /* Initialize data structures */
    struct MixedData mixed_array[256];
    float src_array[1024];
    float dst_array[1024];
    double matrixA[N][M];
    double matrixB[M][N];
    double matrixC[N][N];
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        mixed_array[i].id = i;
        mixed_array[i].value = sin(i * 0.1);
        mixed_array[i].tag = (char)(i % 128);
        mixed_array[i].weight = (float)(cos(i * 0.05));
        mixed_array[i].timestamp = i * 1000L;
    }
    
    for (int i = 0; i < 1024; i++) {
        src_array[i] = (float)((i % 64) * 0.1);
        dst_array[i] = 0.0f;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrixA[i][j] = (i + j) * 0.01;
            matrixB[j][i] = (i * j) * 0.005;
        }
    }
    
    long long total_checksum = 0;
    
    /* Run multiple iterations to ensure sustained scheduling pressure */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Process mixed data with complex loops */
        double sum1 = process_mixed_data(mixed_array, 256);
        total_checksum += (long long)(sum1 * 1000);
        
        /* SIMD processing */
        simd_process(src_array, dst_array, 1024);
        for (int i = 0; i < 1024; i += 8) {
            total_checksum += (long long)(dst_array[i] * 100);
        }
        
        /* Computed goto tests */
        for (int i = 0; i < 32; i++) {
            total_checksum += computed_goto_test(i + iter);
        }
        
        /* Complex switch tests */
        for (int i = 0; i < 64; i++) {
            total_checksum += complex_switch((i * 3 + iter) % 250);
        }
        
        /* Matrix multiplication every few iterations */
        if (iter % 10 == 0) {
            matrix_multiply(matrixA, matrixB, matrixC);
            for (int i = 0; i < N; i += 8) {
                total_checksum += (long long)(matrixC[i][i] * 10000);
            }
        }
        
        /* Additional inline assembly to stress scheduler */
        asm volatile (
            "movq %%rax, %%rbx\n\t"
            "addq $1, %%rbx\n\t"
            "xorq %%rcx, %%rcx\n\t"
            : : : "rax", "rbx", "rcx", "memory"
        );
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %lld\n", total_checksum);
    
    /* Use results to prevent optimization */
    volatile double final_result = mixed_array[0].value + dst_array[0] + matrixC[0][0];
    (void)final_result;
    
    return 0;
}
