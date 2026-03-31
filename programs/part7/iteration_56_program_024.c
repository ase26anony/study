/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Volatile variables to prevent optimization */
volatile int trigger1 = 0;
volatile int trigger2 = 1;

/* Function attributes to prevent inlining and affect scheduling */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c) {
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Array accesses with variable indices */
        int idx = (i + trigger1) % N;
        s += a[idx] * b[idx];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: complex arithmetic */
            for (int k = 0; k < 4; ++k) {
                s += (a[(i + k) % N] & 0xFF) - (b[(i + k) % N] & 0xFF);
            }
        } else if (s < -LIMIT) {
            s = 0;
            /* Path B: different operations */
            for (int k = 0; k < 4; ++k) {
                s ^= (a[(i + k) % N] | 0x55) + (b[(i + k) % N] | 0xAA);
            }
        } else {
            /* Path C: default operations */
            if (i % 3 == 0) {
                s *= 2;
            } else if (i % 3 == 1) {
                s /= 2;
            } else {
                s = -s;
            }
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with register pressure */
        for (j = 0; j < M; ++j) {
            /* Complex dependency chain */
            int jdx = (j + trigger2) % M;
            c[jdx] += s * j;
            
            /* More arithmetic to increase scheduling complexity */
            c[jdx] += (i & 0xF) * (j & 0xF);
            c[jdx] -= (i >> 4) ^ (j >> 4);
            
            /* Switch statement for additional basic blocks */
            switch (j % 4) {
                case 0:
                    c[jdx] += 1;
                    break;
                case 1:
                    c[jdx] -= 1;
                    break;
                case 2:
                    c[jdx] <<= 1;
                    break;
                case 3:
                    c[jdx] >>= 1;
                    break;
            }
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Additional conditional with volatile */
        if (trigger1) {
            s += trigger2;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_arrays(int *a, int *b, int *c, int *d) {
    int sum = 0;
    
    for (int i = 0; i < N; i += 2) {
        /* Unrolled loop with dependencies */
        int t1 = a[i] * b[i];
        int t2 = a[i + 1] * b[i + 1];
        
        /* Complex expression tree */
        sum += (t1 << 3) | (t2 & 0x7F);
        sum -= (t1 >> 5) ^ (t2 >> 2);
        
        /* Volatile access in loop */
        if (trigger1) {
            sum += *((volatile int *)&trigger2);
        }
        
        /* Nested loop with stride */
        for (int j = 0; j < M; j += 4) {
            d[j] += c[i] * (j + i);
            d[j + 1] += c[i + 1] * (j + i + 1);
            d[j + 2] += sum * j;
            d[j + 3] += sum * (j + 1);
        }
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Initialize arrays with pseudo-random values */
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(M * sizeof(int));
    int *d = malloc(M * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Deterministic initialization */
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = 0;
        d[i] = 0;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c);
    int result2 = process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = result1 + result2;
    for (int i = 0; i < M; i++) {
        final_sum += c[i] + d[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
