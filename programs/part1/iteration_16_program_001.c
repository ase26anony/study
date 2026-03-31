#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
__attribute__((optimize("sched-pressure")))
static float hot_loop_scheduler(float *data, int n) {
    volatile float sum = 0.0f;
    
    /* Mixed data dependencies and pointer chasing */
    float *ptr = data;
    float *end = data + n;
    
    /* Complex scheduling region with RAW/WAR/WAW hazards */
    while (ptr < end) {
        float temp1 = *ptr;
        float temp2 = temp1 * temp1;  // WAW hazard potential
        temp1 = temp2 + 1.0f;         // WAR hazard
        *ptr = temp1;                 // RAW hazard
        sum += temp1;
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Pointer chasing with offset */
        ptr += (int)(temp1) % 8 + 1;
    }
    
    return sum;
}

__attribute__((cold, noinline))
static void cold_path_scheduler(int *arr, float *farr, int n) {
    /* Complex control flow challenging scheduler */
    for (int i = 0; i < n; i++) {
        /* Mixed integer/float operations */
        int idx = i * 2;
        float fval = (float)arr[i];
        
        /* Conditional moves and branching mix */
        fval = (idx % 3 == 0) ? fval * 2.0f : fval / 2.0f;
        
        /* Switch statement with sparse cases */
        switch (idx % 7) {
            case 0: farr[i] = fval + 1.0f; break;
            case 1: farr[i] = fval - 1.0f; break;
            case 3: farr[i] = fval * 3.0f; break;  /* Note: case 2 skipped */
            case 6: farr[i] = fval / 3.0f; break;
            default: farr[i] = fval; break;
        }
        
        /* Another scheduling barrier */
        asm volatile("" ::: "r8", "r9", "memory");
    }
}

__attribute__((optimize("O3")))
#pragma GCC unroll 4
static void vectorized_unrolled_scheduler(float *a, float *b, float *c, int n) {
    /* SIMD-friendly loop with unrolling directive */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Mixed operations creating scheduling pressure */
        float t1 = a[i] * b[i];
        float t2 = t1 + (float)i;
        float t3 = sinf(t2);  /* FP operation */
        c[i] = t3 * t3;
        
        /* Memory barrier every 4 iterations */
        if ((i & 3) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

__attribute__((noinline))
static float nested_loop_scheduler(float *data, int n) {
    float acc = 0.0f;
    
    /* Nested loops with multiple exit points */
    for (int i = 0; i < n; i++) {
        if (data[i] < 0) continue;  /* Early continue */
        
        float val = data[i];
        for (int j = 0; j < 8; j++) {
            val = val * 1.1f + (float)j;
            
            /* Early exit from inner loop */
            if (val > 1000.0f) break;
            
            /* Mixed integer/float with dependency chain */
            int int_val = (int)val;
            val = val + (float)(int_val % 5);
        }
        
        /* Compute goto-like pattern using switch */
        switch (i % 4) {
            case 0: acc += val; break;
            case 1: acc -= val; break;
            case 2: acc *= 1.01f; break;
            case 3: acc = fabsf(acc); break;
        }
        
        /* Scheduling barrier with register clobber */
        asm volatile("" ::: "r10", "r11", "r12", "memory");
    }
    
    return acc;
}

__attribute__((optimize("O3")))
static void mixed_workload_scheduler(void) {
    /* Allocate aligned memory for better vectorization */
    float *fdata = (float*)aligned_alloc(32, SIZE * sizeof(float));
    int *idata = (int*)aligned_alloc(32, SIZE * sizeof(int));
    float *fdata2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float *fdata3 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    
    if (!fdata || !idata || !fdata2 || !fdata3) {
        free(fdata); free(idata); free(fdata2); free(fdata3);
        return;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        fdata[i] = (float)(i % 100) * 0.1f;
        idata[i] = i * 2;
        fdata2[i] = (float)(i % 50) * 0.2f;
    }
    
    float total = 0.0f;
    
    /* Execute multiple scheduling-intensive functions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += hot_loop_scheduler(fdata, SIZE);
        
        cold_path_scheduler(idata, fdata2, SIZE);
        
        vectorized_unrolled_scheduler(fdata, fdata2, fdata3, SIZE);
        
        total += nested_loop_scheduler(fdata3, SIZE);
        
        /* Modify data to change dependencies */
        for (int i = 0; i < SIZE; i++) {
            fdata[i] += 0.001f * total;
        }
        
        /* Strong memory barrier */
        asm volatile("mfence" ::: "memory");
    }
    
    printf("Accumulated result: %f\n", total);
    
    free(fdata);
    free(idata);
    free(fdata2);
    free(fdata3);
}

/* Main driver with profile guidance */
int main(void) {
    clock_t start = clock();
    
    printf("Starting selective scheduling stress test...\n");
    
    /* First compilation: profile generation */
    mixed_workload_scheduler();
    
    /* Additional test variations */
    for (int test = 0; test < 3; test++) {
        float *temp = (float*)aligned_alloc(32, 512 * sizeof(float));
        if (temp) {
            for (int i = 0; i < 512; i++) temp[i] = (float)(i + test);
            float result = hot_loop_scheduler(temp, 512);
            printf("Test %d: %f\n", test, result);
            free(temp);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", elapsed);
    
    return 0;
}
