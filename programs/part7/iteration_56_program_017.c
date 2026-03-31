/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;
    int s = 0;
    int result = 0;
    
    /* Complex loop nest with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Artificial dependency via volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        s += (s << 3) ^ (s >> 5);  /* Non-linear mixing */
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = s % 1000;
            /* Additional operations in this path */
            s += trigger * 7;
        } else if (s < -500) {
            s = -s;
            s += trigger * 3;
        } else {
            /* Third path with different operations */
            s = (s * 13) & 0xFFF;
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < M; ++j) {
            /* Complex addressing with variable indices */
            int idx = (i + j) % M;
            c[idx] += s * j;
            
            /* More arithmetic to increase pressure */
            c[idx] = (c[idx] * 2) - (j << 1);
            
            /* Memory barrier to prevent optimization */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += 1;
                break;
            case 1:
                s -= 2;
                break;
            case 2:
                s *= 3;
                break;
            case 3:
                s = s / 4;
                break;
        }
        
        /* Accumulate result */
        result += s;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return result;
}

/* Another complex function to ensure multiple functions are scheduled */
__attribute__((noinline))
static void process_array(int *arr, int size) {
    volatile int v = 0;
    for (int i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int x = arr[i];
        x = (x * x) + (x << 2);
        
        /* Conditional with unpredictable branch */
        if (x & 1) {
            arr[i] = x + v;
        } else {
            arr[i] = x - v;
        }
        
        /* Update volatile to prevent dead code elimination */
        v = i % 16;
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8) + 1; ++k) {
            arr[i] += k * 3;
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Allocate and initialize arrays */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    
    /* Process arrays to create more scheduling opportunities */
    process_array(a, N);
    process_array(b, N);
    
    /* Call the main computation function */
    int checksum = compute_checksum(a, b, c, N, M);
    
    /* Additional computation to ensure everything is used */
    int final_result = checksum;
    for (int i = 0; i < M; ++i) {
        final_result += c[i];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
