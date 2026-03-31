/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int global_threshold = 5;
__attribute__((noinline, used))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* volatile to prevent optimization */
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:sum)
    {
        /* Dynamic arrays to force heap allocation */
        int *a = (int*)malloc(SIZE * sizeof(int));
        int *b = (int*)malloc(SIZE * sizeof(int));
        int *c = (int*)malloc(SIZE * sizeof(int));
        
        if (!a || !b || !c) {
            #pragma omp critical
            printf("Allocation failed\n");
            return -1;
        }
        
        /* Initialize arrays */
        for (i = 0; i < SIZE; i++) {
            a[i] = i % 100;
            b[i] = (i * 2) % 100;
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE]) \
                private(i, j)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx] + n;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 150 && j > M/2) {
                    /* Early exit creates extra control flow */
                    c[idx] = 150;  /* Force value */
                    /* Can't break from collapsed loop easily,
                       but this creates internal basic block */
                }
                
                /* Additional computation to prevent simplification */
                if (c[idx] % 7 == 0) {
                    c[idx] += 1;
                }
            }
        }
        
        /* Process results with reduction */
        for (i = 0; i < SIZE; i++) {
            sum += c[i];
            /* Another conditional to maintain control flow complexity */
            if (c[i] > 200 && (i % 8 == 0)) {
                sum -= 1;  /* Dummy operation */
            }
        }
        
        free(a);
        free(b);
        free(c);
    }
    
    return sum;
}

/* Another test function with different loop structure */
__attribute__((noinline, used))
int simt_test2(int n, int threshold) {
    volatile int i, j, k;
    int sum = 0;
    static int array[64][64];  /* Static to force different mapping */
    
    #pragma omp parallel for private(i, j, k) collapse(2) reduction(+:sum)
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            array[i][j] = i * j;
        }
    }
    
    /* Nested parallel region with target inside */
    #pragma omp parallel private(i, j, k)
    {
        #pragma omp target teams distribute parallel for simd \
                if(n < threshold * 2) \
                map(tofrom: array) \
                collapse(2)
        for (i = 1; i < 63; i++) {
            for (j = 1; j < 63; j++) {
                /* Stencil computation */
                int val = array[i-1][j] + array[i+1][j] +
                         array[i][j-1] + array[i][j+1];
                
                /* Complex conditional to create labels */
                if (val > 1000) {
                    array[i][j] = val / 4;
                    if (val > 2000 && (i + j) % 3 == 0) {
                        array[i][j] -= 1;
                    }
                } else if (val < 100) {
                    array[i][j] = val * 2;
                } else {
                    array[i][j] = val;
                }
            }
        }
        
        #pragma omp for collapse(2) nowait
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                sum += array[i][j];
            }
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    volatile int i;  /* volatile to prevent loop optimization */
    
    printf("Testing SIMT transformation paths...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int result1 = simt_test(i, global_threshold);
        int result2 = simt_test2(i, global_threshold);
        
        total += result1 + result2;
        
        #pragma omp parallel
        {
            #pragma omp master
            printf("Iteration %d: results = %d, %d\n", i, result1, result2);
        }
    }
    
    /* Additional test with dynamic thread count */
    omp_set_num_threads(4);
    #pragma omp parallel
    {
        #pragma omp single
        {
            int final_test = simt_test(8, 3);
            printf("Final test with threads: %d\n", final_test);
            total += final_test;
        }
    }
    
    printf("Total sum: %d\n", total);
    return total > 0 ? 0 : 1;
}
