/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 10000

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile int v1 = 1, v2 = 2, v3 = 3;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int *d) {
    int s = 0;
    int t = 0;
    int u = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Artificial dependency via inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Conditional with multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* More operations in this path */
            t += i * v1;
        } else if (s < -LIMIT) {
            s = LIMIT / 2;
            t -= i * v2;
        } else {
            /* Third path with different operations */
            t += s % 17;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                u += a[i] + b[i];
                break;
            case 1:
                u += a[i] - b[i];
                break;
            case 2:
                u += a[i] * 3;
                break;
            case 3:
                u += b[i] / 2;
                break;
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            /* Use volatile in index calculation */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            d[idx] += t * (j + 1);
            
            /* More conditional logic inside inner loop */
            if ((i + j) % 7 == 0) {
                c[idx] -= u;
            } else if ((i + j) % 7 == 1) {
                d[idx] += u * 2;
            }
            
            /* Prevent loop optimization */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Another conditional with side effects */
        if (i % 13 == 0) {
            trigger = (trigger + 1) % 5;
        }
    }
    
    /* Final computation with mixed operations */
    int result = s + t + u;
    for (int i = 0; i < 10; ++i) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline))
void process_arrays(int *arr1, int *arr2, int size) {
    int acc1 = 0, acc2 = 0;
    
    for (int i = 0; i < size; ++i) {
        /* Data-dependent branching */
        if (arr1[i] > arr2[i]) {
            acc1 += arr1[i] - arr2[i];
            /* Complex operation chain */
            arr1[i] = (arr1[i] * 3 + 7) % 256;
        } else {
            acc2 += arr2[i] - arr1[i];
            arr2[i] = (arr2[i] * 5 + 11) % 256;
        }
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8 + 1); ++k) {
            arr1[i] ^= (acc1 << k);
            arr2[i] ^= (acc2 >> k);
        }
    }
    
    /* Use the accumulators */
    trigger = (acc1 + acc2) % 100;
}

int main(void) {
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *d = (int*)malloc(M * sizeof(int));
    int *work1 = (int*)malloc(N * sizeof(int));
    int *work2 = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !work1 || !work2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        work1[i] = rand() % 500;
        work2[i] = rand() % 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 800;
        d[i] = rand() % 800;
    }
    
    /* Call the complex computation function */
    int checksum = compute_checksum(a, b, c, d);
    
    /* Process arrays with another complex function */
    process_arrays(work1, work2, N);
    
    /* Additional computation to use all results */
    int final_result = checksum;
    for (int i = 0; i < N; ++i) {
        final_result += a[i] + b[i] + work1[i] + work2[i];
    }
    
    for (int i = 0; i < M; ++i) {
        final_result += c[i] + d[i];
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    v3 = final_result % 1000;
    
    printf("Result: %d (volatile v3 = %d)\n", final_result, v3);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(work1);
    free(work2);
    
    return 0;
}
