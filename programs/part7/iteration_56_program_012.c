/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile int guard = 1;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c) {
    int s = 0;
    int i, j;
    
    /* Outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: reset path */
            for (j = 0; j < M/2; ++j) {
                c[j] += s * j;
                /* Artificial dependency via inline asm */
                __asm__ volatile("" : : : "memory");
            }
        } else if (s > LIMIT/2) {
            /* Path B: intermediate path */
            s = s / 2;
            for (j = 0; j < M; j += 2) {
                c[j] += s * (j + trigger);
                __asm__ volatile("" : : : "memory");
            }
        } else {
            /* Path C: normal path */
            for (j = 0; j < M; ++j) {
                /* Variable index using volatile */
                int idx = (j + trigger) % M;
                c[idx] += s * j;
                
                /* Switch statement inside inner loop */
                switch (j % 4) {
                    case 0:
                        c[idx] += 1;
                        break;
                    case 1:
                        c[idx] -= 1;
                        break;
                    case 2:
                        c[idx] *= 2;
                        break;
                    case 3:
                        c[idx] /= 2;
                        break;
                }
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Additional arithmetic to increase register pressure */
        s = s ^ (i * 31);
        s = s + (i << 3);
        s = s - (i >> 2);
        
        /* Use volatile in condition */
        if (guard) {
            s = s % 1000;
        }
    }
    
    return s;
}

/* Another complex function to ensure multiple scheduling regions */
__attribute__((noinline))
void process_arrays(int *a, int *b, int *c, int *d) {
    int temp = 0;
    
    /* Different loop structure */
    for (int i = N-1; i >= 0; --i) {
        temp = a[i] - b[i];
        
        /* Nested loops with dependencies */
        for (int j = 0; j < 8; ++j) {
            d[j] += temp * (i + j);
            
            /* Conditional with side effects */
            if (d[j] > 1000) {
                d[j] = d[j] % 1000;
                trigger = j;  /* Modify volatile */
            }
            
            __asm__ volatile("" : : : "memory");
        }
        
        /* Complex expression chain */
        c[i] = (c[i] * 3 + temp * 7) / 5;
        c[i] = c[i] ^ (temp & 0xFF);
    }
}

int main(void) {
    int a[N], b[N], c[M], d[8];
    int i, result;
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    for (i = 0; i < 8; ++i) {
        d[i] = 0;
    }
    
    /* Call functions with complex control flow */
    result = compute_checksum(a, b, c);
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    int checksum = result;
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    for (i = 0; i < 8; ++i) {
        checksum += d[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
