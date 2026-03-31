/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c) {
    volatile int trigger = 1;  /* Prevent optimization */
    int s = 0;
    int result = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Memory barrier to prevent reordering */
            __asm__ volatile("" : : : "memory");
        } else if (s < -LIMIT) {
            s = 1;
            __asm__ volatile("" : : : "memory");
        } else {
            /* Another arithmetic operation */
            s = s * 2 - a[i];
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger * 3;
                break;
            case 1:
                s -= trigger * 2;
                break;
            case 2:
                s *= (trigger + 1);
                break;
            default:
                s = s / (trigger + 2);
                break;
        }
        
        /* Inner loop with more register pressure */
        for (int j = 0; j < M; ++j) {
            /* Complex index calculation */
            int idx = (i + j) % M;
            c[idx] += s * j;
            
            /* More conditional logic */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Additional arithmetic with volatile */
            result += c[idx] * trigger;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return result;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline))
void initialize_arrays(int *a, int *b, int *c) {
    volatile int seed = 42;  /* Prevent constant propagation */
    
    for (int i = 0; i < N; ++i) {
        /* Pseudo-random but deterministic initialization */
        a[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        b[i] = (i * 1664525 + 1013904223) & 0x7fffffff;
        
        /* More complex initialization with branches */
        if (i % 3 == 0) {
            a[i] = a[i] % 100;
            b[i] = b[i] % 200;
        } else if (i % 3 == 1) {
            a[i] = a[i] % 300;
            b[i] = b[i] % 400;
        } else {
            a[i] = a[i] % 500;
            b[i] = b[i] % 600;
        }
        
        /* Use volatile in calculation */
        a[i] += seed;
        b[i] -= seed;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = 0;
    }
}

int main(void) {
    /* Dynamically allocate to avoid stack overflow */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with complex patterns */
    initialize_arrays(a, b, c);
    
    /* Call the function designed to trigger selective scheduling */
    int checksum = compute_checksum(a, b, c);
    
    /* Additional computation to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < N; ++i) {
        final_result ^= a[i] ^ b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_result ^= c[i];
    }
    final_result += checksum;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
