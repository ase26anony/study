/* test_sel_sched_dump.c
 * Program designed to trigger selective scheduling RTL dumps in GCC
 * Specifically targets sel_print_insn_rtl function in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile int guard = 1;

/* Function with complex loop nest to trigger selective scheduling */
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
            /* Path A: Reset path */
            a[i] = (a[i] + 1) & 0xFF;
        } else if (s > (LIMIT / 2)) {
            /* Path B: Intermediate path */
            s = s / 2;
            b[i] = (b[i] - 1) & 0xFF;
        } else {
            /* Path C: Normal path */
            s = s * 3 + 1;
            /* Use volatile in computation */
            s += trigger * guard;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s ^= 0xAAAAAAAA;
                break;
            case 1:
                s ^= 0x55555555;
                break;
            case 2:
                s ^= 0x33333333;
                break;
            default:
                s ^= 0xCCCCCCCC;
                break;
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more complex dependencies */
        for (j = 0; j < M; ++j) {
            /* Variable array index using volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditional logic */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
                /* Another memory barrier */
                __asm__ volatile("" : : : "memory");
            }
            
            /* Additional arithmetic to increase register pressure */
            c[idx] = (c[idx] * 13 + 17) & 0xFFFF;
        }
        
        /* Use volatile in loop condition variation */
        if (guard) {
            s += i * trigger;
        }
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
                /* Complex addressing pattern */
                int idx = (i * 32 + j + k) % N;
                sum += a[idx] * b[(j * 32 + k) % N];
                
                /* Conditional with side effect */
                if (sum & 1) {
                    c[k] ^= d[j];
                    /* Memory barrier */
                    __asm__ volatile("" : : : "memory");
                } else {
                    d[j] ^= c[k];
                }
            }
            
            /* Store with volatile dependency */
            a[i] = sum + trigger;
        }
        
        /* Branch with unpredictable pattern */
        if (i % 7 == 0) {
            trigger = (trigger + 1) & 0xF;
        }
    }
}

int main(void) {
    int i;
    int result;
    
    /* Allocate and initialize arrays with pseudo-random values */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *d = (int *)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Simple deterministic initialization */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    /* Call functions designed to trigger selective scheduling */
    result = compute_checksum(a, b, c);
    printf("Checksum 1: %d\n", result);
    
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (i = 0; i < N; ++i) {
        final_sum += a[i] + b[i] + d[i];
    }
    for (i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
