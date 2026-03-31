/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int n, int m) {
    volatile int trigger = 0;
    int s = 0;
    int i, j;
    
    /* Complex loop nest with register pressure and dependencies */
    for (i = 0; i < n; ++i) {
        /* Multiple arithmetic operations creating register pressure */
        int idx = (i + trigger) % n;
        s += a[idx] * b[idx];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* More operations in this path */
            a[idx] = (a[idx] * 3) / 2;
        } else if (s < -500) {
            s = 100;
            b[idx] = (b[idx] + 7) * 2;
        } else {
            /* Third path with different operations */
            s = s * 2 - 1;
            a[idx] = a[idx] ^ b[idx];
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += i * 2;
                break;
            case 1:
                s -= i * 3;
                break;
            case 2:
                s = s ^ i;
                break;
            default:
                s = s * i;
                break;
        }
        
        /* Memory barrier to prevent optimization */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with array accesses */
        for (j = 0; j < m; ++j) {
            /* Complex addressing with volatile */
            int jdx = (j + trigger) % m;
            c[jdx] += s * j;
            
            /* More conditional logic */
            if (c[jdx] > 10000) {
                c[jdx] = c[jdx] % 1000;
            }
            
            /* Additional arithmetic */
            c[jdx] = (c[jdx] << 1) | (c[jdx] >> 31);
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Update volatile to prevent dead code elimination */
        trigger = (trigger + 1) & 0xFF;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static int process_arrays(int *arr1, int *arr2, int size) {
    volatile int seed = 42;
    int sum = 0;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int val1 = arr1[i];
        int val2 = arr2[i];
        
        /* Multiple arithmetic operations */
        int prod = val1 * val2;
        int diff = val1 - val2;
        int sum_val = val1 + val2;
        
        /* Nested conditionals */
        if (prod > diff) {
            if (sum_val > 0) {
                arr1[i] = prod >> 2;
            } else {
                arr1[i] = diff << 1;
            }
        } else {
            arr1[i] = sum_val * 3;
        }
        
        /* More operations with volatile */
        sum += arr1[i] + (seed % 17);
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Additional loop with variable bound */
        int k;
        for (k = 0; k < (i % 8) + 1; ++k) {
            arr2[i] ^= k * seed;
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int SIZE = 200;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c || !arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(12345);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = rand() % 1000;
    }
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    /* Call functions that should trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, N, M);
    int result2 = process_arrays(arr1, arr2, SIZE);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = result1 + result2;
    for (int i = 0; i < N; i++) final_sum += a[i];
    for (int i = 0; i < M; i++) final_sum += c[i];
    
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    
    return 0;
}
