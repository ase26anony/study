/* test_sel_sched_dump.c
 * Program designed to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and hint as cold path */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int n, int m) {
    volatile int trigger = 0;  /* Prevent optimization */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < n; ++i) {
        /* Artificial dependency via inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = 0;
            trigger = i;  /* Use volatile */
        } else if (s < -500) {
            s = 100;
            trigger = i * 2;
        } else {
            s = s * 2 - a[i];
            trigger = i * 3;
        }
        
        /* Switch statement for more basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger * 2;
                break;
            case 1:
                s -= trigger / 2;
                break;
            case 2:
                s ^= trigger;
                break;
            default:
                s = (s << 1) | (trigger & 1);
                break;
        }
        
        /* Inner loop with array access */
        for (j = 0; j < m; ++j) {
            /* Complex addressing with volatile */
            int idx = (j + trigger) % m;
            c[idx] += s * j;
            
            /* More arithmetic to increase pressure */
            c[idx] = (c[idx] * 3) / 2;
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Another conditional inside outer loop */
        if (i % 7 == 0) {
            s = s ^ b[i];
            for (j = 0; j < 5; ++j) {
                c[j % m] += j * i;
            }
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline))
static void init_arrays(int *a, int *b, int *c, int n, int m) {
    int i;
    volatile int seed = 42;  /* Use volatile for initialization */
    
    for (i = 0; i < n; ++i) {
        a[i] = (i * 37 + seed) % 100;
        b[i] = (i * 73 + seed) % 100;
    }
    
    for (i = 0; i < m; ++i) {
        c[i] = (i * 19 + seed) % 50;
    }
}

int main(void) {
    const int N = 100;  /* Runtime constants > 10 */
    const int M = 50;
    int *a, *b, *c;
    int result;
    
    /* Allocate arrays dynamically to avoid constant propagation */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    init_arrays(a, b, c, N, M);
    
    /* Call the complex function that should trigger selective scheduling */
    result = compute_checksum(a, b, c, N, M);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    final_sum += result;
    
    printf("Result: %d, Checksum: %d\n", result, final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
