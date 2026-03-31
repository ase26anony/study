/* test_scheduler_coverage.c
 * 
 * This program creates complex basic blocks that force GCC's instruction scheduler
 * to allocate and use the full scheduling context, ensuring the cleanup code
 * in free_sched_block() is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ========== TEST FUNCTION 1: Complex Arithmetic Dependency Chains ========== */
/* This function creates long dependency chains with mixed operations */
static ALWAYS_INLINE float complex_chain(float a, float b, float c, float d, float e) {
    /* Long dependency chain with mixed operations */
    float t1 = a * b + c;
    float t2 = t1 / d - e;
    float t3 = t2 * t1 + a;
    float t4 = t3 - b * c;
    float t5 = t4 / d + e * t3;
    float t6 = sqrtf(fabsf(t5)) + t4;
    float t7 = t6 * t5 - t3;
    float t8 = t7 / (t2 + 1.0f) + t6;
    float t9 = sinf(t8) * cosf(t7) + t5;
    float t10 = t9 * t8 - t7 / t6;
    
    /* Integer operations interleaved */
    int i1 = (int)t10;
    int i2 = i1 * 37 + 19;
    int i3 = i2 / 7 - i1;
    int i4 = i3 ^ (i2 << 3);
    int i5 = i4 | (i3 & 0xFF);
    
    return t10 + (float)i5;
}

/* ========== TEST FUNCTION 2: Loop with Software Pipelining Potential ========== */
/* This creates a loop that might trigger software pipelining and state saving */
static ALWAYS_INLINE double loop_pipeline(int iterations, double* data) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Dependent operations within loop */
        double x = data[i];
        double y = x * x + 1.0;
        double z = y / (x + 2.0);
        sum += z;
        prod *= (x + z);
        
        /* Conditional that might create speculative scheduling */
        if (x > 0.5) {
            sum += sin(x) * cos(z);
        } else {
            sum -= tan(x) / (z + 1.0);
        }
        
        /* More arithmetic */
        data[i] = (sum + prod) / (i + 1);
    }
    
    return sum + prod;
}

/* ========== TEST FUNCTION 3: Wide Basic Block with Vector Operations ========== */
/* This creates a wide basic block that fills instruction queues */
static ALWAYS_INLINE void wide_block_operations(float* restrict a, float* restrict b, 
                                                float* restrict c, int n) {
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < n; i += 4) {
        /* Independent chains that can be reordered */
        float a0 = a[i] * b[i] + c[i];
        float a1 = a[i+1] / b[i+1] - c[i+1];
        float a2 = a[i+2] + b[i+2] * c[i+2];
        float a3 = a[i+3] - b[i+3] / c[i+3];
        
        /* Cross dependencies */
        float b0 = a0 * a1 + a2;
        float b1 = a1 - a2 * a3;
        float b2 = a2 + a3 / a0;
        float b3 = a3 * a0 - a1;
        
        /* Memory operations with potential aliasing */
        c[i] = b0 + b1;
        c[i+1] = b1 - b2;
        c[i+2] = b2 * b3;
        c[i+3] = b3 / (b0 + 1.0f);
        
        /* More arithmetic to increase instruction count */
        a[i] = sqrtf(fabsf(b0)) + sinf(b1);
        a[i+1] = cosf(b2) * expf(b3);
        a[i+2] = logf(fabsf(b0) + 1.0f) - tanf(b1);
        a[i+3] = atanf(b2) + asinf(fminf(0.99f, b3));
    }
}

/* ========== TEST FUNCTION 4: Mixed Integer/FP with Function Calls ========== */
/* This uses inline assembly and function calls as scheduling barriers */
static ALWAYS_INLINE double mixed_operations_with_barriers(int x, float y, double z) {
    /* Integer operations */
    int i1 = x * 17 + 23;
    int i2 = i1 / 3 - x;
    int i3 = i2 << 4;
    int i4 = i3 ^ 0xABCD;
    int i5 = (i4 & 0xFF00) | (x & 0x00FF);
    
    /* Floating point operations */
    double d1 = (double)y * z + 1.234;
    double d2 = d1 / (double)x - 5.678;
    double d3 = d2 * z + d1;
    
    /* Inline assembly as scheduling barrier */
    asm volatile("" : "+r"(i5), "+r"(d3) : : "memory");
    
    /* Function call - acts as scheduling barrier */
    double d4 = pow(d3, 2.0) + sin(d2);
    
    /* More mixed operations */
    float f1 = (float)d4 * y;
    int i6 = (int)f1 + i5;
    double d5 = (double)i6 / d3;
    
    /* Another inline assembly */
    asm volatile("" : "+r"(i6), "+r"(d5) : : "memory");
    
    return d5 + d4 + (double)i6;
}

