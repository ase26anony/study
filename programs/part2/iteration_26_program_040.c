/* sel-sched-trigger.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -o trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#define N 256
#define M 128
#define ITERATIONS 1000

/* Mixed data structure for non-contiguous access */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long timestamp;
};

/* Helper function with varying arguments */
static double __attribute__((optimize("O2"))) compute_transform(double x, int scale, char mode) {
    if (mode == 's') {
        return sin(x * scale);
    } else if (mode == 'c') {
        return cos(x / scale);
    } else {
        return pow(x, 1.0 / scale);
    }
}

/* Function with complex loop-carried dependencies */
void __attribute__((optimize("O2"))) nested_dependency_test(struct MixedData* data, int size) {
    volatile int sum = 0;
    
    /* Outer loop with varying trip count for inner loop */
    for (int i = 1; i < size; ++i) {
        /* Inner loop dependent on outer index */
        for (int j = 0; j < i; ++j) {
            /* Data-dependent operations with mixed types */
            data[j].value = data[i].value * 0.5 + j * 0.1;
            data[j].weight = (float)((data[i].id + j) % 100) * 0.01f;
            
            /* Conditional move operations */
            data[j].id = (j % 3 == 0) ? data[i].id : data[j].id;
            data[j].tag = (data[j].value > 0.5) ? 'A' : 'B';
            
            /* Inline assembly with clobbers to force scheduling constraints */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "memory"
            );
            
            sum += data[j].id;
        }
        
        /* Function call within loop */
        data[i].value = compute_transform(data[i].value, i % 10 + 1, 's');
    }
    
    /* Prevent dead code elimination */
    printf("Nested sum: %d\n", sum);
}

/* SIMD-intensive computation */
void __attribute__((optimize("O3"))) simd_processing(float* src, float* dst, int len) {
    #pragma GCC unroll 4
    for (int i = 0; i < len - 3; i += 4) {
        /* Load unaligned data */
        __m128 vec = _mm_loadu_ps(&src[i]);
        
        /* SIMD operations */
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 scaled = _mm_mul_ps(squared, _mm_set1_ps(0.5f));
        
        /* Conditional operation simulation */
        __m128 mask = _mm_cmpgt_ps(vec, _mm_setzero_ps());
        __m128 result = _mm_blendv_ps(scaled, vec, mask);
        
        /* Store result */
        _mm_storeu_ps(&dst[i], result);
        
        /* More inline assembly with different clobbers */
        asm volatile (
            "mfence\n\t"
            : : : "memory", "xmm0", "xmm1"
        );
    }
    
    /* Handle remainder */
    for (int i = (len / 4) * 4; i < len; ++i) {
        dst[i] = (src[i] > 0) ? src[i] * src[i] * 0.5f : src[i];
    }
}

/* Complex control flow with computed goto */
void __attribute__((optimize("O2"))) jump_table_test(int* results, int size) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    for (int i = 0; i < size; ++i) {
        int idx = results[i] & 0x7;
        
        /* Computed goto */
        goto *jump_table[idx];
        
    label_0:
        results[i] = results[i] * 2;
        continue;
    label_1:
        results[i] = results[i] + results[i-1];
        continue;
    label_2:
        results[i] = results[i] / (results[i-2] + 1);
        continue;
    label_3:
        results[i] = results[i] ^ 0x55AA55AA;
        continue;
    label_4:
        results[i] = ~results[i];
        continue;
    label_5:
        results[i] = results[i] << (i % 8);
        continue;
    label_6:
        results[i] = results[i] >> (results[i-1] % 8);
        continue;
    label_7:
        results[i] = (results[i] + results[i-1] + results[i-2]) / 3;
        continue;
    }
}

/* Switch statement with mixed dense/sparse cases */
int __attribute__((optimize("O2"))) sparse_switch(int x) {
    int result = x;
    
    switch (x) {
        /* Dense range */
        case 0:  result = x * 2; break;
        case 1:  result = x + 100; break;
        case 2:  result = x - 50; break;
        case 3:  result = x ^ 0xFF; break;
        case 4:  result = x << 3; break;
        
        /* Sparse jump */
        case 10: result = x / 2; break;
        case 20: result = x % 17; break;
        case 50: result = x * x; break;
        case 100: result = sqrt(x); break;
        case 200: result = x | 0x0F0F0F0F; break;
        
        /* Very sparse */
        case 1000: result = x & 0x55555555; break;
        case 2000: result = ~x; break;
        case 5000: result = x + x * 2 + x * 3; break;
        
        default:
            /* Complex default with function call */
            result = (int)compute_transform(x, 2, 'c');
            break;
    }
    
    return result;
}

