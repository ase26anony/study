/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, used))
int simt_test(int n, int threshold, int iter) {
    volatile int i, j;
    int sum = 0;
    int a[N*M], b[N*M], c[N*M];
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < N*M; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(n, threshold)
    {
        int local_n = n;
        int local_thresh = threshold;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(local_n > local_thresh) \
                map(to: a[0:N*M], b[0:N*M]) map(from: c[0:N*M]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < local_n; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + iter;
                
                /* Create internal basic block split */
                if (c[idx] > 150 && iter % 2 == 0) {
                    /* Early exit-like construct to force label generation */
                    c[idx] = c[idx] % 100;
                    /* This creates multiple basic blocks */
                    if (c[idx] < 50) {
                        c[idx] = 99;
                    }
                } else if (c[idx] < 50) {
                    c[idx] = c[idx] * 2;
                }
            }
        }
        
        /* Reduction in host after target region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < local_n * M; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline, used))
int simt_test2(int size, int flag) {
    volatile int x, y;
    int total = 0;
    int arr1[256], arr2[256], res[256];
    
    for (x = 0; x < 256; x++) {
        arr1[x] = x * flag;
        arr2[x] = 256 - x;
    }
    
    #pragma omp parallel
    {
        /* Nested target with teams and simd */
        #pragma omp target teams distribute parallel for simd \
                if(flag > 0) map(to: arr1, arr2) map(from: res) \
                collapse(1) num_teams(2)
        for (x = 0; x < size; x++) {
            res[x] = arr1[x] * arr2[x];
            
            /* Complex conditional to force label creation */
            switch (res[x] % 4) {
                case 0:
                    res[x] += flag;
                    break;
                case 1:
                    res[x] -= flag;
                    /* Fall through to create more blocks */
                case 2:
                    res[x] *= 2;
                    break;
                default:
                    res[x] = res[x] / 2;
                    if (res[x] < 0) {
                        res[x] = 0;
                    }
            }
        }
        
        #pragma omp for reduction(+:total)
        for (x = 0; x < size; x++) {
            total += res[x];
        }
    }
    
    return total;
}

int main() {
    int i, results[10];
    volatile int threshold = 5;
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Call test functions with varying parameters to prevent constant prop */
    for (i = 0; i < 10; i++) {
        results[i] = simt_test(i + 1, threshold, i);
        printf("Iteration %d: result = %d\n", i, results[i]);
        
        /* Alternate between two different test patterns */
        if (i % 3 == 0) {
            int res2 = simt_test2(256, i);
            printf("  Secondary test: %d\n", res2);
        }
    }
    
    /* Final validation sum */
    int final_sum = 0;
    for (i = 0; i < 10; i++) {
        final_sum += results[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
