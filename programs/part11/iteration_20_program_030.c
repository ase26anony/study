/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization of test function */
__attribute__((noinline)) 
int simt_test(int n, int threshold, int iter) {
    volatile int size = TOTAL; /* volatile to prevent optimization */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(size)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create internal basic block split - early exit condition */
                if (c[idx] > 200) {
                    /* Dummy operation that creates control flow */
                    c[idx] = c[idx] % 100;
                    /* This creates multiple basic blocks */
                }
                
                /* Additional computation to prevent loop simplification */
                c[idx] += (i * j) & 0xFF;
            }
        }
        
        /* Barrier to ensure target region completes */
        #pragma omp barrier
        
        /* Reduction in parallel region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < TOTAL; i++) {
            sum += c[i];
        }
    }
    
    return sum;
}

/* Another test function with different nesting pattern */
__attribute__((noinline))
int simt_test2(int n, int flag) {
    volatile int dim = 50;
    int x[2500], y[2500], z[2500];
    int i, j, result = 0;
    
    for (i = 0; i < 2500; i++) {
        x[i] = i % 97;
        y[i] = (i * 3) % 97;
    }
    
    /* Different nesting: parallel region inside another construct */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag) map(to: x[0:2500], y[0:2500]) map(from: z[0:2500]) \
                    collapse(2)
            for (i = 0; i < dim; i++) {
                for (j = 0; j < dim; j++) {
                    int idx = i * dim + j;
                    z[idx] = x[idx] * y[idx] + n;
                    
                    /* Multiple basic blocks with conditional return pattern */
                    if (z[idx] > 1000 && i > j) {
                        z[idx] = z[idx] / 2;
                        if (z[idx] < 100) {
                            z[idx] += 50;
                        }
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:result)
        for (i = 0; i < 2500; i++) {
            result += z[i];
        }
    }
    
    return result;
}

int main() {
    int total_sum = 0;
    int threshold = 5;
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int n = iter + 1;
        
        /* Call with varying conditions to trigger different paths */
        int res1 = simt_test(n, threshold, iter);
        int res2 = simt_test2(n, iter % 2);
        
        total_sum += res1 + res2;
        
        printf("Iteration %d: res1 = %d, res2 = %d\n", 
               iter, res1, res2);
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with dynamic teams */
    #pragma omp parallel
    {
        int arr[100];
        #pragma omp for
        for (int i = 0; i < 100; i++) arr[i] = i;
        
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr) if(1) num_teams(8)
        for (int i = 0; i < 100; i++) {
            arr[i] *= 2;
            if (arr[i] > 100) arr[i] -= 50;
        }
    }
    
    return 0;
}
