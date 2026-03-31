/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i % 4;
        
        /* Multiple arithmetic operations with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            a[i] = (a[i] * 3) / 2;
        } else if (s < -LIMIT) {
            s = 1;
            /* Different operations in this path */
            b[i] = (b[i] + 7) * 2;
        } else {
            /* Third path with its own operations */
            s = s * 2 - s / 3;
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with variable bounds */
        int inner_limit = M - (i % 32);
        for (j = 0; j < inner_limit; ++j) {
            /* Complex addressing with volatile influence */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditional logic inside inner loop */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
            }
            
            /* Switch statement for additional basic blocks */
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
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Additional arithmetic to increase register pressure */
        a[i] = a[i] + b[i] - s;
        b[i] = b[i] * 2 - a[i];
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *a, int *b, int *c, int *d) {
    volatile int v = 1;
    int i, j, k;
    
    /* Triple nested loop for maximum scheduling complexity */
    for (i = 0; i < 64; ++i) {
        for (j = 0; j < 32; ++j) {
            /* Use volatile in index calculation */
            int idx = (i * 32 + j + v) % N;
            
            for (k = 0; k < 16; ++k) {
                /* Mixed operations with dependencies */
                d[idx] = a[idx] * b[idx] + c[idx] * k;
                
                /* Complex conditional with multiple branches */
                if (d[idx] & 1) {
                    d[idx] = d[idx] ^ 0xAAAA;
                } else {
                    d[idx] = d[idx] ^ 0x5555;
                }
                
                /* Artificial dependency chain */
                a[idx] = (a[idx] + d[idx]) >> 1;
                b[idx] = (b[idx] - d[idx]) << 1;
                c[idx] = c[idx] ^ d[idx];
            }
            
            /* Memory barrier between inner loops */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Update volatile to prevent loop elimination */
        v = (v * 1103515245 + 12345) & 0x7fffffff;
    }
}

int main(void) {
    int *a, *b, *c, *d;
    int i, result1, result2 = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(M * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);  /* Fixed seed for reproducibility */
    
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        d[i] = 0;
    }
    
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    
    /* Call functions designed to trigger selective scheduling */
    result1 = compute_checksum(a, b, c);
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    for (i = 0; i < N; ++i) {
        result2 += a[i] + b[i] + d[i];
    }
    
    for (i = 0; i < M; ++i) {
        result2 += c[i];
    }
    
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
