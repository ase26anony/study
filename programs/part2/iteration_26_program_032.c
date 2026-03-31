/* sel-sched-trigger.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -lm -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#pragma GCC optimize ("O3")
#pragma GCC optimize ("unroll-loops")

/* Mixed data type structure for complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long timestamp;
};

/* Helper function with attribute to force specific optimization */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double process_value(double x, int iter) {
    /* Conditional moves and math functions */
    double result = (iter % 2) ? sin(x) : cos(x);
    result = (x > 0.5) ? result * 2.0 : result / 2.0;
    
    /* Inline assembly with clobbers to force scheduler work */
    asm volatile ("# Force register pressure\n\t"
                  "nop\n\t" : : : "rax", "rbx", "rcx", "memory");
    
    return result;
}

/* Function with loop-carried dependencies */
__attribute__((hot, optimize("O3")))
static long nested_loop_compute(struct MixedData* data, int N) {
    long total = 0;
    
    /* Outer loop with varying trip counts */
    for (int i = 1; i < N; ++i) {
        double acc = 0.0;
        
        /* Inner loop with data-dependent bounds */
        for (int j = 0; j < i; ++j) {
            /* Complex addressing with mixed types */
            int idx = (i * 17 + j * 13) % N;
            data[idx].value = process_value(data[idx].value, j);
            
            /* Non-contiguous memory access */
            if (j % 3 == 0) {
                data[idx].weight = (float)data[idx].value * 0.5f;
                total += (long)(data[idx].weight * 1000);
            }
            
            /* More inline assembly */
            asm volatile ("# Inner loop clobber\n\t"
                          "nop\n\t" : : : "r8", "r9", "r10", "memory");
        }
        
        /* Function call with varying arguments */
        data[i].value = pow(data[i].value, 1.0 + (i % 5) * 0.1);
    }
    
    return total;
}

/* SIMD processing function */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void simd_processing(int* src, int* dst, int len) {
    /* Process with SSE intrinsics */
    for (int i = 0; i < len; i += 4) {
        if (i + 4 <= len) {
            __m128i a = _mm_loadu_si128((__m128i*)(src + i));
            __m128i b = _mm_add_epi32(a, a);
            __m128i c = _mm_mullo_epi16(b, _mm_set1_epi32(3));
            _mm_storeu_si128((__m128i*)(dst + i), c);
        } else {
            /* Scalar fallback */
            for (int j = i; j < len; ++j) {
                dst[j] = src[j] * 6;
            }
        }
        
        /* Conditional move pattern */
        int temp = (i % 8 == 0) ? dst[i] : src[i];
        dst[i] = (temp > 1000) ? temp / 2 : temp * 2;
    }
}

/* Function with computed goto for complex control flow */
__attribute__((noinline))
static int jump_table_compute(int x) {
    static void* jtable[] = { &&case0, &&case1, &&case2, &&case3, 
                              &&case4, &&case5, &&default_case };
    
    int idx = x % 7;
    int result = 0;
    
    goto *jtable[idx];
    
case0:
    result = x * 2;
    goto end;
case1:
    result = x + x;
    goto end;
case2:
    result = x | 0xFF;
    goto end;
case3:
    result = x ^ 0xAAAA;
    goto end;
case4:
    result = x << 3;
    goto end;
case5:
    result = x >> 1;
    goto end;
default_case:
    result = ~x;
    goto end;
    
end:
    return result;
}

/* Switch statement with mixed density */
__attribute__((optimize("O2")))
static int dense_sparse_switch(int val) {
    int output = 0;
    
    switch (val) {
        /* Dense range */
        case 0:  output = 1; break;
        case 1:  output = 2; break;
        case 2:  output = 3; break;
        case 3:  output = 5; break;
        case 4:  output = 7; break;
        case 5:  output = 11; break;
        
        /* Sparse range */
        case 10: output = 13; break;
        case 20: output = 17; break;
        case 50: output = 19; break;
        case 100: output = 23; break;
        case 200: output = 29; break;
        
        /* Very sparse */
        case 1000: output = 31; break;
        case 5000: output = 37; break;
        
        default:
            output = val % 37;
            break;
    }
    
    return output;
}

/* Main computational kernel */
__attribute__((optimize("O3", "fsel-sched-pipelining", 
                       "fsel-sched-pipelining-outer-loops")))
static unsigned long run_computations() {
    const int N = 512;
    const int ARR_SIZE = 1024;
    
    struct MixedData* data = (struct MixedData*)malloc(N * sizeof(struct MixedData));
    int* src_array = (int*)malloc(ARR_SIZE * sizeof(int));
    int* dst_array = (int*)malloc(ARR_SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data[i].id = i;
        data[i].value = sin(i * 0.1) + cos(i * 0.05);
        data[i].tag = (char)(i % 26 + 'A');
        data[i].weight = (float)(i % 100) / 100.0f;
        data[i].timestamp = i * 1000L;
    }
    
    for (int i = 0; i < ARR_SIZE; ++i) {
        src_array[i] = (i * 37) % 1000;
        dst_array[i] = 0;
    }
    
    unsigned long checksum = 0;
    
    /* Run nested loops with dependencies */
    checksum ^= (unsigned long)nested_loop_compute(data, N);
    
    /* Process with SIMD */
    simd_processing(src_array, dst_array, ARR_SIZE);
    
    /* Accumulate results from SIMD */
    for (int i = 0; i < ARR_SIZE; ++i) {
        checksum += dst_array[i];
    }
    
    /* Use jump table */
    for (int i = 0; i < 100; ++i) {
        checksum ^= jump_table_compute(i);
    }
    
    /* Dense/sparse switch */
    for (int i = 0; i < 200; i += 3) {
        checksum += dense_sparse_switch(i % 5001);
    }
    
    /* Additional complex loop with unrolling hint */
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        if (i % 2 == 0) {
            data[i].value = data[i].value * 0.9 + data[(i + 1) % N].value * 0.1;
        } else {
            data[i].value = data[i].value * 0.8 + data[(i - 1 + N) % N].value * 0.2;
        }
        
        /* More inline assembly */
        asm volatile ("# Final loop clobber\n\t"
                      "nop\n\t" : : : "r12", "r13", "r14", "r15", "memory");
    }
    
    /* Final accumulation */
    for (int i = 0; i < N; ++i) {
        checksum += (unsigned long)(fabs(data[i].value) * 1000.0);
    }
    
    free(data);
    free(src_array);
    free(dst_array);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run multiple iterations to increase scheduling opportunities */
    unsigned long final_checksum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        printf("Iteration %d: ", iter + 1);
        fflush(stdout);
        
        unsigned long result = run_computations();
        final_checksum ^= result;
        
        printf("checksum = 0x%016lx\n", result);
    }
    
    printf("Final checksum: 0x%016lx\n", final_checksum);
    printf("Program completed.\n");
    
    return 0;
}
