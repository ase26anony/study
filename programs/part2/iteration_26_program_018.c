/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 -march=haswell sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

#define N 256
#define M 128
#define ITERATIONS 1000

/* Mixed data type structure for complex memory access */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long long timestamp;
};

/* Global arrays to prevent optimization */
struct MixedData global_data[N];
double matrix_a[N][M];
double matrix_b[M][N];
float vector[N];
int results[N];
__m128i simd_buffer[N/4];

/* Attribute to ensure optimization level */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static void complex_loop_carried_deps(int limit) {
    volatile int sum = 0;
    
    /* Loop-carried dependencies with varying trip counts */
    for (int i = 1; i < limit; ++i) {
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed types */
            global_data[i].value = global_data[j].value * 0.75 + 
                                  (global_data[i].id % 100) * 0.25;
            
            /* Inline assembly with clobbers to force scheduler work */
            asm volatile (
                "movl %0, %%eax\n\t"
                "addl $1, %%eax\n\t"
                "movl %%eax, %0"
                : "+r" (sum)
                : 
                : "eax", "cc"
            );
        }
        
        /* Conditional move operations */
        int temp = (i % 2 == 0) ? sum : -sum;
        global_data[i].id = (temp > 0) ? temp : (temp + 1000);
    }
}

__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void simd_mixed_operations(void) {
    /* Non-contiguous memory access pattern */
    for (int i = 0; i < N - 4; i += 2) {
        /* Load and process using SSE intrinsics */
        __m128d a = _mm_loadu_pd(&global_data[i].value);
        __m128d b = _mm_loadu_pd(&global_data[i+1].value);
        __m128d result = _mm_add_pd(_mm_mul_pd(a, _mm_set1_pd(0.5)), b);
        
        /* Store with barrier */
        _mm_storeu_pd(&global_data[i].value, result);
        
        /* More complex SIMD integer operations */
        __m128i vec1 = _mm_loadu_si128((__m128i*)&global_data[i].id);
        __m128i vec2 = _mm_slli_epi32(vec1, 2);
        _mm_storeu_si128((__m128i*)&global_data[i].id, vec2);
        
        /* Function calls within loop */
        vector[i] = (float)sin(global_data[i].value);
        vector[i+1] = (float)cos(global_data[i+1].value);
    }
}

/* Function with computed goto for complex control flow */
__attribute__((optimize("O2")))
static int computed_goto_pattern(int index) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    int result = 0;
    
    if (index >= 0 && index < 8) {
        goto *jump_table[index];
    }
    
label_0:
    result = global_data[0].id * 2;
    goto end;
    
label_1:
    result = (int)(global_data[1].value * 100);
    goto end;
    
label_2:
    result = global_data[2].tag + 256;
    goto end;
    
label_3:
    result = (int)sqrt(fabs(global_data[3].value));
    goto end;
    
label_4:
    result = -global_data[4].id;
    goto end;
    
label_5:
    result = (int)(pow(global_data[5].value, 1.5));
    goto end;
    
label_6:
    result = 0;
    for (int i = 0; i < 4; ++i) {
        result += global_data[i].id;
    }
    goto end;
    
label_7:
    result = (global_data[7].timestamp % 1000);
    goto end;
    
end:
    return result;
}

/* Matrix multiplication with loop unrolling pragma */
#pragma GCC unroll 4
__attribute__((optimize("O3", "funroll-loops")))
static void matrix_operations(void) {
    double temp[N][M];
    
    /* Nested loops with complex indexing */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            double sum = 0.0;
            
            /* Inner loop with dependency */
            for (int k = 0; k < M; ++k) {
                sum += matrix_a[i][k] * matrix_b[k][j];
                
                /* Conditional operation to create varied ILP */
                if ((k + i + j) % 3 == 0) {
                    sum *= 0.99;
                } else if ((k + i + j) % 5 == 0) {
                    sum += 0.01;
                }
            }
            
            temp[i][j] = sum;
            
            /* Inline assembly with memory clobber */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Copy back with barrier */
    memcpy(matrix_a, temp, sizeof(temp));
}

/* Switch statement with mixed case patterns */
__attribute__((optimize("O2")))
static int sparse_switch_demo(int val) {
    int result = 0;
    
    switch (val) {
        /* Dense range */
        case 0 ... 10:
            result = val * 10;
            break;
            
        /* Sparse cases */
        case 100:
            result = (int)(sin(val * 0.01) * 1000);
            break;
            
        case 500:
            result = (int)(global_data[val % N].value * 100);
            break;
            
        case 1000:
            result = computed_goto_pattern(val % 8);
            break;
            
        case 2000:
            /* Complex expression with multiple operations */
            result = (val & 0xFF) | ((val >> 8) << 16);
            break;
            
        case 3000 ... 3010:
            result = val - 3000 + 100;
            break;
            
        default:
            /* Ternary conditional move pattern */
            result = (val < 0) ? -val : 
                    (val > 10000) ? val % 1000 :
                    val * 2;
            break;
    }
    
    return result;
}

/* Main computational kernel */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static long long run_computations(void) {
    long long total = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        global_data[i].id = i;
        global_data[i].value = sin(i * 0.1);
        global_data[i].tag = (char)(i % 256);
        global_data[i].weight = (float)(i * 0.01);
        global_data[i].timestamp = i * 1000LL;
        
        for (int j = 0; j < M; ++j) {
            matrix_a[i][j] = (i + j) * 0.1;
            matrix_b[j][i] = (i - j) * 0.2;
        }
    }
    
    /* Run multiple patterns to increase scheduling complexity */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* Vary the patterns each iteration */
        int limit = 50 + (iter % 50);
        
        /* Pattern 1: Loop-carried dependencies */
        complex_loop_carried_deps(limit);
        
        /* Pattern 2: SIMD operations */
        if (iter % 3 == 0) {
            simd_mixed_operations();
        }
        
        /* Pattern 3: Matrix operations */
        if (iter % 5 == 0) {
            matrix_operations();
        }
        
        /* Pattern 4: Switch and computed goto */
        int switch_val = iter % 4000;
        results[iter % N] = sparse_switch_demo(switch_val);
        
        /* Pattern 5: Computed goto */
        if (iter % 7 == 0) {
            results[(iter + 1) % N] = computed_goto_pattern(iter % 8);
        }
        
        /* Accumulate results to prevent dead code elimination */
        total += global_data[iter % N].id + 
                (long long)(global_data[iter % N].value * 100) +
                results[iter % N];
    }
    
    return total;
}

int main(void) {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run computations multiple times to ensure scheduler activity */
    long long final_result = 0;
    
    for (int run = 0; run < 3; ++run) {
        long long run_result = run_computations();
        final_result ^= run_result;  /* Combine results */
        
        printf("Run %d: result = %lld\n", run, run_result);
        
        /* Add memory barrier between runs */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %lld\n", final_result);
    printf("Program completed. Check stderr for selective scheduler debug output.\n");
    
    return (final_result != 0) ? 0 : 1;
}