/* ========== TEST FUNCTION 5: SIMD Vector Operations ========== */
/* Uses GCC vector extensions to create parallel operations */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4sf c) {
    v4sf r1 = a * b + c;
    v4sf r2 = r1 / a - b;
    v4sf r3 = r2 * c + r1;
    v4sf r4 = r3 - a / b;
    v4sf r5 = r4 * r2 + r3 / r1;
    
    /* Conditional vector operations */
    v4sf mask = a > b;
    v4sf r6 = r5 * (mask ? c : b);
    
    return r6 + r4 + r3 + r2 + r1;
}

/* ========== TEST FUNCTION 6: Complex Control Flow with Switch ========== */
/* This creates basic blocks ending with switches for state tracking */
static ALWAYS_INLINE double control_flow_with_switch(int mode, double x, double y) {
    double result = 0.0;
    
    /* Complex arithmetic before switch */
    double a = x * x + y * y;
    double b = sin(x) * cos(y);
    double c = exp(-fabs(x - y));
    double d = a * b + c;
    
    /* Switch with multiple cases - requires state tracking */
    switch (mode % 5) {
        case 0:
            result = d + a;
            /* More operations in case */
            result = result * 2.0 - b;
            break;
        case 1:
            result = d - b;
            result = result / (c + 1.0);
            result = sqrt(fabs(result));
            break;
        case 2:
            result = a * c - d;
            result = log(fabs(result) + 1.0);
            break;
        case 3:
            result = b / c + d;
            result = sin(result) * cos(a);
            break;
        case 4:
            result = (a + b + c + d) / 4.0;
            result = tan(result) + atan(result);
            break;
    }
    
    /* More arithmetic after switch */
    result = result * x + y / (result + 1.0);
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */
int main() {
    const int N = 256;
    const int ITERATIONS = 1000;
    
    /* Allocate aligned memory for vector operations */
    float* array1 = (float*)aligned_alloc(16, N * sizeof(float));
    float* array2 = (float*)aligned_alloc(16, N * sizeof(float));
    float* array3 = (float*)aligned_alloc(16, N * sizeof(float));
    double* darray = (double*)malloc(N * sizeof(double));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < N; i++) {
        array1[i] = (float)rand() / RAND_MAX * 10.0f;
        array2[i] = (float)rand() / RAND_MAX * 5.0f;
        array3[i] = (float)rand() / RAND_MAX * 2.0f;
        darray[i] = (double)rand() / RAND_MAX * 3.0;
    }
    
    double total_result = 0.0;
    clock_t start = clock();
    
    /* Execute all test functions multiple times to ensure scheduling occurs */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex chains */
        for (int i = 0; i < 16; i++) {
            float val = complex_chain(array1[i], array2[i], array3[i], 
                                     (float)(i+1), global_float);
            total_result += (double)val;
        }
        
        /* Test 2: Loop pipelining */
        total_result += loop_pipeline(8, darray);
        
        /* Test 3: Wide block operations */
        wide_block_operations(array1, array2, array3, 64);
        
        /* Test 4: Mixed operations with barriers */
        for (int i = 0; i < 32; i++) {
            double val = mixed_operations_with_barriers(
                i, array1[i], darray[i % 8]);
            total_result += val;
        }
        
        /* Test 5: Vector operations */
        for (int i = 0; i < 16; i += 4) {
            v4sf a = {array1[i], array1[i+1], array1[i+2], array1[i+3]};
            v4sf b = {array2[i], array2[i+1], array2[i+2], array2[i+3]};
            v4sf c = {array3[i], array3[i+1], array3[i+2], array3[i+3]};
            v4sf r = vector_operations(a, b, c);
            
            /* Extract results */
            float results[4];
            memcpy(results, &r, sizeof(results));
            total_result += results[0] + results[1] + results[2] + results[3];
        }
        
        /* Test 6: Control flow with switch */
        for (int i = 0; i < 32; i++) {
            double val = control_flow_with_switch(
                i, darray[i % 8], darray[(i+1) % 8]);
            total_result += val;
        }
        
        /* Modify data to prevent dead code elimination */
        array1[iter % N] = (float)total_result * 0.001f;
        darray[iter % 8] = total_result * 0.0001;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Print results to prevent optimization */
    printf("Total result: %.12f\n", total_result);
    printf("Elapsed time: %.6f seconds\n", elapsed);
    printf("Array checksum: %.6f\n", 
           array1[N/2] + array2[N/2] + array3[N/2] + darray[N/8]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(darray);
    
    return (total_result > 0) ? 0 : 1;
}
