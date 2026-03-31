/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold, int iter) {
    volatile int use_offload = (n > threshold);
    int i, j;
    int result = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[TOTAL];
    volatile int b[TOTAL];
    volatile int c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + iter;
        b[i] = i * 2 + iter;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(use_offload)
    {
        int local_n = g_volatile_bound;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(use_offload) \
                map(tofrom: a, b, c) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < local_n; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create internal basic block split - early exit condition */
                if (c[idx] > 1000000 && iter > 1000) {
                    /* This creates additional control flow */
                    c[idx] = c[idx] % 1000;
                    /* Could add break here to force more complex CFG */
                }
                
                /* Additional computation to prevent loop simplification */
                if (j % 8 == 0) {
                    c[idx] += (i & 3);
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:result)
        for (i = 0; i < TOTAL; i++) {
            result += c[i];
        }
    }
    
    return result;
}

/* Another test with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int dyn_size = size;
    int x[256], y[256], z[256];
    int i, j, sum = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i * 3;
        y[i] = i * 2;
    }
    
    /* Complex nesting: parallel -> target -> teams -> distribute -> parallel for simd */
    #pragma omp parallel num_threads(2) if(flag > 0)
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag > 1) map(tofrom: x, y, z) \
                    collapse(2) schedule(static, 16)
            for (i = 0; i < dyn_size; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    z[idx] = x[idx] * y[idx] + flag;
                    
                    /* Force multiple basic blocks with conditional */
                    if (z[idx] % 7 == 0) {
                        z[idx] += 1;
                    } else if (z[idx] % 13 == 0) {
                        z[idx] -= 1;
                    } else {
                        z[idx] *= 2;
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:sum)
        for (i = 0; i < 256; i++) {
            sum += z[i];
        }
    }
    
    return sum;
}

int main() {
    int i, total = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i);
        int result2 = simt_test2(i * 4, i);
        
        total += result1 + result2;
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total sum: %d\n", total);
    
    /* Additional test with larger data */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: total) if(omp_get_thread_num() == 0)
        for (i = 0; i < 100; i++) {
            total += i;
        }
    }
    
    printf("Final total: %d\n", total);
    
    return 0;
}
