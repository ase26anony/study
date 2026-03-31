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
    volatile int use_gpu = (n > threshold);
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j, idx;
    int sum = 0;
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j, idx) firstprivate(n, threshold)
    {
        int local_n = n + omp_get_thread_num(); /* Make thread-dependent */
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(local_n > threshold) \
                map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < g_volatile_bound; i++) {  /* volatile prevents optimization */
            for (j = 0; j < M; j++) {
                idx = i * M + j;
                c[idx] = a[idx] + b[idx] + local_n;
                
                /* Create multiple basic blocks with conditional break */
                if (c[idx] > 150 && j > 30) {
                    /* Early exit creates extra labels/basic blocks */
                    c[idx] = 255; /* Force different value */
                    if (i > n/2) {
                        /* Nested condition for more complex CFG */
                        c[idx] = 128;
                    }
                } else if (c[idx] < 0) {
                    /* Another branch for label generation */
                    c[idx] = 0;
                }
                
                /* Force SIMD transformation with reduction-like pattern */
                c[idx] += (idx % 8); /* Non-uniform access pattern */
            }
        }
        
        /* Additional computation outside target to maintain context */
        #pragma omp for simd reduction(+:sum) schedule(simd:static)
        for (idx = 0; idx < TOTAL; idx++) {
            sum += c[idx];
            /* Another conditional to preserve CFG complexity */
            if (sum > 1000000) {
                sum = 1000000; /* Cap the sum */
            }
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int size, int flag) {
    volatile int dyn_size = size;
    int x[256], y[256], z[256];
    int i, j, result = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i * flag;
        y[i] = 256 - i;
    }
    
    /* Different nesting: teams inside parallel */
    #pragma omp parallel
    {
        #pragma omp target if(flag > 0) map(tofrom: x, y, z) nowait
        {
            #pragma omp teams distribute parallel for simd \
                    num_teams(2) collapse(2)
            for (i = 0; i < dyn_size; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    z[idx] = x[idx] * y[idx];
                    
                    /* Complex conditional with early continue */
                    if (z[idx] % 3 == 0) {
                        z[idx] /= 3;
                        continue;
                    }
                    if (z[idx] > 1000) {
                        z[idx] = 1000;
                        /* Simulate early exit with goto-like pattern */
                        if (i > dyn_size/2) break;
                    }
                }
            }
        }
        
        #pragma omp taskwait
        
        #pragma omp for simd reduction(+:result)
        for (i = 0; i < 256; i++) {
            result += z[i];
        }
    }
    
    return result;
}

int main() {
    int total_sum = 0;
    int i;
    
    /* Varying parameters prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i);
        int result2 = simt_test2(i * 16, i % 3);
        
        total_sum += result1 + result2;
        
        /* Print to prevent dead code elimination */
        printf("Iteration %d: result1=%d, result2=%d\n", 
               i, result1, result2);
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with larger data to trigger different heuristics */
    #pragma omp parallel
    {
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: total_sum) if(total_sum > 1000)
        for (i = 0; i < 100; i++) {
            #pragma omp atomic
            total_sum += i;
        }
    }
    
    printf("Final total: %d\n", total_sum);
    return 0;
}
