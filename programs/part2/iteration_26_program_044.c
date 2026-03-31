/* sel-sched-trigger.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -lm -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed types */
            acc += in[j] * (j % 2 ? 0.5 : 2.0);
            
            /* Inline assembly with clobbers to force scheduling constraints */
            asm volatile ("# Force register pressure" : : : "rax", "rbx", "rcx", "rdx", "memory");
        }
        /* Conditional move pattern */
        out[i] = (acc > 100.0) ? acc : acc * 0.9;
    }
}

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int a;
    double b;
    char c[3];
    float d;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void pointer_arithmetic_and_casts(struct MixedData* arr, int size) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < size - 1; i += 2) {
        /* Pointer arithmetic and type casting */
        double* dbl_ptr = &arr[i].b;
        float* flt_ptr = &arr[i + 1].d;
        
        /* Complex expression with function calls */
        *dbl_ptr = sin(arr[i].a * 0.01) + pow(1.5, arr[i].a % 10);
        *flt_ptr = (float)(*dbl_ptr) * (i % 3 ? 0.7f : 1.3f);
        
        /* More inline assembly */
        asm volatile ("nop\n\tnop\n\tnop" : : : "memory");
    }
}

/* SIMD operations using intrinsics */
__attribute__((target("sse2")))
static void simd_operations(int* dst, const int* src, int len) {
    /* Process in SIMD chunks */
    for (int i = 0; i < len - 3; i += 4) {
        __m128i vec = _mm_loadu_si128((__m128i*)(src + i));
        __m128i result = _mm_add_epi32(vec, _mm_slli_epi32(vec, 1));
        result = _mm_xor_si128(result, _mm_set1_epi32(0x55555555));
        _mm_storeu_si128((__m128i*)(dst + i), result);
    }
    
    /* Scalar tail processing */
    for (int i = len & ~3; i < len; ++i) {
        dst[i] = src[i] ^ 0x55555555;
    }
}

/* Function with computed goto for indirect branching */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static int computed_goto_pattern(int x) {
    static const void* jtable[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    int idx = x % 5;
    int result = 0;
    
    /* Indirect branch - challenging for scheduler */
    goto *jtable[idx];
    
case0:
    result = x * 2;
    goto end;
case1:
    result = x + x;
    goto end;
case2:
    result = x << 1;
    goto end;
case3:
    result = x * 3 - x;
    goto end;
default_case:
    result = x;
    goto end;
    
end:
    return result;
}

/* Switch statement with mixed density cases */
__attribute__((optimize("O3")))
static int dense_sparse_switch(int val) {
    int result = 0;
    
    /* Mix of dense and sparse cases */
    switch (val) {
        case 0:  result = 1; break;
        case 1:  result = 2; break;
        case 2:  result = 3; break;  /* Dense region */
        case 3:  result = 5; break;
        case 4:  result = 8; break;
        case 100: result = 13; break; /* Sparse jump */
        case 200: result = 21; break;
        case 1000: result = 34; break; /* Very sparse */
        default: result = val % 7; break;
    }
    
    /* Ternary operator for conditional move */
    return (result > 20) ? result * 2 : result + 1;
}

/* Loop with pragma unrolling */
__attribute__((optimize("O3", "funroll-loops")))
static void unrolled_loop_processing(float* data, int count) {
    #pragma GCC unroll 4
    for (int i = 0; i < count; ++i) {
        /* Mix of operations to create varied RTL */
        data[i] = data[i] * 1.1f + 0.5f;
        data[i] = (data[i] > 1.0f) ? data[i] - 1.0f : data[i];
        data[i] = sinf(data[i] * 3.14159f);
    }
}

/* Main computational kernel */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static unsigned long benchmark_kernel(int iterations) {
    const int ARRAY_SIZE = 1024;
    double* array1 = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* array2 = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* int_array1 = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* int_array2 = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    struct MixedData* mixed = aligned_alloc(64, ARRAY_SIZE * sizeof(struct MixedData));
    float* float_data = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array1[i] = (i % 100) * 0.01;
        int_array1[i] = i ^ 0x12345678;
        mixed[i].a = i;
        mixed[i].b = i * 0.5;
        mixed[i].c[0] = i & 0xFF;
        float_data[i] = i * 0.01f;
    }
    
    unsigned long checksum = 0;
    
    /* Perform multiple passes with different computation patterns */
    for (int iter = 0; iter < iterations; ++iter) {
        /* 1. Complex loop-carried dependencies */
        complex_loop_carried_deps(ARRAY_SIZE, array2, array1);
        
        /* 2. Pointer arithmetic and type casting */
        pointer_arithmetic_and_casts(mixed, ARRAY_SIZE);
        
        /* 3. SIMD operations */
        simd_operations(int_array2, int_array1, ARRAY_SIZE);
        
        /* 4. Computed goto pattern */
        for (int i = 0; i < 100; ++i) {
            checksum ^= computed_goto_pattern(i + iter);
        }
        
        /* 5. Dense/sparse switch */
        for (int i = 0; i < 50; ++i) {
            checksum += dense_sparse_switch((i * 17 + iter) % 1500);
        }
        
        /* 6. Unrolled loop */
        unrolled_loop_processing(float_data, ARRAY_SIZE);
        
        /* Mix data between iterations */
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            array1[i] = array2[i] * 0.99 + mixed[i].b * 0.01;
            int_array1[i] = int_array2[i] ^ (iter & 0xFF);
        }
        
        /* Additional inline assembly for register pressure */
        asm volatile ("# Iteration barrier" : : : "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
    
    /* Final checksum computation */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        checksum ^= *(unsigned long*)&array2[i];
        checksum += int_array2[i];
        checksum ^= *(unsigned int*)&float_data[i];
    }
    
    free(array1);
    free(array2);
    free(int_array1);
    free(int_array2);
    free(mixed);
    free(float_data);
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    printf("Running selective scheduler trigger with %d iterations...\n", iterations);
    
    /* Force selective scheduler activation with complex control flow */
    unsigned long result = 0;
    for (int run = 0; run < 3; ++run) {
        result += benchmark_kernel(iterations);
        
        /* Vary the control flow between runs */
        if (run == 1) {
            /* Insert some system calls that might affect scheduling */
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "Run %d complete", run);
        }
    }
    
    printf("Final checksum: 0x%016lx\n", result);
    printf("(Note: Compile with -fsel-sched-debug to see scheduler diagnostics)\n");
    
    return 0;
}
