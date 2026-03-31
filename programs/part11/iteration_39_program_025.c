/* test_sched_context.c
 * Comprehensive test to trigger scheduling context cleanup in haifa-sched.cc
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -mtune=generic -march=x86-64 -fno-omit-frame-pointer -funroll-loops -fno-vect-cost-model -o test_sched test_sched_context.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 256
#define ITERS 1000

/* Inline functions to increase instruction count */
static inline float compute_poly(float x, float a, float b, float c, float d) {
    return a*x*x*x + b*x*x + c*x + d;
}

static inline int int_chain(int a, int b, int c, int d) {
    int t1 = a * b + c;
    int t2 = t1 ^ d;
    int t3 = t2 << 3;
    int t4 = t3 - a;
    int t5 = t4 * b;
    return t5;
}

/* Function with complex dependency chains and memory aliasing */
void test_complex_block(float* restrict arr1, float* restrict arr2, 
                        float* restrict arr3, int n) {
    /* Wide basic block with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Multiple dependent FP operations */
        float a = arr1[i] * 2.5f;
        float b = sinf(a) + cosf(a);
        float c = b * b - a;
        float d = sqrtf(fabsf(c)) + 1.0f;
        
        /* Integer operations interleaved */
        int idx = (int)(d * 100) % n;
        arr2[idx] = arr2[idx] + d;
        
        /* More FP chain */
        float e = arr3[i] * 0.7f;
        float f = e * e * e;
        float g = f + a - b;
        
        /* Conditional update creating scheduling pressure */
        if (g > 0.0f) {
            arr1[i] = g * 0.5f;
        } else {
            arr1[i] = -g * 0.3f;
        }
        
        /* Additional independent chains */
        float h = arr2[i] * arr3[i];
        arr3[i] = h + arr1[i];
    }
}

/* Function with unrolled loops creating wide basic blocks */
void test_unrolled_operations(int* restrict out, int* restrict in1, 
                              int* restrict in2, int n) {
    /* Manual unrolling to create large basic block */
    for (int i = 0; i < n; i += 4) {
        /* Independent computation paths */
        int t1 = in1[i] * in2[i];
        int t2 = in1[i+1] + in2[i+1];
        int t3 = in1[i+2] - in2[i+2];
        int t4 = in1[i+3] ^ in2[i+3];
        
        /* Dependency chains */
        t1 = t1 * 3 - t2;
        t2 = t2 + t3 * 2;
        t3 = t3 ^ t4;
        t4 = t4 * 7 + t1;
        
        /* More operations to fill ready list */
        out[i] = t1 + t2;
        out[i+1] = t2 - t3;
        out[i+2] = t3 * t4;
        out[i+3] = t4 / (t1 + 1);
        
        /* Additional parallel chains */
        int a = in1[i] << 2;
        int b = in2[i] >> 1;
        int c = a | b;
        out[i] ^= c;
    }
}

/* Function with vector-like operations using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

void test_vector_ops(float* restrict a, float* restrict b, 
                     float* restrict c, int n) {
    /* Process 4 elements at a time */
    for (int i = 0; i < n; i += 4) {
        /* Load vectors */
        v4sf va = {a[i], a[i+1], a[i+2], a[i+3]};
        v4sf vb = {b[i], b[i+1], b[i+2], b[i+3]};
        
        /* Mixed vector operations creating scheduling pressure */
        v4sf vc = va * vb + va;
        v4sf vd = vc * vc - vb;
        v4sf ve = __builtin_ia32_sqrtps(vd);  /* SSE intrinsic */
        
        /* Store results */
        c[i] = ve[0];
        c[i+1] = ve[1];
        c[i+2] = ve[2];
        c[i+3] = ve[3];
        
        /* Additional scalar operations to mix with vector ops */
        float t = a[i] * b[i];
        c[i] += t * 0.5f;
    }
}

/* Function with speculative scheduling opportunities */
void test_speculative(int* restrict data, int n) {
    /* Inner loop with small iteration count */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Multiple conditional updates */
        if (val > 1000) {
            val = val * 2 - 50;
        } else if (val > 500) {
            val = val + 100;
        } else {
            val = val * 3;
        }
        
        /* Complex arithmetic chain */
        for (int j = 0; j < 6; j++) {  /* Small loop for software pipelining */
            val = (val * 1103515245 + 12345) & 0x7fffffff;
            val = val ^ (val >> 16);
        }
        
        /* Conditional store */
        if (val % 2 == 0) {
            data[i] = val / 2;
        } else {
            data[i] = val * 3 + 1;
        }
    }
}

/* Function with function calls and side effects */
volatile int global_counter = 0;

static inline void update_counter(int delta) {
    /* Inline assembly to prevent optimization and create scheduling barrier */
    asm volatile("" : "+r"(delta) : : "memory");
    global_counter += delta;
}

void test_with_barriers(float* arr, int n) {
    for (int i = 0; i < n; i++) {
        /* Computation before barrier */
        float x = arr[i] * 2.0f;
        float y = x * x - x + 1.0f;
        
        /* Function call acting as barrier */
        update_counter((int)y);
        
        /* Computation after barrier */
        float z = y * 0.5f;
        arr[i] = z + (float)i;
        
        /* More mixed operations */
        int t = (int)z;
        t = t * 7 - 3;
        arr[i] += (float)(t % 100);
    }
}

/* Main driver with multiple test functions */
int main() {
    /* Allocate test arrays */
    float* arr1 = malloc(SIZE * sizeof(float));
    float* arr2 = malloc(SIZE * sizeof(float));
    float* arr3 = malloc(SIZE * sizeof(float));
    int* int_arr1 = malloc(SIZE * sizeof(int));
    int* int_arr2 = malloc(SIZE * sizeof(int));
    int* int_arr3 = malloc(SIZE * sizeof(int));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (float)rand() / RAND_MAX * 100.0f;
        arr2[i] = (float)rand() / RAND_MAX * 100.0f;
        arr3[i] = (float)rand() / RAND_MAX * 100.0f;
        int_arr1[i] = rand() % 1000;
        int_arr2[i] = rand() % 1000;
        int_arr3[i] = 0;
    }
    
    printf("Starting scheduling stress test...\n");
    
    /* Run multiple iterations to ensure scheduling happens */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Test 1: Complex block with mixed operations */
        test_complex_block(arr1, arr2, arr3, SIZE);
        
        /* Test 2: Unrolled integer operations */
        test_unrolled_operations(int_arr3, int_arr1, int_arr2, SIZE);
        
        /* Test 3: Vector operations (if supported) */
        test_vector_ops(arr1, arr2, arr3, SIZE);
        
        /* Test 4: Speculative scheduling */
        test_speculative(int_arr1, SIZE);
        
        /* Test 5: Barriers and function calls */
        test_with_barriers(arr1, SIZE);
        
        /* Mix data between tests to prevent optimization */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = arr1[i] * 0.99f + arr2[i] * 0.01f;
            int_arr1[i] = (int_arr1[i] + int_arr2[i]) & 0xFFF;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float sum1 = 0.0f, sum2 = 0.0f;
    int sum3 = 0;
    for (int i = 0; i < SIZE; i++) {
        sum1 += arr1[i];
        sum2 += arr2[i] + arr3[i];
        sum3 += int_arr1[i] + int_arr2[i] + int_arr3[i];
    }
    
    printf("Checksums: float1=%.2f, float2=%.2f, int=%d\n", 
           sum1, sum2, sum3);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    
    return 0;
}
