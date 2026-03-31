/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Function attributes to prevent optimization and inlining */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional branch creating basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            a[i] = (a[i] * 3) / 2;
        } else if (s < -LIMIT) {
            s = 1;
            /* Different operations in this path */
            b[i] = (b[i] + 7) * 2;
        } else {
            /* Third path with more operations */
            s = s * 2 - 1;
            __asm__ volatile("" : : : "memory");  /* Memory barrier */
        }
        
        /* Switch statement for multiple basic blocks */
        switch (i % 4) {
            case 0:
                s += trigger * 2;
                break;
            case 1:
                s -= trigger / 2;
                break;
            case 2:
                s ^= trigger;
                __asm__ volatile("" : : : "memory");
                break;
            case 3:
                s = (s << 1) | (s >> 31);
                break;
        }
        
        /* Inner loop with array accesses */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Another memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *a, int *b, int *c, int *d) {
    volatile int v1 = 1, v2 = 2;
    int i, j, k;
    
    /* Triple nested loop for more scheduling complexity */
    for (i = 0; i < 64; ++i) {
        for (j = 0; j < 32; ++j) {
            /* Use volatile in index calculation */
            int idx = (i * 32 + j + v1) % N;
            
            for (k = 0; k < 16; ++k) {
                /* Mixed operations with dependencies */
                d[idx] = a[idx] * b[idx] + c[idx] * k;
                
                /* Complex conditional */
                if ((d[idx] & 1) == 0) {
                    d[idx] = d[idx] * 3 + 1;
                } else {
                    d[idx] = d[idx] / 2;
                }
                
                /* Cross-iteration dependency */
                a[idx] = (a[idx] + d[idx]) % 100;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Conditional with side effects */
            if (j % 8 == 0) {
                b[idx] = (b[idx] << v2) | (b[idx] >> (32 - v2));
            }
        }
        
        /* Switch with fall-through cases */
        switch (i % 5) {
            case 0:
                v1++;
                /* Fall through */
            case 1:
                v2--;
                /* Fall through */
            case 2:
                __asm__ volatile("" : : : "memory");
                /* Fall through */
            case 3:
                v1 ^= v2;
                break;
            case 4:
                v2 = v1 * 2;
                break;
        }
    }
}

int main(void) {
    int a[N], b[N], c[M], d[N];
    int i, result1, result2 = 0;
    
    /* Initialize with deterministic pseudo-random values */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        d[i] = 0;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 500;
    }
    
    /* Call functions that should trigger selective scheduling */
    result1 = compute_checksum(a, b, c);
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    for (i = 0; i < N; ++i) {
        result2 += a[i] + b[i] + d[i];
    }
    for (i = 0; i < M; ++i) {
        result2 += c[i];
    }
    
    printf("Result1: %d, Result2: %d\n", result1, result2);
    return 0;
}
