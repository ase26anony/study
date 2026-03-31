/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

/* Mixed data type structure for complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long long timestamp;
};

/* Helper function with attribute to ensure optimization */
static __attribute__((optimize("O2"))) double process_value(double x, int iter) {
    /* Conditional moves and mixed operations */
    double result = (iter % 2) ? x * 0.5 : x * 2.0;
    result = (x > 0) ? result + sin(x) : result - cos(x);
    
    /* Inline assembly with clobbers to force scheduler constraints */
    asm volatile ("# BEGIN ASM BLOCK\n\t"
                  "nop\n\t"
                  "# END ASM BLOCK" 
                  : : : "rax", "rcx", "memory");
    
    return result;
}

/* Function with complex loop-carried dependencies */
__attribute__((optimize("O2"))) 
void nested_dependency_pattern(int N, double* restrict out, const double* restrict in) {
    /* Outer loop with varying inner loop trip count */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with pointer arithmetic */
            double val = in[(i * 31 + j * 17) % N];
            acc += process_value(val, j);
            
            /* Conditional operation encouraging CMOV */
            acc = (val > 0.5) ? acc * 1.1 : acc * 0.9;
        }
        out[i] = acc;
    }
}

/* SIMD processing function with mixed operations */
__attribute__((optimize("O3")))
void simd_processing(float* restrict dst, const float* restrict src, int len) {
    /* Process with SSE intrinsics */
    for (int i = 0; i < len - 3; i += 4) {
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_loadu_ps(src + ((i * 7) % len));
        
        /* Mixed SIMD operations */
        __m128 sum = _mm_add_ps(a, b);
        __m128 prod = _mm_mul_ps(a, b);
        __m128 result = _mm_sub_ps(sum, _mm_mul_ps(prod, _mm_set1_ps(0.5f)));
        
        _mm_storeu_ps(dst + i, result);
    }
    
    /* Scalar tail processing */
    for (int i = len - (len % 4); i < len; ++i) {
        dst[i] = src[i] * 0.75f + src[(i * 3) % len] * 0.25f;
    }
}

/* Function with computed goto and switch for complex control flow */
__attribute__((noinline))
int jump_table_computation(int x, int y) {
    static const void* jtable[] = { &&add, &&sub, &&mul, &&div_op, &&mod };
    
    int result = x;
    
    /* Multiple jumps based on complex condition */
    for (int i = 0; i < 5; ++i) {
        int idx = (x * i + y) % 5;
        goto *jtable[idx];
        
    add:
        result += y;
        continue;
    sub:
        result -= y;
        continue;
    mul:
        result *= y;
        continue;
    div_op:
        if (y != 0) result /= (y | 1); /* Avoid division by zero */
        continue;
    mod:
        if (y != 0) result %= (y | 1);
        continue;
    }
    
    return result;
}

/* Matrix-style operation with loop unrolling pragma */
#pragma GCC unroll 4
void matrix_style_operation(double mat[][64], int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            /* Non-contiguous access pattern */
            double sum = 0.0;
            for (int k = 0; k < size; k += 2) {
                sum += mat[i][k] * mat[k][j] - mat[i][k+1] * mat[k+1][j];
            }
            mat[i][j] = sum * 0.01;
        }
    }
}

/* Function with mixed data structure access */
void process_mixed_array(struct MixedData* data, int count) {
    for (int i = 0; i < count; i += 2) {
        /* Non-contiguous structure field access */
        data[i].value = data[i+1].id * 0.5;
        data[i].weight = sin(data[i].value) * 0.5f;
        
        /* Pointer arithmetic with casting */
        char* ptr = (char*)&data[i];
        for (int j = 0; j < 8; ++j) {
            ptr[j] ^= (i + j) & 0xFF;
        }
        
        /* Library function call in loop */
        data[i].timestamp = (long long)fabs(data[i].value * 1000.0);
    }
}

/* Main benchmark function */
int main(void) {
    const int N = 256;
    const int M = 64;
    
    /* Initialize with patterned data */
    double* array1 = aligned_alloc(32, N * sizeof(double));
    double* array2 = aligned_alloc(32, N * sizeof(double));
    float* farray1 = aligned_alloc(16, N * sizeof(float));
    float* farray2 = aligned_alloc(16, N * sizeof(float));
    double matrix[M][M];
    struct MixedData mixed[N/2];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < N; ++i) {
        array1[i] = (double)(i * 1.1 - 0.5);
        farray1[i] = (float)(sin(i * 0.1) * 100.0);
    }
    
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            matrix[i][j] = (i * M + j) * 0.01;
        }
    }
    
    for (int i = 0; i < N/2; ++i) {
        mixed[i].id = i;
        mixed[i].value = i * 0.25;
        mixed[i].tag = 'A' + (i % 26);
        mixed[i].weight = (float)(i * 0.01);
        mixed[i].timestamp = i * 1000LL;
    }
    
    /* Execute various patterns to trigger scheduler */
    long long checksum = 0;
    
    /* Pattern 1: Nested dependencies */
    nested_dependency_pattern(N, array2, array1);
    
    /* Pattern 2: SIMD processing */
    simd_processing(farray2, farray1, N);
    
    /* Pattern 3: Jump table computation */
    int jump_result = 0;
    for (int i = 0; i < 100; ++i) {
        jump_result ^= jump_table_computation(i, i * 3 + 1);
    }
    checksum += jump_result;
    
    /* Pattern 4: Matrix operation */
    matrix_style_operation(matrix, M);
    
    /* Pattern 5: Mixed data structure processing */
    process_mixed_array(mixed, N/2);
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < N; ++i) {
        checksum += (long long)(array2[i] * 1000.0);
        checksum ^= (long long)(farray2[i] * 1000.0f);
    }
    
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            checksum += (long long)(matrix[i][j] * 10000.0);
        }
    }
    
    for (int i = 0; i < N/2; ++i) {
        checksum ^= mixed[i].timestamp;
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray1);
    free(farray2);
    
    return 0;
}
