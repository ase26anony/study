/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 10000

/* Non-inlineable function with complex loop structure */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c) {
    volatile int trigger = 0;
    int s = 0;
    int i, j;
    
    /* Outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            trigger = i;  /* Use volatile to prevent optimization */
        } else if (s < -LIMIT) {
            s = LIMIT;
            trigger = -i;
        } else {
            /* Switch statement for additional basic blocks */
            switch (i % 4) {
                case 0:
                    s += i;
                    break;
                case 1:
                    s -= i * 2;
                    break;
                case 2:
                    s *= 3;
                    break;
                case 3:
                    s /= 2;
                    break;
            }
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more complex operations */
        for (j = 0; j < M; ++j) {
            /* Variable index using volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditional logic */
            if (c[idx] > 1000000) {
                c[idx] = 0;
            }
            
            /* Additional arithmetic to increase register pressure */
            c[idx] = (c[idx] * 13 + 7) % 1000;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline))
static void process_array(int *arr, int size) {
    volatile int seed = 42;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Complex dependency chain */
        arr[i] = (arr[i] * 3 + seed) % 100;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Nested conditionals */
        if (i % 5 == 0) {
            arr[i] += i;
        } else if (i % 5 == 1) {
            arr[i] -= i * 2;
        } else if (i % 5 == 2) {
            arr[i] *= 3;
        } else if (i % 5 == 3) {
            arr[i] /= 2;
        } else {
            arr[i] ^= 0xFF;
        }
        
        /* Prevent loop unrolling */
        if (i % 8 == 0) {
            __asm__ volatile("" : : : "memory");
        }
    }
}

int main(void) {
    int a[N], b[N], c[M];
    int i, result;
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    /* Process arrays to create more scheduling opportunities */
    process_array(a, N);
    process_array(b, N);
    
    /* Main computation with complex loop nest */
    result = compute_checksum(a, b, c);
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = result;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
