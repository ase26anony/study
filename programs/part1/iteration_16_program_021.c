#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3", "unroll-loops")))
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed integer and floating point operations */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: reusing temp */
        temp = temp + (float)i * 0.1f;
        
        /* WAW hazard: multiple writes to b[i] */
        b[i] = temp;
        b[i] = b[i] + sinf(temp * 0.01f);
        
        /* Complex dependency chain */
        sum += b[i] * cosf((float)i * 0.001f);
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_function(int* restrict arr, int n) {
    int result = 0;
    
    /* Pointer chasing with varying latencies */
    int* ptr = arr;
    for (int i = 0; i < n; i++) {
        /* Load/store sequence with hazards */
        int val = *ptr;
        
        /* Inline assembly with register clobbering */
        asm volatile(
            "addl $1, %0\n\t"
            : "+r" (val)
            :
            : "cc"
        );
        
        *ptr = val;
        
        /* Conditional move vs branching */
        ptr = (val % 2) ? (ptr + 1) : (ptr + 2);
        
        /* Complex control flow within loop */
        switch (val % 5) {
            case 0: result += val * 2; break;
            case 1: result += val / 2; break;
            case 2: result += val << 1; break;
            case 3: result += val >> 1; break;
            default: result += val ^ 0xFF; break;
        }
        
        /* Early exit condition */
        if (ptr >= arr + n) break;
    }
    return result;
}

__attribute__((optimize("O3"), noinline))
static double vectorized_loop(double* restrict a, double* restrict b, 
                              double* restrict c, int n) {
    double sum = 0.0;
    
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* SIMD-friendly operations */
        double t1 = a[i] * b[i];
        double t2 = sin(t1);
        double t3 = cos(b[i]);
        
        /* Multiple dependencies */
        c[i] = t1 + t2 * t3;
        
        /* Mixed precision operations */
        sum += (float)c[i] * (double)(i % 8);
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((optimize("O3")))
static void nested_control_flow(int* data, int n) {
    /* Complex control flow patterns */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Nested if-else with computed operations */
        if (val > 100) {
            if (val % 3 == 0) {
                data[i] = val * 2;
            } else if (val % 3 == 1) {
                /* Inline assembly with specific constraints */
                asm volatile(
                    "imull %%ecx, %%eax\n\t"
                    : "+a" (val)
                    : "c" (val)
                    : "cc"
                );
                data[i] = val;
            } else {
                data[i] = val >> 2;
            }
        } else {
            /* Switch with sparse case values */
            switch (val % 7) {
                case 0: data[i] = val + 1; break;
                case 1: data[i] = val - 1; break;
                case 3: data[i] = val * 3; break;  /* Note: case 2 is missing */
                case 6: data[i] = val / 2; break;
                default: data[i] = val ^ 0x55; break;
            }
        }
        
        /* Continue condition */
        if (data[i] < 0) continue;
        
        /* Multiple exit points */
        if (data[i] > 1000) {
            data[i] = 1000;
            if (i > n/2) return;
        }
    }
}

__attribute__((noinline, optimize("O3", "tree-vectorize")))
static float mixed_hazards(float* a, float* b, float* c, int n) {
    float acc = 0.0f;
    
    /* Loop with multiple interleaved dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* RAW: c depends on a and b */
        float t1 = a[i-1] + b[i];
        float t2 = a[i] * b[i+1];
        
        /* WAR: reusing t1 */
        t1 = t1 * t2;
        
        /* WAW: multiple writes to c[i] */
        c[i] = t1;
        c[i] = c[i] + a[i+1];
        
        /* Complex expression with mixed operations */
        acc += c[i] * (float)(i & 0xF) - sqrtf(fabsf(t1));
        
        /* Scheduling barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return acc;
}

int main(void) {
    /* Initialize data */
    float* fa = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fb = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fc = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double* da = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* db = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* dc = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    int* idata = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        da[i] = (double)rand() / RAND_MAX * 100.0;
        db[i] = (double)rand() / RAND_MAX * 100.0;
        idata[i] = rand() % 1000;
    }
    
    float total_sum = 0.0f;
    double vector_sum = 0.0;
    int int_result = 0;
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function (should trigger aggressive scheduling) */
        total_sum += hot_function(fa, fb, ARRAY_SIZE);
        
        /* Call cold function (different scheduling characteristics) */
        int_result += cold_function(idata, ARRAY_SIZE);
        
        /* Vectorized operations */
        vector_sum += vectorized_loop(da, db, dc, ARRAY_SIZE);
        
        /* Complex control flow */
        nested_control_flow(idata, ARRAY_SIZE);
        
        /* Mixed hazard patterns */
        total_sum += mixed_hazards(fa, fb, fc, ARRAY_SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            fa[i] += 0.01f;
            da[i] += 0.01;
            idata[i] = (idata[i] + 1) % 1000;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: hot_sum=%.6f, cold_result=%d, vector_sum=%.6f\n",
           total_sum, int_result, vector_sum);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(da);
    free(db);
    free(dc);
    free(idata);
    
    return 0;
}
