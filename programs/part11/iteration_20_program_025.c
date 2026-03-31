/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
/* Also try: gcc -O3 -fopenmp -foffload=amdgcn-amdhsa */

#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
volatile int g_volatile_threshold = 5;
static int __attribute__((noinline)) dummy_side_effect = 0;

/* Helper to prevent dead code elimination */
__attribute__((noinline, used))
static int use_result(int val) {
    dummy_side_effect += val;
    return dummy_side_effect;
}

/* Main test function with complex OpenMP nesting */
__attribute__((noinline))
int simt_test(int n, int threshold) {
    volatile int local_bound = M; /* Volatile to prevent optimization */
    int i, j;
    
    /* Arrays with mapping to force offloading */
    int a[N * M], b[N * M], c[N * M];
    
    /* Initialize arrays */
    for (i = 0; i < N * M; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(n, threshold)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(to: a[0:N*M], b[0:N*M]) \
                map(from: c[0:N*M]) \
                reduction(+:local_sum)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < local_bound; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150) {
                    /* Early exit path - creates extra labels */
                    c[idx] = 150;
                    /* Could break here but break in collapsed loop
                       creates complex control flow */
                }
                
                /* Another condition to split basic blocks */
                if (j % 32 == 0 && i > n) {
                    c[idx] += 1;
                }
                
                local_sum += c[idx];
            }
        }
        
        /* Use local_sum to prevent optimization */
        #pragma omp atomic
        dummy_side_effect += local_sum;
    }
    
    /* Final reduction */
    int total = 0;
    #pragma omp parallel for reduction(+:total) private(i)
    for (i = 0; i < N * M; i++) {
        total += c[i];
        /* Additional computation to prevent loop simplification */
        if (c[i] % 7 == 0) {
            total += 1;
        }
    }
    
    return total;
}

/* Another test with different construct nesting */
__attribute__((noinline))
int simt_test2(int size, int flag) {
    volatile int dyn_size = size;
    int x[1000], y[1000], z[1000];
    int i, result = 0;
    
    for (i = 0; i < 1000; i++) {
        x[i] = i;
        y[i] = i * 3;
    }
    
    /* Nested parallel with target containing simd */
    #pragma omp parallel num_threads(2) if(flag > 0)
    {
        #pragma omp target teams distribute parallel for simd \
                if(flag > 2) \
                map(to: x[0:dyn_size], y[0:dyn_size]) \
                map(from: z[0:dyn_size])
        for (i = 0; i < dyn_size; i++) {
            z[i] = x[i] + y[i];
            
            /* Complex control flow inside SIMD loop */
            if (z[i] > 1000) {
                z[i] = 1000;
                /* Simulate early exit with goto-like pattern */
                if (i > dyn_size / 2) {
                    z[i] = -1;
                }
            }
        }
        
        #pragma omp for simd reduction(+:result)
        for (i = 0; i < dyn_size; i++) {
            result += z[i];
        }
    }
    
    return result;
}

int main() {
    int i, total = 0;
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = (i % 3) + 2;  /* Vary between 2-4 */
        int result = simt_test(i * 16, threshold);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
        
        /* Second test with different parameters */
        if (i % 2 == 0) {
            int result2 = simt_test2(i * 50, i);
            total += result2;
            printf("  Test2: %d\n", result2);
        }
    }
    
    /* Use results to prevent elimination */
    total = use_result(total);
    printf("Final total: %d (dummy: %d)\n", total, dummy_side_effect);
    
    return 0;
}
