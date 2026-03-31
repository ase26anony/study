/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic chains to create dependency chains */
static inline ALWAYS_INLINE double complex_chain_1(double a, double b, double c, double d) {
    double t1 = a * b + c;
    double t2 = t1 / (d + 1.0);
    double t3 = sin(t2) * cos(t1);
    double t4 = t3 * t3 - t2 * t2;
    return t4 + sqrt(fabs(t3));
}

static inline ALWAYS_INLINE int integer_chain_1(int a, int b, int c, int d) {
    int t1 = a * b + c;
    int t2 = t1 ^ d;
    int t3 = t2 * 0x5A827999;
    int t4 = (t3 >> 13) | (t3 << 19);
    int t5 = t4 + a * c - b * d;
    return t5 * 0x6ED9EBA1;
}

/* Memory operations with potential aliasing */
static inline ALWAYS_INLINE void memory_ops(int* restrict arr1, int* restrict arr2, 
                                           int* restrict arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr3[i] = arr1[i] * arr2[i] + arr3[i-1];
    }
}

/* Function with side effects (non-pure) to create scheduling barriers */
static int global_counter = 0;
static inline ALWAYS_INLINE int with_side_effect(int x) {
    global_counter += x;
    asm volatile("" ::: "memory");  /* Memory barrier */
    return global_counter;
}

/* Test function 1: Mixed integer/FP with dependency chains */
void test_mixed_operations(int iterations) {
    double acc_fp = 0.0;
    int acc_int = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create a wide basic block with independent chains */
        double fp1 = complex_chain_1(i * 1.1, i * 2.2, i * 3.3, i * 4.4);
        double fp2 = complex_chain_1(i * 5.5, i * 6.6, i * 7.7, i * 8.8);
        int int1 = integer_chain_1(i, i+1, i+2, i+3);
        int int2 = integer_chain_1(i+4, i+5, i+6, i+7);
        
        /* Interdependent operations */
        acc_fp += fp1 * fp2 - (double)int1 / (double)(int2 + 1);
        acc_int += int1 * int2 - (int)(fp1 + fp2);
        
        /* Memory operations */
        int* mem1 = (int*)__builtin_alloca(64);
        int* mem2 = (int*)__builtin_alloca(64);
        for (int j = 0; j < 16; j++) {
            mem1[j] = i + j;
            mem2[j] = i * j;
        }
        
        /* Vector operations (SIMD) */
        v4si vec1 = {i, i+1, i+2, i+3};
        v4si vec2 = {i+4, i+5, i+6, i+7};
        v4si vec3 = vec1 * vec2 + vec1;
        
        /* Use results to prevent elimination */
        acc_int += vec3[0] + vec3[1] + vec3[2] + vec3[3];
        
        /* Function call with side effect */
        acc_int += with_side_effect(i);
    }
    
    printf("Test1: fp=%.6f, int=%d\n", acc_fp, acc_int);
}

/* Test function 2: Unrolled loop creating wide basic block */
void test_unrolled_loop(int size) {
    int* data = (int*)malloc(size * sizeof(int));
    int* result = (int*)malloc(size * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        data[i] = i * 0x1234567;
    }
    
    /* Process with manual unrolling - creates wide basic block */
    int i = 0;
    for (; i + 7 < size; i += 8) {
        /* 8 independent chains - fills ready list */
        result[i]   = integer_chain_1(data[i],   data[i+1], data[i+2], data[i+3]);
        result[i+1] = integer_chain_1(data[i+1], data[i+2], data[i+3], data[i+4]);
        result[i+2] = integer_chain_1(data[i+2], data[i+3], data[i+4], data[i+5]);
        result[i+3] = integer_chain_1(data[i+3], data[i+4], data[i+5], data[i+6]);
        result[i+4] = integer_chain_1(data[i+4], data[i+5], data[i+6], data[i+7]);
        result[i+5] = integer_chain_1(data[i+5], data[i+6], data[i+7], data[i]);
        result[i+6] = integer_chain_1(data[i+6], data[i+7], data[i],   data[i+1]);
        result[i+7] = integer_chain_1(data[i+7], data[i],   data[i+1], data[i+2]);
        
        /* Mix in FP operations */
        double fp_val = complex_chain_1(data[i], data[i+1], data[i+2], data[i+3]);
        result[i] += (int)(fp_val * 1000.0);
    }
    
    /* Handle remainder */
    for (; i < size; i++) {
        result[i] = data[i] * 0x5A827999;
    }
    
    /* Compute checksum */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += result[i];
    }
    
    printf("Test2: checksum=%lld\n", checksum);
    free(data);
    free(result);
}

