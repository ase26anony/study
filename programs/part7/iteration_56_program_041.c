/* sel-sched-trigger.c
 * Program designed to trigger selective scheduling RTL dumps in GCC
 * Specifically targets uncovered lines in sel-sched-dump.cc
 */

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
            a[i] = s;
        } else if (s > LIMIT / 2) {
            /* Path B: intermediate path */
            s = s / 2;
            b[i] = s;
        } else {
            /* Path C: normal path */
            s = s + trigger;  /* Use volatile */
            c[i % M] = s;
        }
        
        /* Inner loop with dependencies */
        for (j = 0; j < M; ++j) {
            /* Complex operation with artificial dependency */
            c[j] += s * j;
            
            /* Inline assembly barrier to prevent optimization */
            __asm__ volatile("" : : : "memory");
            
            /* Another conditional inside inner loop */
            if ((j & 3) == 0) {
                c[j] = c[j] * 2;
            } else if ((j & 3) == 1) {
                c[j] = c[j] + guard;  /* Use volatile */
            } else {
                c[j] = c[j] - 1;
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += 1;
                break;
            case 1:
                s += a[i];
                break;
            case 2:
                s += b[i];
                break;
            case 3:
                s += c[i % M];
                break;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_arrays(int *a, int *b, int *c, int *d) {
    int i, j, k;
    
    /* Triple nested loop for maximum scheduling complexity */
    for (i = 0; i < 32; ++i) {
        for (j = 0; j < 32; ++j) {
            int sum = 0;
            for (k = 0; k < 32; ++k) {
                /* Complex addressing with multiple dependencies */
                sum += a[i * 32 + k] * b[k * 32 + j];
                
                /* Conditional with volatile */
                if (sum > trigger) {
                    sum = sum % 256;
                }
            }
            c[i * 32 + j] = sum;
            
            /* More arithmetic with branches */
            d[i * 32 + j] = (sum > 128) ? sum * 2 : sum / 2;
        }
        
        /* Function call-like barrier */
        __asm__ volatile("" : : : "memory");
    }
}

int main(void) {
    int *a, *b, *c, *d;
    int i, result1, result2 = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(M * sizeof(int));
    d = (int*)malloc(1024 * sizeof(int));  /* 32x32 */
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);  /* Fixed seed for reproducibility */
    
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    for (i = 0; i < 1024; ++i) {
        d[i] = rand() % 100;
    }
    
    /* Call the complex function to trigger selective scheduling */
    result1 = compute_checksum(a, b, c);
    
    /* Call another complex function */
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent dead code elimination */
    for (i = 0; i < N; ++i) {
        result2 += a[i] + b[i];
    }
    
    for (i = 0; i < M; ++i) {
        result2 += c[i];
    }
    
    for (i = 0; i < 1024; ++i) {
        result2 += d[i];
    }
    
    /* Print result to ensure execution */
    printf("Checksum results: %d, %d\n", result1, result2);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
