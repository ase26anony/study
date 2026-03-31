/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup logic in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require extensive instruction scheduling with:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Mixed instruction types and dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ======================================================================
 * Function 1: Complex arithmetic with mixed types and dependencies
 * Creates long dependency chains and parallel independent paths
 * ====================================================================== */
ALWAYS_INLINE
static float complex_arithmetic_chain(int iterations) {
    float a = global_float;
    double b = global_double;
    int c = global_seed;
    float result = 0.0f;
    
    /* Multiple dependent chains */
    for (int i = 0; i < iterations; i++) {
        /* Chain 1: FP operations with dependencies */
        a = a * 1.1f + sinf(a) * 0.5f;
        b = b * 1.05 + cos(b) * 0.3;
        
        /* Chain 2: Integer operations with dependencies */
        c = (c * 1103515245 + 12345) & 0x7fffffff;
        int d = c % 100;
        d = d * d + d;
        
        /* Chain 3: Mixed operations */
        float e = a * d;
        double f = b + d;
        
        /* Chain 4: Memory operations with potential aliasing */
        float* ptr1 = &a;
        float* ptr2 = &result;
        *ptr1 = *ptr1 * 0.9f;
        *ptr2 += *ptr1 + f;
        
        /* Independent parallel computations */
        float g = sqrtf(fabsf(e));
        double h = log(fabs(f) + 1.0);
        int j = (c >> 16) & 0xFF;
        
        /* More dependencies */
        result = result + g + h + j;
    }
    
    return result;
}

/* ======================================================================
 * Function 2: Vector operations using GCC vector extensions
 * Creates many parallel operations that fill instruction queues
 * ====================================================================== */
ALWAYS_INLINE
static v4sf vector_operations(v4sf a, v4sf b, v4sf c) {
    /* Multiple vector operations creating independent chains */
    v4sf r1 = a + b * c;
    v4sf r2 = a - b / (c + 1.0f);
    v4sf r3 = sqrtf(fabsf(r1));
    v4sf r4 = r2 * r3 + a;
    v4sf r5 = r4 - b * 0.5f;
    v4sf r6 = r5 + c * 2.0f;
    v4sf r7 = r6 * r1 - r2;
    v4sf r8 = r7 / (r3 + 2.0f);
    
    /* Interleave with scalar operations */
    float s1 = r1[0] + r2[1];
    float s2 = r3[2] * r4[3];
    float s3 = s1 * s2 - global_float;
    
    /* More vector operations */
    v4sf r9 = r8 + s3;
    v4sf r10 = r9 * a - b;
    v4sf r11 = r10 / (c + 0.001f);
    
    return r11;
}

/* ======================================================================
 * Function 3: Memory-intensive operations with potential aliasing
 * Forces the scheduler to handle memory dependencies
 * ====================================================================== */
ALWAYS_INLINE
static double memory_intensive_processing(double* array, int size, int iterations) {
    double sum = 0.0;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < iterations; i++) {
        int idx1 = i % size;
        int idx2 = (i * 7) % size;
        int idx3 = (i * 13) % size;
        int idx4 = (i * 23) % size;
        
        /* Multiple memory operations with complex addressing */
        double val1 = array[idx1] * global_double;
        double val2 = array[idx2] + sin(array[idx3]);
        double val3 = array[idx4] * cos(val1);
        
        /* Potential aliasing through pointers */
        double* ptr1 = &array[idx1];
        double* ptr2 = &array[idx2];
        *ptr1 = val1 * 0.9;
        *ptr2 = val2 * 1.1;
        
        /* More computations */
        sum += val1 + val2 + val3;
        
        /* Additional independent operations */
        double temp1 = sqrt(fabs(val1));
        double temp2 = log(fabs(val2) + 1.0);
        double temp3 = exp(val3 * 0.01);
        
        sum += temp1 + temp2 + temp3;
    }
    
    return sum;
}

/* ======================================================================
 * Function 4: Control flow with speculative scheduling opportunities
 * Creates basic blocks ending with conditional branches
 * ====================================================================== */
