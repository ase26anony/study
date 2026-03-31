/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization */
static volatile int force_offload = 1;
static volatile int loop_bound = N;

/* Helper function to prevent inlining */
__attribute__((noinline, target("noipa")))
int simt_test(int n, int threshold) {
    int i, j;
    int sum = 0;
    
    /* Use volatile to prevent optimization */
    volatile int *a = (volatile int*)malloc(SIZE * sizeof(int));
    volatile int *b = (volatile int*)malloc(SIZE * sizeof(int));
    volatile int *c = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) return -1;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(n, threshold)
    {
        int local_n = n;
        int local_thresh = threshold;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(local_n > local_thresh) \
                map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE]) \
                num_teams(2) thread_limit(128)
        for (i = 0; i < loop_bound; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Force basic block split with conditional */
                c[idx] = a[idx] + b[idx];
                
                /* Create internal control flow with early exit possibility */
                if (c[idx] > 150 && force_offload) {
                    /* Dummy operation to prevent dead code elimination */
                    c[idx] = c[idx] % 100;
                }
                
                /* Another conditional to create more basic blocks */
                if (idx % 7 == 0 && local_n > 5) {
                    c[idx] += 1;
                }
            }
        }
        
        /* Reduction outside target region */
        #pragma omp for reduction(+:sum)
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
        }
    }
    
    free((void*)a);
    free((void*)b);
    free((void*)c);
    
    return sum;
}

/* Another test function with different nesting pattern */
__attribute__((noinline, target("noipa")))
int simt_test2(int n, int threshold) {
    int i, j;
    volatile int result = 0;
    
    #pragma omp parallel
    {
        volatile int local_arr[64];
        
        /* Initialize local array */
        for (i = 0; i < 64; i++) {
            local_arr[i] = i * omp_get_thread_num();
        }
        
        /* Nested target with teams and simd */
        #pragma omp target teams distribute parallel for simd \
                if(n > threshold) \
                map(to: local_arr[0:64]) \
                num_teams(1) num_threads(32)
        for (i = 0; i < 64; i++) {
            /* Complex loop body to encourage transformation */
            int temp = local_arr[i];
            for (j = 0; j < 3; j++) {
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                if (temp % 1000 == 0 && j == 1) {
                    break;  /* Early break creates more control flow */
                }
            }
            local_arr[i] = temp;
            result += temp;
        }
    }
    
    return result;
}

int main() {
    int i, total = 0;
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = (i % 3) + 2;  /* Vary threshold */
        int result = simt_test(i * 10, threshold);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
        
        /* Also call second test function */
        if (i % 2 == 0) {
            int result2 = simt_test2(i * 5, threshold);
            total += result2;
            printf("Iteration %d (test2): result = %d\n", i, result2);
        }
    }
    
    printf("Total sum: %d\n", total);
    
    /* Force one more execution with different conditions */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Final test with large data:\n");
        }
        
        volatile int final_check = 0;
        #pragma omp target teams distribute parallel for simd \
                if(force_offload) \
                map(tofrom: final_check) \
                reduction(+:final_check)
        for (i = 0; i < 1000; i++) {
            final_check += i % 77;
            if (i == 999 && force_offload) {
                final_check += 1000;  /* Extra branch */
            }
        }
        
        #pragma omp single
        {
            printf("Final check value: %d\n", final_check);
        }
    }
    
    return 0;
}
