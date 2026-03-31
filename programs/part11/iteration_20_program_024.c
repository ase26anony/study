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
        b[i] = (i * 2) % 100;
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
                
                /* Create internal basic block split */
                if (c[idx] > 150) {
                    /* Early exit-like condition to force label generation */
                    if (c[idx] > 200 && j % 8 == 0) {
                        c[idx] = c[idx] / 2;  /* Dummy operation */
                    }
                }
                
                /* Another condition to create more control flow */
                if (i > vol_thresh && j < vol_thresh) {
                    c[idx] += 1;
                }
                
                local_sum += c[idx] % 10;
            }
        }
        
        #pragma omp atomic
        result += local_sum;
    }
    
    /* Final reduction */
    int final_sum = 0;
    for (i = 0; i < N*M; i++) {
        final_sum += c[i];
    }
    
    return final_sum + result;
}

/* Another test function with different structure */
__attribute__((noinline))
int simt_test2(int n, int threshold) {
    int a[256], b[256], c[256];
    int i, j;
    
    for (i = 0; i < 256; i++) {
        a[i] = i;
        b[i] = 256 - i;
    }
    
    /* Nested parallel regions */
    #pragma omp parallel
    {
        #pragma omp for collapse(2) nowait
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                
                /* Target with teams and simd */
                #pragma omp target teams distribute parallel for simd \
                        if(n > threshold) \
                        map(to: a[idx:1], b[idx:1]) \
                        map(from: c[idx:1])
                for (int k = 0; k < 10; k++) {
                    c[idx] = a[idx] * b[idx] + k;
                    
                    /* Multiple basic blocks */
                    if (c[idx] % 3 == 0) {
                        c[idx] += n;
                        if (k > 5) {
                            c[idx] -= threshold;
                        }
                    }
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < 256; i++) {
        sum += c[i];
    }
    return sum;
}

int main() {
    int i, total = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = i % 3;
        int result1 = simt_test(i, threshold);
        int result2 = simt_test2(i * 2, threshold);
        
        total += result1 + result2;
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with dynamic bounds */
    #pragma omp parallel
    {
        int dyn_size = 100 + (omp_get_thread_num() % 10);
        int x[dyn_size], y[dyn_size], z[dyn_size];
        
        #pragma omp target teams distribute parallel for simd \
                if(dyn_size > 50) \
                map(to: x[0:dyn_size], y[0:dyn_size]) \
                map(from: z[0:dyn_size])
        for (int i = 0; i < dyn_size; i++) {
            x[i] = i;
            y[i] = dyn_size - i;
            z[i] = x[i] * y[i];
            
            /* Complex control flow */
            if (z[i] % 7 == 0) {
                z[i] = z[i] / 7;
                if (i % 4 == 0) {
                    z[i] += dyn_size;
                }
            }
        }
    }
    
    return 0;
}
