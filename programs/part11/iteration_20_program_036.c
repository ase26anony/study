/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64

/* Prevent optimization */
volatile int g_volatile_bound = N;
volatile int g_volatile_threshold = 5;

/* Helper function to prevent inlining */
__attribute__((noinline)) 
int simt_test(int n, int threshold) {
    int i, j;
    int result = 0;
    
    /* Use volatile to prevent optimization */
    volatile int vol_n = n;
    volatile int vol_thresh = threshold;
    
    /* Arrays that will be mapped to device */
    int a[N*M], b[N*M], c[N*M];
    
    /* Initialize arrays */
    for (i = 0; i < N*M; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(vol_n, vol_thresh)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(vol_n > vol_thresh) \
                map(to: a[0:N*M], b[0:N*M]) \
                map(from: c[0:N*M]) \
                reduction(+:local_sum)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + vol_n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > 30) {
                    /* Early exit creates additional labels/basic blocks */
                    c[idx] = c[idx] % 100;
                    if (i % 2 == 0) {
                        /* Another nested condition for more complex CFG */
                        local_sum += 1;
                    }
                } else if (c[idx] < 50) {
                    /* Alternative path */
                    c[idx] = c[idx] * 2;
                }
                
                /* Force side effect */
                local_sum += c[idx] % 10;
            }
        }
        
        #pragma omp atomic
        result += local_sum;
    }
    
    /* Verify computation */
    int final_sum = 0;
    for (i = 0; i < N*M; i++) {
        final_sum += c[i];
    }
    
    return final_sum + result;
}

/* Another test function with different structure */
__attribute__((noinline))
int simt_test2(int n) {
    int x[1000], y[1000], z[1000];
    int i, j;
    
    for (i = 0; i < 1000; i++) {
        x[i] = i;
        y[i] = i * 2;
    }
    
    volatile int vol_n = n;
    
    /* Nested parallel regions */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Teams construct with distribute */
            #pragma omp target teams distribute parallel for simd \
                    if(vol_n > 0) \
                    map(to: x, y) map(from: z) \
                    num_teams(2) thread_limit(128)
            for (i = 0; i < 1000; i++) {
                z[i] = x[i] + y[i];
                
                /* Multiple basic blocks */
                if (z[i] > 1000) {
                    z[i] = 1000;
                    /* Early return simulation */
                    if (i > 500) {
                        z[i] = z[i] / 2;
                    }
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < 1000; i++) {
        sum += z[i];
    }
    return sum;
}

int main() {
    int i;
    int total = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = g_volatile_threshold;
        int result1 = simt_test(i, threshold);
        int result2 = simt_test2(i);
        
        total += result1 + result2;
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with dynamic bounds */
    #pragma omp parallel
    {
        int arr[256];
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr) if(1)  /* Always true condition */
        for (int k = 0; k < 256; k++) {
            arr[k] = k * k;
            if (arr[k] > 10000) {
                arr[k] = 10000;
            }
        }
    }
    
    return 0;
}
