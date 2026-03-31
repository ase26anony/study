/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, used))
int simt_test(int n, int threshold, int *result) {
    volatile int use_gpu = 0;
    int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(n, threshold)
    {
        int local_n = n;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(local_n > threshold) \
                map(tofrom: a, b, c) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Force basic block split with conditional */
                c[idx] = a[idx] + b[idx] + local_n;
                
                /* Early exit condition creating multiple basic blocks */
                if (c[idx] > 200 && j % 8 == 0) {
                    /* Dummy operation that can't be optimized away */
                    c[idx] = c[idx] % 100;
                    use_gpu = 1;  /* Volatile write */
                }
                
                /* Another conditional to create more control flow */
                if (i > n / 2 && c[idx] < 50) {
                    c[idx] = c[idx] * 2;
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    *result = sum;
    return use_gpu;
}

/* Helper to prevent loop optimization */
__attribute__((noinline))
void init_params(int *params, int count) {
    for (int i = 0; i < count; i++) {
        params[i] = (i * 7) % 13 + 1;
    }
}

int main() {
    int i, results[10];
    int params[10];
    int total_sum = 0;
    int gpu_used_count = 0;
    
    /* Initialize varying parameters */
    init_params(params, 10);
    
    /* Call test function with varying arguments */
    for (i = 0; i < 10; i++) {
        int threshold = 5;
        int result;
        int gpu_used = simt_test(params[i], threshold, &result);
        
        results[i] = result;
        total_sum += result;
        gpu_used_count += gpu_used;
        
        /* Print to prevent dead code elimination */
        printf("Iteration %d: n=%d, result=%d, gpu_used=%d\n", 
               i, params[i], result, gpu_used);
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("GPU was used in %d iterations\n", gpu_used_count);
    
    /* Additional test with different collapse factor */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                collapse(3) if(1) map(tofrom: results) \
                num_teams(1) num_threads(32)
        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 5; y++) {
                for (int z = 0; z < 1; z++) {
                    int idx = x * 5 + y;
                    results[idx] += x + y + z;
                }
            }
        }
    }
    
    /* Final reduction */
    int final_check = 0;
    for (i = 0; i < 10; i++) {
        final_check ^= results[i];
    }
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
