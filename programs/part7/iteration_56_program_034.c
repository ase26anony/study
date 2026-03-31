/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 10000

/* Volatile variables to prevent optimization */
volatile int trigger1 = 0;
volatile int trigger2 = 1;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c) {
    int s = 0;
    int i, j;
    
    /* Complex outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Use volatile in array index to prevent optimization */
        int idx = i + trigger1;
        if (idx >= N) idx = idx % N;
        
        /* Multiple arithmetic operations with dependencies */
        int temp = a[idx] * b[idx];
        s += temp;
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: reset path */
            for (int k = 0; k < 4; ++k) {
                s += k * trigger2;
            }
        } else if (s > LIMIT / 2) {
            /* Path B: intermediate path */
            s = s / 2;
            /* Inline assembly to create artificial dependencies */
            __asm__ volatile("" : : : "memory");
        } else {
            /* Path C: normal path */
            s = s + (i % 8);
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile influence */
            int jdx = j + (trigger1 % 4);
            if (jdx >= M) jdx = jdx % M;
            
            /* Multiple operations to increase register pressure */
            int mult = s * jdx;
            c[jdx] += mult;
            
            /* Conditional inside inner loop */
            if (c[jdx] > 1000000) {
                c[jdx] = c[jdx] % 1000;
            }
            
            /* More arithmetic to create scheduling opportunities */
            c[jdx] = c[jdx] + (i * j) % 256;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += a[i % 16];
                break;
            case 1:
                s -= b[i % 16];
                break;
            case 2:
                s *= 2;
                break;
            case 3:
                s = s ^ 0x55;
                break;
        }
        
        /* Prevent loop unrolling */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_arrays(int *a, int *b, int *c, int *d) {
    int sum = 0;
    
    /* Loop with data-dependent branches */
    for (int i = 0; i < N; i += 2) {
        int idx = i + (trigger2 % 8);
        
        /* Complex expression with multiple operations */
        int val1 = a[idx] * 3 + b[idx] * 7;
        int val2 = c[idx] * 5 - d[idx] * 2;
        
        /* Nested conditionals */
        if (val1 > val2) {
            if (val1 > 1000) {
                a[idx] = val2;
            } else {
                b[idx] = val1;
            }
        } else {
            if (val2 < 0) {
                c[idx] = -val2;
            } else {
                d[idx] = val1 + val2;
            }
        }
        
        /* Update sum with complex expression */
        sum += (a[idx] + b[idx] + c[idx] + d[idx]) * (i % 16);
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Use the sum to prevent dead code elimination */
    trigger1 = sum % 100;
}

int main() {
    /* Initialize arrays with pseudo-random values */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    /* Simple deterministic RNG */
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        d[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = rand() % 1000;
    }
    
    /* Call the complex function */
    int checksum = compute_checksum(a, b, c);
    
    /* Process arrays with another pattern */
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i] + d[i];
    }
    for (int i = 0; i < M; i++) {
        final_sum += c[i];
    }
    final_sum += checksum;
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
