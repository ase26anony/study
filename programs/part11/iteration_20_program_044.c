/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
__attribute__((noinline, cold))
int simt_test(int n, int threshold) {
    volatile int i, j;
    int result = 0;
    
    /* Arrays with mapping to force offloading */
    int a[N*M], b[N*M], c[N*M];
    
    /* Initialize arrays */
    for (i = 0; i < N*M; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:result)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:N*M], b[0:N*M]) map(from: c[0:N*M]) \
                private(i, j)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j % 8 == 0) {
                    /* Early exit from inner loop - creates extra labels */
                    c[idx] = c[idx] % 100;
                    if (i > n && n > threshold) {
                        /* Complex condition to prevent optimization */
                        c[idx] += threshold;
                    }
                } else if (c[idx] < 0) {
                    /* Another basic block */
                    c[idx] = -c[idx];
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for simd reduction(+:local_sum)
        for (i = 0; i < N*M; i++) {
            local_sum += c[i];
        }
        result += local_sum;
    }
    
    return result;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int x, y;
    int arr[256][256];
    int sum = 0;
    
    #pragma omp parallel
    {
        /* Nested teams with distribute and simd */
        #pragma omp target teams distribute parallel for simd \
                if(flag) map(tofrom: arr) collapse(2) \
                num_teams(4) thread_limit(64)
        for (x = 0; x < size; x++) {
            for (y = 0; y < size; y++) {
                arr[x][y] = x * y + flag;
                
                /* Multiple basic blocks with goto-like logic */
                if (arr[x][y] % 7 == 0) {
                    arr[x][y] /= 2;
                    if (x > y) {
                        arr[x][y] += size;
                    }
                }
            }
        }
        
        #pragma omp for simd reduction(+:sum) collapse(2)
        for (x = 0; x < size; x++) {
            for (y = 0; y < size; y++) {
                sum += arr[x][y];
            }
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    volatile int iter;
    
    /* Varying arguments to prevent constant propagation */
    for (iter = 1; iter <= 10; iter++) {
        int threshold = iter % 3;
        int result1 = simt_test(iter * 16, threshold);
        int result2 = simt_test2(iter * 8, iter % 2);
        
        total += result1 + result2;
        printf("Iteration %d: result1=%d, result2=%d\n", 
               iter, result1, result2);
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with dynamic teams */
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: total) if(total > 1000)
    for (int i = 0; i < 100; i++) {
        total += i % 10;
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