/* Test function 3: Complex control flow with speculative scheduling */
void test_speculative_flow(int limit) {
    volatile int condition; /* volatile to prevent optimization */
    
    double acc = 0.0;
    for (int i = 0; i < limit; i++) {
        /* Complex condition that might cause speculative scheduling */
        condition = (i * i) % 13;
        
        if (condition < 3) {
            /* Branch 1: Heavy FP chain */
            double val = i * 1.234;
            for (int j = 0; j < 4; j++) {
                val = sin(val) * cos(val) + tan(val/100.0);
            }
            acc += val;
        } 
        else if (condition < 7) {
            /* Branch 2: Integer operations */
            int val = i;
            for (int j = 0; j < 6; j++) {
                val = (val * 0x9E3779B9) ^ (val >> 13);
                val = val + i * j;
            }
            acc += (double)val;
        }
        else if (condition < 10) {
            /* Branch 3: Mixed operations */
            double val = i * 2.71828;
            int ival = i * 0x123456;
            val = val * (double)ival / (double)(i + 1);
            ival = (int)(val * 1000.0);
            acc += (double)ival;
        }
        else {
            /* Branch 4: Memory intensive */
            int* buf = (int*)__builtin_alloca(32);
            for (int j = 0; j < 8; j++) {
                buf[j] = i + j * 0x11111111;
            }
            int sum = 0;
            for (int j = 0; j < 8; j++) {
                sum += buf[j];
            }
            acc += (double)sum;
        }
        
        /* Common post-branch computation */
        acc = sqrt(fabs(acc)) + 1.0;
    }
    
    printf("Test3: acc=%.6f\n", acc);
}

/* Test function 4: Software pipelining candidate */
void test_software_pipeline(int n, double* input, double* output) {
    /* Small loop that might be software pipelined */
    for (int i = 2; i < n - 2; i++) {
        /* 5-point stencil with dependent operations */
        double center = input[i];
        double left1 = input[i-1];
        double left2 = input[i-2];
        double right1 = input[i+1];
        double right2 = input[i+2];
        
        /* Chain of FP operations */
        double avg = (left2 + left1 + center + right1 + right2) / 5.0;
        double diff1 = center - avg;
        double diff2 = diff1 * diff1;
        double smoothed = avg + 0.1 * diff1 - 0.01 * diff2;
        
        /* Additional math to increase complexity */
        smoothed = smoothed * (1.0 + sin((double)i * 0.01));
        smoothed = smoothed / (1.0 + fabs(cos((double)i * 0.02)));
        
        output[i] = smoothed;
        
        /* Function call barrier */
        if (i % 7 == 0) {
            with_side_effect(i);
        }
    }
}

/* Test function 5: Maximum scheduling pressure */
void test_max_pressure(int size) {
    /* Allocate aligned memory for vector operations */
    v4sf* vec_array = (v4sf*)aligned_alloc(16, size * sizeof(v4sf));
    float* float_array = (float*)malloc(size * 4 * sizeof(float));
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        vec_array[i] = (v4sf){i*1.0f, i*2.0f, i*3.0f, i*4.0f};
    }
    
    /* Process with maximum parallelism */
    for (int i = 0; i < size - 1; i++) {
        /* Multiple independent vector chains */
        v4sf v1 = vec_array[i];
        v4sf v2 = vec_array[i+1];
        v4sf v3 = vec_array[(i * 3) % size];
        
        /* Chain 1 */
        v4sf r1 = v1 * v2 + v3;
        v4sf r2 = r1 * (v1 - v2);
        v4sf r3 = __builtin_shuffle(r2, r1, (v4si){0, 1, 2, 3});
        
        /* Chain 2 (independent) */
        v4sf s1 = v2 * v3 - v1;
        v4sf s2 = s1 / (v3 + 1.0f);
        v4sf s3 = __builtin_shuffle(s2, s1, (v4si){3, 2, 1, 0});
        
        /* Chain 3 (mixed with scalar) */
        float f1 = v1[0] + v2[1] * v3[2];
        float f2 = f1 * f1 - v1[3];
        float f3 = sinf(f2) * cosf(f1);
        
        /* Store results */
        vec_array[i] = r3 + s3;
        float_array[i*4] = f3;
        
        /* Integer operations in parallel */
        int idx = i * 0x123456;
        idx = (idx * 0x5A827999) ^ (idx >> 13);
        idx = idx + i * 0x9E3779B9;
        float_array[i*4 + 1] = (float)idx;
    }
    
    /* Compute final checksum */
    double checksum = 0.0;
    for (int i = 0; i < size * 4; i++) {
        checksum += float_array[i];
    }
    
    printf("Test5: checksum=%.6f\n", checksum);
    
    free(vec_array);
    free(float_array);
}

/* Main driver */
int main(int argc, char** argv) {
    int iterations = 1000;
    int size = 1024;
    
    printf("Starting scheduling coverage tests...\n");
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Test 1: Mixed operations */
    clock_t start = clock();
    test_mixed_operations(iterations);
    printf("  Time: %.3f ms\n", (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 2: Unrolled loops */
    start = clock();
    test_unrolled_loop(size);
    printf("  Time: %.3f ms\n", (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 3: Speculative flow */
    start = clock();
    test_speculative_flow(iterations);
    printf("  Time: %.3f ms\n", (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 4: Software pipelining */
    double* input = (double*)malloc(size * sizeof(double));
    double* output = (double*)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        input[i] = sin(i * 0.01);
    }
    start = clock();
    test_software_pipeline(size, input, output);
    printf("  Time: %.3f ms\n", (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 5: Maximum pressure */
    start = clock();
    test_max_pressure(size / 4);
    printf("  Time: %.3f ms\n", (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Final checksum to prevent dead code elimination */
    double final_check = 0.0;
    for (int i = 0; i < size; i++) {
        final_check += output[i];
    }
    printf("Final check: %.6f\n", final_check);
    
    free(input);
    free(output);
    
    printf("All tests completed.\n");
    return 0;
}
