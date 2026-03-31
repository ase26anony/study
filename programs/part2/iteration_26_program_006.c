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
static void complex_loop_carried_deps(int N, double* restrict a, double* restrict b) {
    /* Loop-carried dependencies with varying trip counts */
    for (int i = 1; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed types */
            a[i] += b[j] * (i - j) * 0.5;
            b[i] += a[j] * sin(i * 0.01) * cos(j * 0.01);
        }
        /* Inline assembly with clobbers to force scheduling constraints */
        asm volatile ("# Complex dependency barrier" : : : "memory", "rax", "rbx");
    }
}

/* Mixed data types and non-contiguous memory access */
struct MixedData {
    int id;
    double value;
    char tag;
    float aux;
    long counter;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_memory_patterns(struct MixedData* data, int size) {
    /* Non-contiguous access pattern */
    for (int i = 0; i < size; i += 2) {
        data[i].value = data[i + 1].id * 0.75;
        data[i + 1].aux = data[i].value * 2.0f;
        
        /* Conditional moves/select operations */
        data[i].counter = (data[i].id > 100) ? data[i].counter * 2 : data[i].counter / 2;
        data[i].tag = (data[i].value > 50.0) ? 'A' : 'B';
        
        /* Function calls with varying arguments */
        data[i].value = pow(data[i].value, 1.5);
        data[i].aux = sin(data[i].aux * 3.14159f);
    }
}

/* SIMD operations with intrinsics */
__attribute__((optimize("O2", "fsel-sched-pipelining", "funroll-loops")))
static void simd_processing(float* restrict src, float* restrict dst, int len) {
    int i;
    /* Process with SSE intrinsics */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 vec = _mm_loadu_ps(&src[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 scaled = _mm_mul_ps(squared, _mm_set1_ps(0.5f));
        _mm_storeu_ps(&dst[i], scaled);
        
        /* Mix with scalar operations */
        dst[i] += sin(src[i]) * 0.1f;
        dst[i + 1] += cos(src[i + 1]) * 0.2f;
    }
    
    /* Remainder loop with different pattern */
    for (; i < len; ++i) {
        dst[i] = src[i] * (i % 3 + 1) * 0.33f;
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int jump_table_computation(int x, int y) {
    static void* jtable[] = { &&add, &&sub, &&mul, &&div_op, &&mod };
    int result = x;
    
    /* Use computed goto for complex control flow */
    if (y >= 0 && y < 5) {
        goto *jtable[y];
    }
    
add:
    result = x + y;
    goto end;
sub:
    result = x - y;
    goto end;
mul:
    result = x * y;
    goto end;
div_op:
    result = (y != 0) ? x / y : 0;
    goto end;
mod:
    result = (y != 0) ? x % y : 0;
    goto end;
    
end:
    /* More operations to create scheduling pressure */
    result = (result > 1000) ? result >> 2 : result << 2;
    return result ^ (result * 137);
}

/* Switch with dense and sparse cases */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static int complex_switch(int val) {
    int result = 0;
    
    switch (val) {
        /* Dense range */
        case 0:  result = val * 2; break;
        case 1:  result = val + 100; break;
        case 2:  result = val - 50; break;
        case 3:  result = val ^ 0xFF; break;
        case 4:  result = val << 3; break;
        case 5:  result = val >> 1; break;
        
        /* Sparse range */
        case 10: result = val * val; break;
        case 20: result = sqrt(val); break;
        case 50: result = val % 17; break;
        case 100: result = val / 3; break;
        case 200: result = -val; break;
        
        default:
            /* Complex default computation */
            for (int i = 0; i < (val % 10); ++i) {
                result += i * (val % (i + 1));
            }
            break;
    }
    
    /* Additional operations to prevent dead code elimination */
    result += (result % 2 == 0) ? result : -result;
    return result;
}

/* Outer loop with pipelining opportunities */
#pragma GCC unroll 4
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static long outer_loop_pipelining(int size) {
    long total = 0;
    double* array = malloc(size * sizeof(double));
    
    if (!array) return 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; ++i) {
        array[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    
    /* Outer loop that should trigger pipelining */
    for (int outer = 0; outer < 100; ++outer) {
        double accum = 0.0;
        
        /* Inner loop with dependencies */
        for (int i = 1; i < size; ++i) {
            array[i] = array[i - 1] * 0.99 + sin(outer * 0.01);
            accum += array[i];
            
            /* Periodic inline assembly */
            if (i % 16 == 0) {
                asm volatile ("# Pipeline marker %0" : : "r"(i) : "memory");
            }
        }
        
        total += (long)(accum * 1000);
    }
    
    free(array);
    return total;
}

/* Main benchmark function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
int main() {
    const int N = 512;
    const int MIXED_SIZE = 1000;
    const int SIMD_SIZE = 1024;
    
    /* Initialize with random but deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    double* a = malloc(N * sizeof(double));
    double* b = malloc(N * sizeof(double));
    struct MixedData* mixed = malloc(MIXED_SIZE * sizeof(struct MixedData));
    float* src = malloc(SIMD_SIZE * sizeof(float));
    float* dst = malloc(SIMD_SIZE * sizeof(float));
    
    if (!a || !b || !mixed || !src || !dst) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100 * 0.1;
        b[i] = rand() % 100 * 0.2;
    }
    
    for (int i = 0; i < MIXED_SIZE; ++i) {
        mixed[i].id = rand() % 200;
        mixed[i].value = rand() % 100 * 0.5;
        mixed[i].tag = 'A' + (rand() % 26);
        mixed[i].aux = rand() % 100 * 0.01f;
        mixed[i].counter = rand() % 1000;
    }
    
    for (int i = 0; i < SIMD_SIZE; ++i) {
        src[i] = (rand() % 100) * 0.01f;
    }
    
    long checksum = 0;
    
    /* Execute all complex patterns to trigger scheduler activity */
    clock_t start = clock();
    
    /* 1. Loop-carried dependencies */
    complex_loop_carried_deps(N, a, b);
    
    /* 2. Mixed memory patterns */
    mixed_memory_patterns(mixed, MIXED_SIZE);
    
    /* 3. SIMD processing */
    simd_processing(src, dst, SIMD_SIZE);
    
    /* 4. Complex control flow */
    for (int i = 0; i < 1000; ++i) {
        checksum += jump_table_computation(i, i % 5);
    }
    
    /* 5. Switch statements */
    for (int i = 0; i < 500; ++i) {
        checksum += complex_switch(i % 250);
    }
    
    /* 6. Outer loop pipelining */
    checksum += outer_loop_pipelining(256);
    
    /* Combine results to prevent dead code elimination */
    for (int i = 0; i < N; ++i) {
        checksum += (long)(a[i] * 1000);
        checksum += (long)(b[i] * 1000);
    }
    
    for (int i = 0; i < SIMD_SIZE; i += 8) {
        checksum += (long)(dst[i] * 1000);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Checksum: %ld\n", checksum);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Selective scheduler debug output should be triggered\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(mixed);
    free(src);
    free(dst);
    
    return 0;
}
