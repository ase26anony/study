/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int size_a, int size_b, int size_c) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < size_a; ++i) {
        /* Multiple arithmetic operations with dependencies */
        s += a[i] * b[i % size_b];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            a[i] = (a[i] * 3) / 2;
        } else if (s < -LIMIT) {
            s = 1;
            /* Different operations in this path */
            a[i] = (a[i] * 2) / 3;
        } else {
            /* Third path with its own operations */
            a[i] = a[i] + (s % 256);
        }
        
        /* Artificial memory barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with variable bounds */
        int inner_limit = M - (i % 32);
        for (j = 0; j < inner_limit; ++j) {
            /* Complex addressing with multiple operations */
            int idx = (i * j) % size_c;
            c[idx] += s * j;
            
            /* More conditional logic inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = c[idx] % 10000;
                trigger = idx;  /* Use volatile variable */
            }
            
            /* Switch statement for additional basic blocks */
            switch (j % 4) {
                case 0:
                    c[idx] += trigger * 2;
                    break;
                case 1:
                    c[idx] -= trigger;
                    break;
                case 2:
                    c[idx] *= (trigger + 1);
                    break;
                case 3:
                    c[idx] /= (trigger > 0 ? trigger : 1);
                    break;
            }
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Additional arithmetic to increase register pressure */
        s = (s * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *arr1, int *arr2, int size) {
    volatile int v = 1;
    int i, j;
    
    for (i = 0; i < size; i += 2) {
        /* Unrolled loop with complex dependencies */
        int t1 = arr1[i];
        int t2 = arr2[i];
        int t3 = arr1[i + 1];
        int t4 = arr2[i + 1];
        
        /* Cross dependencies between iterations */
        arr1[i] = t1 * t4 + v;
        arr2[i] = t2 * t3 - v;
        arr1[i + 1] = t3 * t2 + (v << 1);
        arr2[i + 1] = t4 * t1 - (v >> 1);
        
        /* Variable shift to prevent optimization */
        v = (v * 1664525 + 1013904223) & 0xff;
        
        /* Nested loop with triangular iteration space */
        for (j = 0; j < i; ++j) {
            arr1[j] += arr2[size - j - 1];
            arr2[j] -= arr1[size - j - 1];
        }
    }
}

int main(void) {
    int i;
    int result;
    
    /* Allocate and initialize arrays with pseudo-random data */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(M * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        c[i] = 0;
    }
    for (i = 0; i < M; ++i) {
        b[i] = rand() % 1000;
    }
    
    /* Call the complex computation function */
    result = compute_checksum(a, b, c, N, M, N);
    
    /* Process arrays with different pattern */
    process_arrays(a, c, N);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (i = 0; i < N; ++i) {
        final_sum += a[i] + c[i];
    }
    for (i = 0; i < M; ++i) {
        final_sum += b[i];
    }
    final_sum += result;
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