/* Main computational kernel */
void __attribute__((optimize("O2"))) computational_kernel() {
    struct MixedData data[N];
    float src_array[M], dst_array[M];
    int results[N];
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; ++i) {
        data[i].id = i;
        data[i].value = sin(i * 0.1);
        data[i].tag = 'A' + (i % 26);
        data[i].weight = (i % 100) * 0.01f;
        data[i].timestamp = i * 1000;
        results[i] = i * 3;
    }
    
    for (int i = 0; i < M; ++i) {
        src_array[i] = (i % 2 == 0) ? i * 0.1f : -i * 0.1f;
    }
    
    long long checksum = 0;
    
    /* Multiple iterations to create sustained pressure */
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        /* Vary the computation each iteration */
        int offset = iter % N;
        
        /* Call different computational patterns */
        nested_dependency_test(data + offset, N - offset);
        
        simd_processing(src_array, dst_array, M);
        
        /* Process results with jump table */
        jump_table_test(results, N);
        
        /* Apply sparse switch to each element */
        for (int i = 0; i < N; i += 4) {
            results[i] = sparse_switch(results[i] + iter);
            
            /* Mixed memory access pattern */
            data[i].value = data[(i + 1) % N].value * 0.9 + 
                           data[(i + 2) % N].value * 0.1;
            
            /* More inline assembly */
            asm volatile (
                "pause\n\t"
                : : : "memory"
            );
        }
        
        /* Accumulate checksum to prevent elimination */
        for (int i = 0; i < N; ++i) {
            checksum += data[i].id + (int)data[i].value + results[i];
        }
        
        /* Function pointer call to add variety */
        double (*func_ptr)(double, int, char) = compute_transform;
        data[iter % N].value = func_ptr(data[iter % N].value, iter % 5 + 1, 's');
    }
    
    /* Final output to ensure computation isn't optimized away */
    printf("Final checksum: %lld\n", checksum);
    printf("Sample values: data[0].value=%f, results[100]=%d\n", 
           data[0].value, results[100]);
}

/* Additional complex loop for outer-loop pipelining */
void __attribute__((optimize("O3"))) outer_loop_pipelining_test() {
    double matrix[64][64];
    double result[64][64];
    
    /* Initialize matrix */
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            matrix[i][j] = (i + j) * 0.01;
        }
    }
    
    /* Triple nested loops with dependencies */
    #pragma GCC unroll 2
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            double sum = 0.0;
            for (int k = 0; k < 64; ++k) {
                /* Complex dependency chain */
                sum += matrix[i][k] * matrix[k][j];
                
                /* Conditional in innermost loop */
                if ((i + j + k) % 3 == 0) {
                    sum *= 0.99;
                }
                
                /* Memory barrier */
                asm volatile ("" ::: "memory");
            }
            result[i][j] = sum;
            
            /* Function call with math library */
            result[i][j] = sin(result[i][j]) + cos(result[i][j] * 0.5);
        }
    }
    
    /* Verify computation */
    double total = 0;
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            total += result[i][j];
        }
    }
    printf("Matrix total: %f\n", total);
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Enable optimization hints */
    #pragma GCC optimize ("O2")
    
    /* Run computational kernels */
    computational_kernel();
    
    /* Additional test for outer-loop pipelining */
    outer_loop_pipelining_test();
    
    /* More varied computation */
    for (int phase = 0; phase < 3; ++phase) {
        printf("Phase %d\n", phase);
        
        /* Allocate and process dynamic data */
        int* dynamic_array = malloc(1000 * sizeof(int));
        for (int i = 0; i < 1000; ++i) {
            dynamic_array[i] = (i * phase) % 97;
            
            /* Mix of operations */
            dynamic_array[i] = (dynamic_array[i] > 50) ? 
                               dynamic_array[i] * 2 : 
                               dynamic_array[i] / 2;
                               
            /* Pointer arithmetic */
            int* ptr = &dynamic_array[i];
            *ptr += *(ptr - (i > 0 ? 1 : 0));
        }
        
        free(dynamic_array);
    }
    
    printf("Program completed.\n");
    return 0;
}
