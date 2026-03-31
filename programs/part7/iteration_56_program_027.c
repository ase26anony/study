/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Volatile to prevent elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with register pressure and dependencies */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations with array accesses */
        s += a[i] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional operations in this path */
            a[i] = (a[i] + 1) & 0xFF;
        } else if (s > 500) {
            s = s / 2;
            b[i] = (b[i] - 1) & 0xFF;
        } else {
            s = s * 3;
            /* Memory barrier to prevent reordering */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Complex addressing with volatile influence */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = c[idx] % 1000;
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += 1;
                break;
            case 1:
                s -= 2;
                break;
            case 2:
                s *= 3;
                break;
            case 3:
                s = s / 4;
                break;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inline function with different pattern */
__attribute__((noinline, cold))
static void process_array(int *arr, int size) {
    volatile int v = 0;
    int i, temp;
    
    for (i = 0; i < size; ++i) {
        v = i;
        
        /* Complex dependency chain */
        temp = arr[i];
        arr[i] = (temp * 3 + v) & 0xFF;
        
        if (i % 3 == 0) {
            arr[i] += 5;
            __asm__ volatile("" : : : "memory");
        } else if (i % 3 == 1) {
            arr[i] -= 3;
        } else {
            arr[i] *= 2;
        }
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8); ++k) {
            arr[i] ^= k;
        }
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
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 256;
    }
    
    /* Call the complex computation function */
    result = compute_checksum(a, b, c, N, M);
    
    /* Process arrays with different function */
    process_array(a, N);
    process_array(b, N);
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = result;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
