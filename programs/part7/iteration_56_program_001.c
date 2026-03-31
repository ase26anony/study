/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M, int LIMIT) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int result = 0;
    
    /* Complex loop nest with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            for (int k = 0; k < 3; ++k) {
                __asm__ volatile("" : : : "memory");  /* Memory barrier */
                s += k;
            }
        } else if (s < -LIMIT) {
            s = 1;
            /* Different operations in else-if path */
            for (int k = 0; k < 2; ++k) {
                __asm__ volatile("" : : : "memory");
                s -= k;
            }
        } else {
            /* Default path with switch statement */
            switch (i % 4) {
                case 0: s += 1; break;
                case 1: s -= 1; break;
                case 2: s *= 2; break;
                case 3: s /= 2; break;
            }
        }
        
        /* Inner loop with variable bounds */
        int inner_bound = M - (i % 5);
        for (int j = 0; j < inner_bound; ++j) {
            /* Complex array access with variable index */
            int idx = (i + j) % M;
            c[idx] += s * j;
            
            /* More arithmetic to increase register pressure */
            c[idx] += a[i] * b[(i + j) % N];
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional conditional with side effects */
        if (i % 7 == 0) {
            for (int j = 0; j < 2; ++j) {
                s += c[j] * trigger;
            }
        }
        
        /* Accumulate result */
        result += s;
    }
    
    return result;
}

/* Another non-inlineable function to create more scheduling opportunities */
__attribute__((noinline))
static void process_array(int *arr, int size, volatile int *control) {
    for (int i = 0; i < size; ++i) {
        /* Variable stride access pattern */
        int idx = (*control + i) % size;
        arr[idx] = arr[idx] * 3 + 1;
        
        /* Conditional with unpredictable branch */
        if (arr[idx] % 2 == 0) {
            arr[idx] /= 2;
        } else {
            arr[idx] = arr[idx] * 5 - 3;
        }
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            __asm__ volatile("" : : : "memory");
        }
    }
}

int main(void) {
    /* Use runtime constants to prevent compile-time optimization */
    const int N = 100;
    const int M = 50;
    const int LIMIT = 1000;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    /* Volatile control variable */
    volatile int control = 0;
    
    /* Call the complex computation function */
    int checksum = compute_checksum(a, b, c, N, M, LIMIT);
    
    /* Process arrays further to create more scheduling opportunities */
    process_array(a, N, &control);
    process_array(b, N, &control);
    
    /* Compute final checksum to prevent optimization */
    int final_result = checksum;
    for (int i = 0; i < N; ++i) {
        final_result += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_result += c[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
