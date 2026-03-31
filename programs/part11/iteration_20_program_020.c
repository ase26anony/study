/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    volatile int viter = iter; /* Prevent constant propagation */
    int i, j;
    int sum = 0;
    
    /* Arrays with mapping to force offloading */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + viter;
        b[i] = i * 2 - viter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel shared(a, b, c) private(i, j) reduction(+:sum)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                private(i, j)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + (i * j);
                
                /* Create internal basic block split - early exit condition */
                if (c[idx] > 10000 && j % 16 == 0) {
                    /* Force multiple basic blocks with dummy operation */
                    c[idx] = c[idx] / 2;
                    /* This creates control flow divergence */
                }
                
                /* Additional computation to prevent loop simplification */
                c[idx] += (idx % 7) - (idx % 5);
            }
        }
        
        /* Post-process results in parallel region */
        #pragma omp for simd reduction(+:local_sum)
        for (i = 0; i < TOTAL; i++) {
            if (c[i] > 0) {
                local_sum += c[i] % 256;
            } else {
                local_sum -= (-c[i]) % 256;
            }
        }
        sum += local_sum;
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int vsize = size;
    int x[256], y[256], z[256];
    int result = 0;
    
    for (int i = 0; i < 256; i++) {
        x[i] = i * 3;
        y[i] = i * 5;
    }
    
    /* Nested parallel regions with target inside */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                #pragma omp target teams distribute parallel for simd \
                        if(flag) map(to: x[0:256], y[0:256]) map(from: z[0:256])
                for (int i = 0; i < vsize; i++) {
                    z[i] = x[i] * y[i];
                    
                    /* Multiple basic blocks within SIMD loop */
                    if (z[i] % 7 == 0) {
                        z[i] += i;
                        if (z[i] > 1000) {
                            z[i] = 1000;
                        }
                    } else {
                        z[i] -= i;
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:result)
        for (int i = 0; i < 256; i++) {
            result += z[i];
        }
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int threshold = 5;
        int n = iter + 3;
        
        /* Call with different conditions to trigger different paths */
        int res1 = simt_test(n, threshold, iter);
        int res2 = simt_test2((iter * 32) % 256, iter % 2);
        
        total += res1 + res2;
        printf("Iteration %d: res1=%d, res2=%d\n", iter, res1, res2);
    }
    
    printf("Total sum: %d\n", total);
    
    /* Additional test with dynamic bounds */
    volatile int dyn_size = 100;
    #pragma omp target teams distribute parallel for simd \
            if(dyn_size > 50) map(tofrom: dyn_size)
    for (int i = 0; i < dyn_size; i++) {
        /* Empty loop still generates SIMT structure */
    }
    
    return 0;
}
