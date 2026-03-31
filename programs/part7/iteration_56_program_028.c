#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define LIMIT 1000

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c) {
    volatile int trigger = 1;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            a[i] = (a[i] * 3) / 2;
        } else if (s < -LIMIT) {
            s = LIMIT / 2;
            b[i] = (b[i] + i) * 2;
        } else {
            /* Third path with different operations */
            s = s * 2 - s / 3;
            __asm__ volatile("" : : : "memory");
        }
        
        /* Switch statement for more basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger * 2;
                break;
            case 1:
                s -= trigger;
                break;
            case 2:
                s *= (trigger + 1);
                break;
            default:
                s = s / (trigger + 2);
                break;
        }
        
        /* Inner loop with array accesses */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = 10000;
            } else if (c[idx] < -10000) {
                c[idx] = -10000;
            }
            
            /* Prevent loop unrolling */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional arithmetic to increase register pressure */
        a[i] = (a[i] + s) ^ (b[i] - i);
        b[i] = (b[i] * 2) | (s & 0xFF);
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *a, int *b, int *c, int *d) {
    volatile int v = 7;
    int i, j, k;
    
    /* Triple nested loop for more scheduling complexity */
    for (i = 0; i < N/2; ++i) {
        for (j = 0; j < M/2; ++j) {
            for (k = 0; k < 10; ++k) {
                /* Complex expression with multiple dependencies */
                d[i] = a[i] * b[j] + c[k] * v;
                
                /* Conditional with side effects */
                if ((i + j + k) % 3 == 0) {
                    a[i] += d[i];
                    __asm__ volatile("" : : : "memory");
                } else if ((i + j + k) % 3 == 1) {
                    b[j] -= d[i];
                } else {
                    c[k] ^= d[i];
                }
                
                /* More arithmetic operations */
                v = (v * 13 + 17) % 100;
            }
        }
    }
}

int main(void) {
    int a[N], b[N], c[M], d[N/2];
    int i, result1, result2 = 0;
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    for (i = 0; i < N/2; ++i) {
        d[i] = 0;
    }
    
    /* Call functions that should trigger selective scheduling */
    result1 = compute_checksum(a, b, c);
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    for (i = 0; i < N; ++i) {
        result2 += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        result2 += c[i];
    }
    for (i = 0; i < N/2; ++i) {
        result2 += d[i];
    }
    
    printf("Result1: %d, Result2: %d\n", result1, result2);
    return 0;
}
