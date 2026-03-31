/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization */
static volatile int global_seed = 42;
static int __attribute__((noinline)) get_seed() {
    return global_seed;
}

/* Test function with complex OpenMP nesting */
__attribute__((noinline))
int simt_test(int n, int threshold) {
    volatile int use_offload = n; /* Prevent constant propagation */
    int i, j;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[TOTAL], b[TOTAL];
    int c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + get_seed();
        b[i] = i * 2 + get_seed();
        c[i] = 0;
    }
    
    int result = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:result) private(i, j)
    {
        int local_n = use_offload; /* Force variable capture */
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(local_n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create internal basic block split - early exit condition */
                if (c[idx] > 10000) {
                    /* Force multiple basic blocks with dummy operation */
                    c[idx] = c[idx] % 1000;
                    /* This creates control flow divergence */
                }
                
                /* Additional computation to prevent loop simplification */
                c[idx] += (i * j) & 0xFF;
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for simd reduction(+:result)
        for (i = 0; i < TOTAL; i++) {
            result += c[i];
        }
    }
    
    return result;
}

/* Another test with different construct nesting */
__attribute__((noinline))
int simt_test2(int size, int flag) {
    volatile int dynamic_size = size;
    int x[256], y[256], z[256];
    int i, j;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = i * 3;
        z[i] = 0;
    }
    
    int sum = 0;
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag) map(to: x, y) map(from: z) \
                    collapse(2) num_teams(2)
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    z[idx] = x[idx] * y[idx];
                    
                    /* Force label generation with conditional break */
                    if (z[idx] > 1000 && i > 8) {
                        z[idx] = 999; /* Creates another basic block */
                    }
                }
            }
        }
        
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < 256; i++) {
            sum += z[i];
        }
    }
    
    return sum;
}

int main() {
    int i, total_result = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold);
        int result2 = simt_test2(i * 16, i % 2);
        
        total_result += result1 + result2;
        
        /* Print to prevent dead code elimination */
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    printf("Total result: %d\n", total_result);
    
    /* Additional test with explicit SIMD clause */
    {
        int small_arr[100];
        #pragma omp parallel for simd simdlen(8)
        for (i = 0; i < 100; i++) {
            small_arr[i] = i * i;
        }
        
        int check = 0;
        #pragma omp simd reduction(+:check)
        for (i = 0; i < 100; i++) {
            check += small_arr[i];
        }
        printf("SIMD check: %d\n", check);
    }
    
    return total_result > 0 ? 0 : 1;
}
