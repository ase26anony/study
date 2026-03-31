/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_bound = N;
volatile int g_volatile_m = M;
volatile int g_volatile_threshold = 5;

__attribute__((noinline, cold))
int simt_test(int n, int threshold) {
    volatile int i, j;  /* volatile to prevent optimization */
    int sum = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel reduction(+:sum) private(i, j)
    {
        /* Dynamic arrays to force heap allocation */
        int *a = (int*)malloc(SIZE * sizeof(int));
        int *b = (int*)malloc(SIZE * sizeof(int));
        int *c = (int*)malloc(SIZE * sizeof(int));
        
        if (!a || !b || !c) {
            #pragma omp single
            fprintf(stderr, "Allocation failed\n");
            free(a); free(b); free(c);
            return -1;
        }
        
        /* Initialize arrays */
        #pragma omp for collapse(2)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < g_volatile_m; j++) {
                int idx = i * M + j;
                a[idx] = i + j;
                b[idx] = i - j;
                c[idx] = 0;
            }
        }
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:SIZE], b[0:SIZE]) \
                map(from: c[0:SIZE]) \
                private(i, j)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < g_volatile_m; j++) {
                int idx = i * M + j;
                c[idx] = a[idx] + b[idx];
                
                /* Create basic block split with conditional */
                if (c[idx] > (i * j) % 100) {
                    /* Dummy operation to prevent dead code elimination */
                    c[idx] += (idx % 7);
                    
                    /* Early break simulation - creates additional labels */
                    if (c[idx] > 1000 && i > 20) {
                        /* This creates another basic block */
                        c[idx] = 1000;
                    }
                } else {
                    /* Alternative path */
                    c[idx] -= (idx % 5);
                }
            }
        }
        
        /* Reduction on CPU side */
        #pragma omp for collapse(2) reduction(+:sum)
        for (i = 0; i < g_volatile_bound; i++) {
            for (j = 0; j < g_volatile_m; j++) {
                int idx = i * M + j;
                sum += c[idx];
                
                /* Another conditional to maintain complexity */
                if (sum < 0) {
                    sum = 0;  /* Reset - creates control flow */
                }
            }
        }
        
        free(a); free(b); free(c);
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, cold))
int simt_test2(int n, int threshold) {
    volatile int i, j;
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    int sum = 0;
    
    /* Initialize */
    #pragma omp parallel for simd collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int idx = i * M + j;
            arr1[idx] = (i * j) % 97;
            arr2[idx] = (i + j) % 113;
        }
    }
    
    /* Nested parallel region with target inside */
    #pragma omp parallel reduction(+:sum)
    {
        #pragma omp single
        {
            /* Teams construct with distribute */
            #pragma omp target teams distribute parallel for simd \
                    if(n < threshold * 2) \
                    map(to: arr1, arr2) map(from: arr3) \
                    collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    int idx = i * M + j;
                    arr3[idx] = arr1[idx] * arr2[idx];
                    
                    /* Complex conditional structure */
                    switch (arr3[idx] % 4) {
                        case 0: arr3[idx] += 1; break;
                        case 1: arr3[idx] -= 1; break;
                        case 2: arr3[idx] *= 2; break;
                        case 3: arr3[idx] /= 2; break;
                    }
                }
            }
        }
        
        /* Reduction */
        #pragma omp for simd reduction(+:sum)
        for (i = 0; i < SIZE; i++) {
            sum += arr3[i];
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    volatile int i;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int result1 = simt_test(i, g_volatile_threshold);
        int result2 = simt_test2(i, g_volatile_threshold);
        
        total += result1 + result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
        
        /* Prevent loop unrolling */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    printf("Total sum: %d\n", total);
    printf("Test completed.\n");
    
    return 0;
}
