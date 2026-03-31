/* sel-sched-test.c
 * Test program to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test sel-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Volatile variables to prevent optimization */
volatile int volatile_trigger = 0;
volatile int volatile_mod = 7;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int i, j;
    
    /* Outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        int idx = (i * 17 + volatile_trigger) % N;
        s += a[idx] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000000) {
            s = s / 2;
            /* Path A: Additional computation */
            a[i] = s % 256;
        } else if (s < -1000000) {
            s = 0;
            /* Path B: Different computation */
            b[i] = (s + i) % 128;
        } else {
            /* Path C: Default path */
            s = s * 3 - 1;
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with dependencies */
        for (j = 0; j < M; ++j) {
            /* Variable index using volatile */
            int inner_idx = (j + volatile_mod) % M;
            c[inner_idx] += s * j;
            
            /* More complex arithmetic */
            if (j % 4 == 0) {
                c[inner_idx] -= a[i];
            } else if (j % 4 == 1) {
                c[inner_idx] += b[i] * 2;
            } else {
                c[inner_idx] = c[inner_idx] * 3 / 2;
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Switch-like construct with multiple cases */
        switch (i % 5) {
            case 0:
                s += a[i] >> 1;
                break;
            case 1:
                s -= b[i] << 1;
                break;
            case 2:
                s ^= a[i] & b[i];
                break;
            case 3:
                s = (s * 13) % 997;
                break;
            default:
                s = ~s;
                break;
        }
        
        /* Update volatile to create side effects */
        volatile_trigger = (volatile_trigger + 1) % 13;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int secondary_computation(int *arr, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Complex index calculation */
        int idx1 = (i * 3) % size;
        int idx2 = (i * 7) % size;
        int idx3 = (i * 11) % size;
        
        /* Multiple dependent operations */
        int temp = arr[idx1] * arr[idx2];
        temp += arr[idx3] << 2;
        
        /* Conditional with nested ternary */
        sum += (temp > 0) ? temp : -temp;
        
        /* More arithmetic */
        arr[idx1] = (arr[idx1] + i) % 1000;
        arr[idx2] = (arr[idx2] - i) % 1000;
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Small inner loop */
        for (int k = 0; k < 3; ++k) {
            sum += (arr[(i + k) % size] * k);
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ARR_SIZE = 200;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *arr = (int *)malloc(ARR_SIZE * sizeof(int));
    
    if (!a || !b || !c || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = simple_rand() % 1000;
        b[i] = simple_rand() % 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = simple_rand() % 1000;
    }
    
    for (int i = 0; i < ARR_SIZE; ++i) {
        arr[i] = simple_rand() % 1000;
    }
    
    /* Call the complex computation functions */
    int result1 = compute_checksum(a, b, c, N, M);
    int result2 = secondary_computation(arr, ARR_SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < N; ++i) {
        final_checksum += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ARR_SIZE; ++i) {
        final_checksum += arr[i];
    }
    
    final_checksum += result1 + result2;
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
