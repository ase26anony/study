/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < size; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = a[i] + b[i];
        float t3 = a[i] - b[i];
        float t4 = a[i] / (b[i] + 0.1f);  /* FP divide has latency */
        
        /* Group 2: More independent operations */
        float t5 = t1 * t2;
        float t6 = t3 * t4;
        float t7 = t5 + t6;
        float t8 = t5 - t6;
        
        /* Group 3: Integer operations mixed in */
        int it1 = (int)t1;
        int it2 = (int)t2;
        int it3 = it1 * it2;
        int it4 = it1 + it2;
        
        /* Group 4: Memory operations with dependencies */
        c[i] = t7 * t8 + (float)it3;
        
        /* Artificial dependency chain */
        t1 = t1 * vol_float1;
        t2 = t2 + vol_float2;
        
        /* Inline asm to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "mov $0, %%esi\n"
            "mov $0, %%edi\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static float mixed_dependency(float x, float y, int n) {
    float result = 0.0f;
    
    /* Create instruction delays through data dependencies */
    for (int i = 0; i < n; i++) {
        /* Long latency operation */
        float div_result = x / (y + 0.001f);
        
        /* Dependent chain */
        float dep1 = div_result * vol_float1;
        float dep2 = dep1 + vol_float2;
        float dep3 = dep2 * div_result;
        
        /* Independent calculations that can be scheduled */
        float indep1 = x * y;
        float indep2 = x + y;
        float indep3 = x - y;
        
        /* Resource conflict: use same functional unit */
        float conflict1 = sqrtf(indep1);  /* FP sqrt has latency */
        float conflict2 = sqrtf(indep2);  /* Another sqrt - competes for unit */
        
        /* Mix with volatile accesses */
        int v1 = vol_var1;
        int v2 = vol_var2;
        
        /* More independent ops */
        float mix1 = conflict1 * conflict2;
        float mix2 = (float)v1 * (float)v2;
        
        result += dep3 + mix1 + mix2;
        
        /* Change values to break pattern */
        x += 0.1f;
        y -= 0.05f;
    }
    
    return result;
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
static int control_flow_priority(int *arr, int size) {
    int sum = 0;
    int product = 1;
    
    for (int i = 0; i < size; i++) {
        /* Branch creates different priority paths */
        if (arr[i] > 0) {
            /* High priority path - critical operations */
            sum += arr[i] * vol_var1;
            product *= (arr[i] + 1);
            
            /* More independent ops in this path */
            int t1 = sum * 2;
            int t2 = product / 2;
            int t3 = t1 + t2;
            sum = t3 - arr[i];
        } else {
            /* Lower priority path */
            sum -= arr[i];
            product /= 2;
            
            /* Different operations with different latencies */
            float ft = (float)arr[i] * 0.5f;
            sum += (int)ft;
        }
        
        /* Common independent instructions */
        int common1 = arr[i] * i;
        int common2 = arr[i] + i;
        int common3 = common1 - common2;
        
        sum += common3;
        
        /* Break dependency occasionally */
        if (i % 10 == 0) {
            __asm__ volatile ("mfence" ::: "memory");
        }
    }
    
    return sum + product;
}

/* Main computational kernel */
__attribute__((noinline))
static float compute_kernel(float *a, float *b, float *c, 
                           int *int_arr, int size) {
    float total = 0.0f;
    
    /* Create scheduling complexity with multiple phases */
    for (int iter = 0; iter < 10; iter++) {
        /* Phase 1: High register pressure */
        high_pressure_loop(a, b, c, size / 2);
        
        /* Phase 2: Mixed dependencies */
        total += mixed_dependency(a[iter], b[iter], 50);
        
        /* Phase 3: Control flow with priority differences */
        total += control_flow_priority(int_arr, size / 4);
        
        /* Modify data to prevent optimization */
        for (int i = 0; i < size; i++) {
            a[i] += 0.01f;
            b[i] -= 0.005f;
            c[i] = a[i] * b[i];
            int_arr[i] = (int)c[i];
        }
    }
    
    return total;
}

int main() {
    /* Allocate and initialize data */
    float *a = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *b = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *c = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    int *int_arr = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f;
        int_arr[i] = rand() % 100 - 50;
    }
    
    /* Perform computation - this is where scheduling happens */
    float result = 0.0f;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        result += compute_kernel(a, b, c, int_arr, ARRAY_SIZE);
        
        /* Prevent optimization */
        vol_var1 = (int)result % 100;
        vol_var2 = (int)result % 50 + 1;
        vol_float1 = result * 0.1f;
        vol_float2 = result * 0.2f;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(int_arr);
    
    return 0;
}
