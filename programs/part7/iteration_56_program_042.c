/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
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
        /* Multiple arithmetic operations with dependencies */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            trigger = i;  /* Use volatile variable */
        } else if (s < -LIMIT) {
            s = 0;
            trigger = -i;
        } else {
            /* Switch statement for additional basic blocks */
            switch (i % 4) {
                case 0:
                    s += i * 2;
                    break;
                case 1:
                    s -= i * 3;
                    break;
                case 2:
                    s ^= i;
                    break;
                default:
                    s = (s << 1) | (s >> 31);
                    break;
            }
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with array accesses */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile variable */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More arithmetic operations */
            if (j % 8 == 0) {
                c[idx] ^= 0x5555;
            } else if (j % 8 == 4) {
                c[idx] &= 0xAAAA;
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional conditional with volatile */
        if (trigger % 7 == 0) {
            s = (s * 3) / 2;
        }
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
        int temp = 0;
        
        for (j = 0; j < 32; ++j) {
            /* Variable array indices with volatile */
            int idx1 = (i + v1) % 64;
            int idx2 = (j + v2) % 32;
            
            temp += a[idx1] * b[idx2];
            
            /* Complex conditional chain */
            if (temp > 1000) {
                for (k = 0; k < 16; ++k) {
                    d[k] += temp * k;
                    if (d[k] > 5000) d[k] = 0;
                }
                temp = 0;
            } else if (temp < -1000) {
                temp = -temp;
            }
            
            __asm__ volatile("" : : : "memory");
        }
        
        c[i] = temp;
        v1 = (v1 * 3) % 17;  /* Update volatile */
    }
}

int main(void) {
    int *a, *b, *c, *d;
    int i, result1, result2 = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(64 * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = 0;
    }
    for (i = 0; i < 64; ++i) {
        d[i] = 0;
    }
    
    /* Call functions designed to trigger selective scheduling */
    result1 = compute_checksum(a, b, c);
    process_arrays(a, b, c, d);
    
    /* Compute final checksum to prevent optimization */
    for (i = 0; i < N; ++i) {
        result2 += c[i];
    }
    for (i = 0; i < 64; ++i) {
        result2 += d[i];
    }
    
    printf("Result1: %d, Result2: %d, Total: %d\n", 
           result1, result2, result1 + result2);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
