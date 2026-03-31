/* sel-sched-trigger.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Enable selective scheduling optimizations on specific functions */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static void complex_loop_carried_deps(int N, double* restrict out, const double* restrict in) {
    /* Nested loops with varying trip counts - creates complex scheduling decisions */
    for (int i = 0; i < N; ++i) {
        double acc = 0.0;
        /* Inner loop count depends on outer index - creates data-dependent scheduling */
        for (int j = 0; j < i; ++j) {
            /* Mixed operations with dependencies */
            acc += in[j] * (j % 2 ? 0.5 : 2.0);
            /* Inline assembly with clobbers to force register pressure */
            asm volatile ("# Force register clobber" : : : "rax", "rbx", "rcx", "rdx", "memory");
        }
        out[i] = acc + (i % 3 ? sin(acc) : cos(acc));
    }
}

/* Structure with mixed data types for non-contiguous access */
struct MixedData {
    int id;
    double value;
    char tag;
    float extra;
    long counter;
};

__attribute__((optimize("O3", "fsel-sched-pipelining")))
static void mixed_data_access(struct MixedData* data, int count) {
    /* Non-contiguous, strided access pattern */
    for (int i = 0; i < count; i += 2) {
        /* Pointer arithmetic with casting */
        struct MixedData* curr = &data[i];
        struct MixedData* next = &data[i + 1];
        
        /* Data-dependent operations with mixed types */
        curr->value = next->id * 0.5 + (curr->tag ? 1.0 : -1.0);
        next->value = curr->id * 2.0 - (next->tag ? 0.5 : 1.5);
        
        /* Conditional move operations */
        curr->extra = (curr->value > 0) ? sqrt(fabs(curr->value)) : -sqrt(fabs(curr->value));
        next->extra = (next->value < 0) ? pow(fabs(next->value), 1.5) : pow(next->value, 0.5);
        
        /* Function calls with varying arguments */
        curr->counter += (long)exp(curr->extra);
        next->counter -= (long)log(fabs(next->extra) + 1.0);
    }
}

/* SIMD operations using intrinsics */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
static void simd_processing(float* restrict dst, const float* restrict src, int len) {
    int i;
    /* Process with SIMD intrinsics */
    for (i = 0; i + 4 <= len; i += 4) {
        __m128 a = _mm_loadu_ps(&src[i]);
        __m128 b = _mm_loadu_ps(&src[i + 4 < len ? i + 4 : i]);
        __m128 c = _mm_add_ps(a, b);
        __m128 d = _mm_mul_ps(c, _mm_set1_ps(0.5f));
        _mm_storeu_ps(&dst[i], d);
        
        /* More complex SIMD operations */
        __m128 e = _mm_sub_ps(a, b);
        __m128 f = _mm_mul_ps(e, e);
        __m128 g = _mm_sqrt_ps(f);
        _mm_storeu_ps(&dst[i + 8 < len ? i + 8 : i], g);
    }
    
    /* Scalar tail processing */
    for (; i < len; ++i) {
        dst[i] = src[i] * (i % 3 ? 0.7f : 1.3f) + (i % 2 ? -0.5f : 0.5f);
    }
}

/* Complex control flow with computed goto */
__attribute__((optimize("O2")))
static int computed_goto_dispatch(int op, int x, int y) {
    static void* jump_table[] = {
        &&add_op, &&sub_op, &&mul_op, &&div_op, 
        &&mod_op, &&and_op, &&or_op, &&xor_op
    };
    
    if (op < 0 || op >= 8) return 0;
    
    goto *jump_table[op];
    
add_op:
    return x + y;
sub_op:
    return x - y;
mul_op:
    return x * y;
div_op:
    return y != 0 ? x / y : 0;
mod_op:
    return y != 0 ? x % y : 0;
and_op:
    return x & y;
or_op:
    return x | y;
xor_op:
    return x ^ y;
}

/* Switch with mixed dense/sparse cases */
__attribute__((optimize("O3", "funroll-loops")))
static double switch_complex(int code, double val) {
    double result = val;
    
    switch (code) {
        /* Dense range */
        case 0: result = sin(val); break;
        case 1: result = cos(val); break;
        case 2: result = tan(val); break;
        case 3: result = exp(val); break;
        case 4: result = log(val + 1.0); break;
        
        /* Sparse range */
        case 10: result = val * val; break;
        case 20: result = sqrt(val); break;
        case 30: result = pow(val, 1.5); break;
        case 100: result = 1.0 / (val + 0.001); break;
        case 200: result = fmod(val, 3.14159); break;
        
        default:
            /* Complex default case with loop */
            #pragma GCC unroll 4
            for (int i = 0; i < 4; ++i) {
                result = (result + val) * 0.5;
            }
            break;
    }
    
    return result;
}

/* Main computational kernel with all patterns combined */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static double compute_kernel(int size) {
    double* array1 = malloc(size * sizeof(double));
    double* array2 = malloc(size * sizeof(double));
    struct MixedData* mixed = malloc(size * sizeof(struct MixedData));
    float* simd_src = malloc(size * sizeof(float));
    float* simd_dst = malloc(size * sizeof(float));
    
    /* Initialize with patterned data */
    for (int i = 0; i < size; ++i) {
        array1[i] = (i % 100) * 0.01;
        array2[i] = (i % 50) * 0.02;
        mixed[i].id = i;
        mixed[i].value = i * 0.1;
        mixed[i].tag = i % 2;
        mixed[i].extra = i * 0.01f;
        mixed[i].counter = i;
        simd_src[i] = i * 0.001f;
    }
    
    double total = 0.0;
    
    /* Execute all patterns to maximize scheduler activity */
    complex_loop_carried_deps(size / 2, array2, array1);
    mixed_data_access(mixed, size);
    simd_processing(simd_dst, simd_src, size);
    
    /* Combine results with complex control flow */
    for (int i = 0; i < size; ++i) {
        int op = i % 8;
        int int_val = computed_goto_dispatch(op, i, size - i);
        
        double switch_val = switch_complex(i % 250, array2[i % (size / 2)]);
        
        /* Final accumulation with data-dependent operations */
        total += array1[i] + array2[i % (size / 2)] + 
                 mixed[i].value + simd_dst[i] + 
                 int_val * 0.01 + switch_val;
                 
        /* Periodic inline assembly to disrupt scheduling */
        if (i % 17 == 0) {
            asm volatile ("# Periodic scheduling boundary" : : : "rax", "rbx", "rcx", "memory");
        }
    }
    
    free(array1);
    free(array2);
    free(mixed);
    free(simd_src);
    free(simd_dst);
    
    return total;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Multiple iterations to ensure sustained scheduling activity */
    double final_result = 0.0;
    for (int iter = 0; iter < 3; ++iter) {
        printf("Iteration %d: ", iter);
        
        /* Varying sizes to create different scheduling scenarios */
        int size = 1000 + iter * 500;
        double result = compute_kernel(size);
        final_result += result;
        
        printf("Result = %.6f\n", result);
        
        /* Force memory operations between iterations */
        asm volatile ("# Inter-iteration barrier" : : : "memory");
    }
    
    printf("Final checksum: %.12f\n", final_result);
    printf("Program completed.\n");
    
    return 0;
}
