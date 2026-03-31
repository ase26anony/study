/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Volatile to prevent elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional operations in this path */
            a[i] = b[i] + trigger;
        } else if (s > 500) {
            s = s / 2;
            /* Different operations in else-if */
            b[i] = a[i] - trigger;
        } else {
            /* Third path with more operations */
            s = s * 3;
            a[i] = trigger * 2;
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
                s *= 2;
                break;
            default:
                s = s ^ trigger;  /* XOR operation */
                break;
        }
        
        /* Inner loop with memory accesses */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = 0;
            }
        }
        
        /* Inline assembly barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inline function with different pattern */
__attribute__((noinline, cold))
static void process_array(int *arr, int size) {
    volatile int v = 0;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int t = arr[i];
        arr[i] = (t << 3) | (t >> 5);  /* Rotate left 3 */
        
        /* Nested if-else chain */
        if (i % 7 == 0) {
            arr[i] += v;
            v = arr[i] % 256;
        } else if (i % 7 == 1) {
            arr[i] -= v;
            v = arr[i] % 128;
        } else if (i % 7 == 2) {
            arr[i] *= v + 1;
            v = arr[i] % 64;
        } else {
            arr[i] ^= v;
            v = arr[i] % 32;
        }
        
        /* More arithmetic */
        arr[i] = (arr[i] * 1103515245 + 12345) & 0x7fffffff;
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int i, result;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    
    /* Call the complex function to trigger selective scheduling */
    result = compute_checksum(a, b, c, N, M);
    
    /* Process arrays further to increase scheduling opportunities */
    process_array(a, N);
    process_array(b, N);
    process_array(c, M);
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = result;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
