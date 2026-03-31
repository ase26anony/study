/* sel-sched-trigger.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts - creates complex dependencies */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed computations */
            acc += in[j] * (j % 3 == 0 ? 1.5 : (j % 3 == 1 ? 2.0 : 0.5));
            
            /* Conditional move operations */
            double temp = (j > i/2) ? acc * 0.8 : acc * 1.2;
            
            /* Function call within loop - creates call instruction */
            temp = sin(temp) * cos(temp);
            
            /* Inline assembly with clobbers - forces scheduler constraints */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "rcx", "memory"
            );
            
            out[j] += temp;
        }
        out[i] = acc;
    }
}

/* Mixed data structure with non-contiguous access patterns */
struct MixedData {
    int a;
    double b;
    char c[8];
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
static void mixed_data_access(struct MixedData* arr, int size) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic and casting */
        arr[i].b = arr[i + 1].a * 0.5;
        arr[i + 1].d = (float)arr[i].b * 2.0f;
        
        /* String operations within structure */
        memcpy(arr[i].c, arr[i + 1].c, sizeof(arr[i].c));
        
        /* Complex conditional */
        int selector = (i * 17) % 5;
        switch (selector) {
            case 0: arr[i].a += 1; break;
            case 1: arr[i].a *= 2; break;
            case 2: arr[i].a -= 3; break;
            case 3: arr[i].a /= 4; break;
            default: arr[i].a = 0; break;
        }
    }
}

/* SIMD operations using intrinsics */
__attribute__((optimize("O2", "fsel-sched-pipelining", "march=native")))
static void simd_operations(float* restrict dst, const float* restrict src, int len) {
    /* Process with SIMD intrinsics */
    for (int i = 0; i < len - 3; i += 4) {
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_add_ps(a, a);
        __m128 c = _mm_mul_ps(b, _mm_set1_ps(0.5f));
        _mm_storeu_ps(dst + i, c);
        
        /* Mix with scalar operations */
        dst[i] = (dst[i] > 1.0f) ? dst[i] * 0.9f : dst[i] * 1.1f;
    }
    
    /* Handle remainder with scalar operations */
    for (int i = len - (len % 4); i < len; ++i) {
        dst[i] = src[i] * 2.0f;
    }
}

/* Computed goto for indirect branching */
__attribute__((optimize("O2")))
static int computed_goto_operation(int* data, int size) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    int result = 0;
    for (int i = 0; i < size; ++i) {
        int idx = data[i] & 0x7;
        
        /* Indirect branch - challenging for scheduler */
        goto *jump_table[idx];
        
    label_0:
        result += i * 1;
        continue;
    label_1:
        result += i * 2;
        continue;
    label_2:
        result += i * 3;
        continue;
    label_3:
        result += i * 4;
        continue;
    label_4:
        result += i * 5;
        continue;
    label_5:
        result += i * 6;
        continue;
    label_6:
        result += i * 7;
        continue;
    label_7:
        result += i * 8;
        continue;
    }
    
    return result;
}

/* Matrix multiplication with loop unrolling hints */
#pragma GCC unroll 4
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void matrix_multiply(int N, double C[restrict N][N],
                           const double A[restrict N][N],
                           const double B[restrict N][N]) {
    /* Triple nested loops - good for outer loop pipelining */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
                
                /* Insert inline assembly periodically */
                if ((k & 0x3F) == 0) {
                    asm volatile ("nop" : : : "memory");
                }
            }
            C[i][j] = sum;
            
            /* Complex math function call */
            C[i][j] = pow(fabs(C[i][j]), 0.5);
        }
    }
}

/* Main benchmark function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
int main(void) {
    const int N = 256;
    const int M = 100;
    
    /* Initialize with patterned data */
    double* data1 = aligned_alloc(64, N * sizeof(double));
    double* data2 = aligned_alloc(64, N * sizeof(double));
    float* fdata1 = aligned_alloc(64, N * sizeof(float));
    float* fdata2 = aligned_alloc(64, N * sizeof(float));
    struct MixedData* mixed = aligned_alloc(64, M * sizeof(struct MixedData));
    int* idata = aligned_alloc(64, N * sizeof(int));
    
    double matrixA[N][N];
    double matrixB[N][N];
    double matrixC[N][N];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < N; ++i) {
        data1[i] = (double)rand() / RAND_MAX;
        fdata1[i] = (float)rand() / RAND_MAX;
        idata[i] = rand() % 100;
        
        for (int j = 0; j < N; ++j) {
            matrixA[i][j] = (double)rand() / RAND_MAX;
            matrixB[i][j] = (double)rand() / RAND_MAX;
        }
    }
    
    for (int i = 0; i < M; ++i) {
        mixed[i].a = rand();
        mixed[i].b = (double)rand() / RAND_MAX;
        mixed[i].d = (float)rand() / RAND_MAX;
        snprintf(mixed[i].c, sizeof(mixed[i].c), "%d", i);
    }
    
    /* Execute all complex patterns */
    complex_loop_carried_deps(N, data2, data1);
    mixed_data_access(mixed, M);
    simd_operations(fdata2, fdata1, N);
    int goto_result = computed_goto_operation(idata, N);
    matrix_multiply(16, matrixC, matrixA, matrixB);
    
    /* Combine results to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < N; ++i) {
        checksum += data2[i] + fdata2[i];
    }
    
    for (int i = 0; i < M; ++i) {
        checksum += mixed[i].b + mixed[i].d;
    }
    
    checksum += goto_result;
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            checksum += matrixC[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata1);
    free(fdata2);
    free(mixed);
    free(idata);
    
    return 0;
}