ALWAYS_INLINE
static int control_flow_intensive(int limit) {
    int result = 0;
    
    /* Loop with small iteration count for potential software pipelining */
    for (int i = 0; i < 8; i++) {
        /* Complex condition with side effects */
        int cond = (global_seed * i) & 0xF;
        
        /* Multiple dependent operations before branch */
        float f1 = global_float * i;
        double d1 = global_double / (i + 1);
        int i1 = global_seed + i * 3;
        
        /* Branch creates speculative scheduling opportunity */
        if (cond > 7) {
            /* Branch target 1 */
            f1 = f1 * 2.0f + sinf(f1);
            d1 = d1 * 1.5 + cos(d1);
            i1 = i1 * 2 - 1;
            result += (int)(f1 + d1) + i1;
        } else if (cond > 3) {
            /* Branch target 2 */
            f1 = f1 / 2.0f - cosf(f1);
            d1 = d1 / 1.5 - sin(d1);
            i1 = i1 / 2 + 1;
            result += (int)(f1 - d1) * i1;
        } else {
            /* Branch target 3 */
            f1 = sqrtf(fabsf(f1));
            d1 = log(fabs(d1) + 1.0);
            i1 = (i1 * 3) % 100;
            result += (int)(f1 * d1) | i1;
        }
        
        /* More operations after branch */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* ======================================================================
 * Function 5: Mixed operations with inline assembly barriers
 * Forces scheduler to handle non-reorderable operations
 * ====================================================================== */
ALWAYS_INLINE
static double mixed_with_assembly(int iterations) {
    double sum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Floating point operations */
        double d1 = global_double * i;
        float f1 = global_float + i;
        
        /* Integer operations */
        int i1 = global_seed * i;
        int i2 = i1 ^ 0x55555555;
        
        /* Memory barrier via inline assembly */
        asm volatile("" ::: "memory");
        
        /* More FP operations */
        d1 = d1 + sin(d1 * 0.01);
        f1 = f1 * cosf(f1 * 0.02f);
        
        /* More integer operations */
        i1 = (i1 * 3 + 7) % 100;
        i2 = (i2 >> 3) | (i2 << 29);
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Mixed type computations */
        sum += d1 + f1 + i1;
        fsum += f1 * 0.5f + i2 * 0.01f;
        isum += i1 + (int)f1;
        
        /* Complex dependency chain */
        global_double = global_double * 0.99 + d1 * 0.01;
        global_float = global_float * 0.95f + f1 * 0.05f;
        global_seed = (global_seed + i1) & 0x7fffffff;
    }
    
    return sum + fsum + isum;
}

/* ======================================================================
 * Function 6: Very wide basic block through manual unrolling
 * Creates maximum instruction queue pressure
 * ====================================================================== */
ALWAYS_INLINE
static double wide_basic_block(int unroll_factor) {
    double result = 0.0;
    
    /* Manually unrolled computations */
    double a1 = 1.0, a2 = 2.0, a3 = 3.0, a4 = 4.0;
    double b1 = 5.0, b2 = 6.0, b3 = 7.0, b4 = 8.0;
    double c1 = 9.0, c2 = 10.0, c3 = 11.0, c4 = 12.0;
    
    for (int i = 0; i < unroll_factor; i++) {
        /* 32 independent floating point operations */
        a1 = a1 * 1.01 + sin(a1 * 0.1);
        a2 = a2 * 1.02 + cos(a2 * 0.2);
        a3 = a3 * 1.03 + sin(a3 * 0.3);
        a4 = a4 * 1.04 + cos(a4 * 0.4);
        
        b1 = b1 * 0.99 - sin(b1 * 0.5);
        b2 = b2 * 0.98 - cos(b2 * 0.6);
        b3 = b3 * 0.97 - sin(b3 * 0.7);
        b4 = b4 * 0.96 - cos(b4 * 0.8);
        
        c1 = c1 + a1 * b1;
        c2 = c2 + a2 * b2;
        c3 = c3 + a3 * b3;
        c4 = c4 + a4 * b4;
        
        /* More operations mixing results */
        double d1 = sqrt(fabs(c1 - c2));
        double d2 = log(fabs(c3 - c4) + 1.0);
        double d3 = exp((c1 + c3) * 0.01);
        double d4 = pow(fabs(c2 + c4), 0.5);
        
        /* Even more operations */
        a1 = a1 + d1 * 0.1;
        a2 = a2 + d2 * 0.2;
        a3 = a3 + d3 * 0.3;
        a4 = a4 + d4 * 0.4;
        
        b1 = b1 - d1 * 0.5;
        b2 = b2 - d2 * 0.6;
        b3 = b3 - d3 * 0.7;
        b4 = b4 - d4 * 0.8;
        
        /* Final accumulation */
        result += a1 + a2 + a3 + a4 + b1 + b2 + b3 + b4 + c1 + c2 + c3 + c4;
    }
    
    return result;
}

/* ======================================================================
 * Main driver function
 * Calls all test functions with different parameters to ensure
 * various scheduling scenarios are triggered
 * ====================================================================== */
int main(int argc, char** argv) {
    clock_t start = clock();
    double total_result = 0.0;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex arithmetic chains */
    printf("Test 1: Complex arithmetic chains...\n");
    for (int i = 0; i < 10; i++) {
        float r1 = complex_arithmetic_chain(5);
        total_result += r1;
    }
    
    /* Test 2: Vector operations */
    printf("Test 2: Vector operations...\n");
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_c = {0.1f, 0.2f, 0.3f, 0.4f};
    
    for (int i = 0; i < 20; i++) {
        v4sf r2 = vector_operations(vec_a, vec_b, vec_c);
        total_result += r2[0] + r2[1] + r2[2] + r2[3];
        vec_a = vec_b;
        vec_b = vec_c;
        vec_c = r2;
    }
    
    /* Test 3: Memory intensive processing */
    printf("Test 3: Memory intensive processing...\n");
    double* array = (double*)malloc(1000 * sizeof(double));
    for (int i = 0; i < 1000; i++) {
        array[i] = sin(i * 0.01);
    }
    
    for (int i = 0; i < 5; i++) {
        double r3 = memory_intensive_processing(array, 1000, 50);
        total_result += r3;
    }
    
    /* Test 4: Control flow intensive */
    printf("Test 4: Control flow intensive...\n");
    for (int i = 0; i < 100; i++) {
        int r4 = control_flow_intensive(100);
        total_result += r4;
    }
    
    /* Test 5: Mixed with assembly */
    printf("Test 5: Mixed operations with assembly...\n");
    for (int i = 0; i < 10; i++) {
        double r5 = mixed_with_assembly(20);
        total_result += r5;
    }
    
    /* Test 6: Wide basic block */
    printf("Test 6: Wide basic block...\n");
    for (int i = 0; i < 5; i++) {
        double r6 = wide_basic_block(3);
        total_result += r6;
    }
    
    free(array);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nTotal result: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Scheduler coverage test complete.\n");
    
    /* Use result to prevent dead code elimination */
    if (total_result > 0.0) {
        return 0;
    } else {
        return 1;
    }
}
