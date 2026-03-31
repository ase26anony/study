/* sel-sched-trigger.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts creating complex dependencies */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index - creates complex scheduling */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operation with mixed computations */
            acc += in[j] * (j % 2 ? 0.5 : 2.0) + sin(in[j] * 0.01);
        }
        out[i] = acc / i + (i % 3 ? 1.0 : -1.0);
        
        /* Inline assembly with clobbers to force scheduler constraints */
        asm volatile ("# Selective scheduler test\n\t"
                     "nop" : : : "rax", "rcx", "memory");
    }
}

/* Mixed data structure with non-contiguous access */
struct MixedData {
    int id;
    double value;
    char tag[8];
    float vector[4];
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_memory_access(struct MixedData* data, int count) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < count - 1; i += 2) {
        /* Pointer arithmetic and type mixing */
        data[i].value = data[i + 1].id * 0.5 + (data[i].tag[0] * 0.01);
        
        /* Conditional move operations */
        float temp = (i % 5 == 0) ? data[i].vector[0] : data[i].vector[1];
        data[i].vector[2] = temp * (data[i].id > 100 ? 2.0f : 0.5f);
        
        /* Function call with varying arguments */
        data[i].value += pow(fabs(data[i].value), 1.5);
    }
}

/* SIMD operations using intrinsics */
__attribute__((target("sse2")))
static void simd_operations(float* restrict dst, const float* restrict src, int len) {
    /* Unroll pragma to create larger basic blocks */
    #pragma GCC unroll 4
    for (int i = 0; i < len; i += 4) {
        /* SSE intrinsics generating specific RTL patterns */
        __m128 a = _mm_loadu_ps(src + i);
        __m128 b = _mm_loadu_ps(src + i + 4);
        __m128 c = _mm_add_ps(a, b);
        __m128 d = _mm_mul_ps(c, _mm_set1_ps(0.5f));
        _mm_storeu_ps(dst + i, d);
        
        /* More complex SIMD operations */
        __m128 e = _mm_sub_ps(a, b);
        __m128 f = _mm_mul_ps(e, e);
        _mm_storeu_ps(dst + i + 4, f);
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_switch(int x) {
    static const void* jtable[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_default, &&case_5, &&case_6, &&case_7
    };
    
    int result = 0;
    int idx = x & 7;
    
    /* Indirect branch challenging for scheduler */
    goto *jtable[idx];
    
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
    result = x ^ 0xFF;
    goto end;
case_5:
    result = x << 2;
    goto end;
case_6:
    result = x >> 1;
    goto end;
case_7:
    result = ~x;
    goto end;
case_default:
    result = x + 1;
    goto end;
    
end:
    return result;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int mixed_switch(int val) {
    int result = 0;
    
    /* Switch creates various jump table implementations */
    switch (val) {
        case 0 ... 10:  /* Dense range */
            result = val * 10;
            break;
        case 20:
            result = val + 100;
            break;
        case 50:
            result = val - 25;
            break;
        case 100:
            result = val / 2;
            break;
        case 200 ... 210:  /* Another dense range */
            result = val * 2;
            break;
        case 500:
            result = val ^ 0xABCD;
            break;
        case 1000:
            result = val << 3;
            break;
        default:
            result = -val;
            break;
    }
    
    /* Ternary operators generating conditional moves */
    return (result > 0) ? result : (val % 2 ? result * 2 : result / 2);
}

/* Main computational kernel */
__attribute__((optimize("O3", "fsel-sched-pipelining", 
                       "fsel-sched-pipelining-outer-loops", "funroll-loops")))
static unsigned long benchmark_kernel(int iterations) {
    const int N = 256;
    double* array1 = aligned_alloc(64, N * sizeof(double));
    double* array2 = aligned_alloc(64, N * sizeof(double));
    float* simd_src = aligned_alloc(64, N * sizeof(float));
    float* simd_dst = aligned_alloc(64, N * sizeof(float));
    struct MixedData* mixed = aligned_alloc(64, N * sizeof(struct MixedData));
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; i++) {
        array1[i] = sin(i * 0.1);
        simd_src[i] = (i % 10) * 0.1f;
        mixed[i].id = i;
        mixed[i].value = i * 0.5;
        mixed[i].tag[0] = 'A' + (i % 26);
        for (int j = 0; j < 4; j++) {
            mixed[i].vector[j] = (i + j) * 0.25f;
        }
    }
    
    unsigned long checksum = 0;
    
    /* Perform multiple computation patterns to stress scheduler */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pattern 1: Loop-carried dependencies */
        complex_loop_carried_deps(N, array2, array1);
        
        /* Pattern 2: Mixed memory access */
        mixed_memory_access(mixed, N);
        
        /* Pattern 3: SIMD operations */
        simd_operations(simd_dst, simd_src, N);
        
        /* Pattern 4: Complex control flow */
        int switch_val = iter % 1500;
        int r1 = computed_goto_switch(switch_val);
        int r2 = mixed_switch(switch_val);
        
        /* Combine results to prevent dead code elimination */
        checksum ^= *(unsigned long*)&array2[N/2];
        checksum += *(unsigned int*)&mixed[N/4].value;
        checksum ^= *(unsigned int*)&simd_dst[N/3];
        checksum += r1 ^ r2;
        
        /* Rotate data for next iteration */
        double temp = array1[0];
        memmove(array1, array1 + 1, (N - 1) * sizeof(double));
        array1[N - 1] = temp;
    }
    
    free(array1);
    free(array2);
    free(simd_src);
    free(simd_dst);
    free(mixed);
    
    return checksum;
}

/* Main function with optimization attributes */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run benchmark with sufficient iterations to trigger scheduling decisions */
    unsigned long result = benchmark_kernel(1000);
    
    printf("Result checksum: %lu\n", result);
    printf("Program completed. Check compiler debug output for sel-sched dumps.\n");
    
    return 0;
}
