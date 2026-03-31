/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC,
 * specifically targeting dump_insn_rtx_1 calls in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_dependencies(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts - creates complex dependency chains */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed computations */
            acc += in[j] * (j % 2 ? 0.5 : 2.0) + sin(in[i] * 0.01);
        }
        /* Conditional move operation */
        out[i] = (acc > 100.0) ? acc : acc * 0.9;
    }
}

/* Mixed data structure with non-contiguous access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float extra[3];
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_memory_access(struct MixedData* data, int count) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < count - 1; i += 2) {
        /* Pointer arithmetic with casting */
        data[i].value = data[i + 1].id * 0.5 + (double)data[i].tag;
        
        /* Complex addressing mode */
        float* ptr = &data[i].extra[1];
        *ptr = (float)data[i].value * 0.25f;
        
        /* Inline assembly with clobbers - forces scheduler constraints */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : : : "rax", "rbx", "rcx", "memory"
        );
    }
}

/* SIMD operations using intrinsics */
__attribute__((target("sse2")))
static void simd_operations(float* restrict dst, const float* restrict src, int len) {
    int i;
    /* Process with SIMD where possible */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 a = _mm_loadu_ps(&src[i]);
        __m128 b = _mm_loadu_ps(&src[i + 4 < len ? i + 4 : 0]);
        __m128 c = _mm_add_ps(a, b);
        __m128 d = _mm_mul_ps(c, _mm_set1_ps(0.5f));
        _mm_storeu_ps(&dst[i], d);
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * 0.5f + (i % 3 ? 0.1f : -0.1f);
    }
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static int computed_goto_pattern(int x) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_default
    };
    
    int idx = (x >= 0 && x < 6) ? x : 6;
    goto *jump_table[idx];
    
case_0:
    return x * 2;
case_1:
    return x + 100;
case_2:
    return x / 2;
case_3:
    return x * x;
case_4:
    return x | 0xFF;
case_5:
    return x & 0x0F;
case_default:
    return -x;
}

/* Switch statement with mixed density patterns */
__attribute__((optimize("O2")))
static double switch_dense_sparse(int val) {
    double result = 0.0;
    
    /* Dense switch for small values */
    switch (val % 10) {
        case 0: result = sin(val); break;
        case 1: result = cos(val); break;
        case 2: result = tan(val * 0.1); break;
        case 3: result = exp(val * 0.01); break;
        case 4: result = log(val + 1.0); break;
        case 5: result = sqrt(val); break;
        case 6: result = pow(val, 1.5); break;
        case 7: result = atan(val); break;
        case 8: result = asin(val * 0.01); break;
        case 9: result = acos(val * 0.01); break;
    }
    
    /* Sparse switch for larger values */
    switch (val) {
        case 100: result += 1.0; break;
        case 200: result += 2.0; break;
        case 300: result += 3.0; break;
        case 1000: result += 10.0; break;
        case 2000: result += 20.0; break;
        default: result += 0.5; break;
    }
    
    return result;
}

/* Loop with pragma unrolling */
#pragma GCC unroll 4
__attribute__((optimize("O3", "funroll-loops")))
static void unrolled_loop_computation(int* data, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        /* Mix of operations to create varied RTL */
        data[i] = (data[i] * 3 + 7) & 0xFF;
        sum += data[i];
        
        /* Function call within loop */
        data[i] ^= (int)sqrt(fabs((double)data[i]));
        
        /* Ternary operator generating conditional moves */
        data[i] = (sum > 1000) ? data[i] >> 2 : data[i] << 2;
    }
}

/* Main computational kernel that combines all patterns */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static double compute_kernel(int iterations) {
    const int ARRAY_SIZE = 1024;
    double* array1 = (double*)aligned_alloc(16, ARRAY_SIZE * sizeof(double));
    double* array2 = (double*)aligned_alloc(16, ARRAY_SIZE * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(100 * sizeof(struct MixedData));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with patterned data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array1[i] = sin(i * 0.01) + cos(i * 0.02);
        float_data[i] = (float)(i * 0.03);
        int_data[i] = i * 7;
    }
    
    for (int i = 0; i < 100; ++i) {
        mixed[i].id = i;
        mixed[i].value = i * 0.5;
        mixed[i].tag = (char)('A' + (i % 26));
        for (int j = 0; j < 3; ++j) {
            mixed[i].extra[j] = (float)(i * j * 0.1);
        }
    }
    
    double total = 0.0;
    
    /* Execute multiple patterns in sequence */
    for (int iter = 0; iter < iterations; ++iter) {
        /* Pattern 1: Complex loop dependencies */
        complex_loop_dependencies(ARRAY_SIZE / 2, array2, array1);
        
        /* Pattern 2: Mixed memory access with inline asm */
        mixed_memory_access(mixed, 100);
        
        /* Pattern 3: SIMD operations */
        simd_operations(float_data, float_data, ARRAY_SIZE);
        
        /* Pattern 4: Computed goto */
        int goto_result = computed_goto_pattern(iter % 10);
        
        /* Pattern 5: Switch statements */
        double switch_result = switch_dense_sparse(iter);
        
        /* Pattern 6: Unrolled loops */
        unrolled_loop_computation(int_data, ARRAY_SIZE / 4);
        
        /* Combine results to prevent dead code elimination */
        total += array2[iter % ARRAY_SIZE] + mixed[iter % 100].value 
                 + float_data[iter % ARRAY_SIZE] + goto_result 
                 + switch_result + int_data[iter % ARRAY_SIZE];
        
        /* Additional inline asm with clobbers */
        asm volatile (
            "movq %%rax, %%rbx\n\t"
            "addq $1, %%rax\n\t"
            : : : "rax", "rbx", "cc"
        );
    }
    
    free(array1);
    free(array2);
    free(mixed);
    free(float_data);
    free(int_data);
    
    return total;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    printf("Running selective scheduler trigger program...\n");
    printf("Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse2 sel-sched-trigger.c -lm\n");
    printf("Or for maximum scheduling: gcc -O3 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -funroll-loops -march=native sel-sched-trigger.c -lm\n\n");
    
    double result = compute_kernel(iterations);
    
    printf("Computation result: %f\n", result);
    printf("(This value should vary with different iteration counts)\n");
    
    return 0;
}
