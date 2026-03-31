/* sel-sched-test.c
 * Test program to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test sel-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int simple_rand(void) {
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
        int idx = (i + volatile_trigger) % N;
        s += a[idx] * b[i];
        
        /* Conditional with multiple basic blocks */
        if (s > 1000) {
            s = s / 2;
            /* Path A: additional computation */
            a[i] = s % 256;
        } else if (s < -500) {
            s = s * 2;
            /* Path B: different computation */
            b[i] = s % 128;
        } else {
            /* Path C: default path */
            s = s + (i % volatile_mod);
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more computations */
        for (j = 0; j < M; ++j) {
            /* Variable index with volatile */
            int inner_idx = (j + volatile_trigger) % M;
            c[inner_idx] += s * j;
            
            /* Another conditional inside inner loop */
            if ((i + j) % 3 == 0) {
                c[inner_idx] -= a[i];
            } else if ((i + j) % 3 == 1) {
                c[inner_idx] += b[j % N];
            } else {
                c[inner_idx] *= 2;
            }
            
            /* Prevent loop invariant code motion */
            __asm__ volatile("" : : : "memory");
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
                s /= 4;
                break;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_arrays(int *arr1, int *arr2, int size) {
    int temp = 0;
    volatile int vol_idx = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing mode */
        int idx1 = (i * 3) % size;
        int idx2 = (i * 5) % size;
        
        /* Multiple operations to increase register pressure */
        arr1[idx1] = arr1[idx1] + arr2[idx2] * 3;
        arr2[idx2] = arr2[idx2] - arr1[idx1] / 2;
        temp = arr1[idx1] ^ arr2[idx2];
        
        /* Use volatile in index */
        arr1[vol_idx % size] = temp;
        vol_idx++;
        
        /* Conditional with side effects */
        if (temp > 10000) {
            arr2[i] = 0;
            __asm__ volatile("" : : : "memory");
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ARR_SIZE = 200;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *arr1 = (int *)malloc(ARR_SIZE * sizeof(int));
    int *arr2 = (int *)malloc(ARR_SIZE * sizeof(int));
    
    if (!a || !b || !c || !arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        a[i] = simple_rand() % 1000;
        b[i] = simple_rand() % 1000;
    }
    
    for (int i = 0; i < M; i++) {
        c[i] = simple_rand() % 500;
    }
    
    for (int i = 0; i < ARR_SIZE; i++) {
        arr1[i] = simple_rand() % 2000;
        arr2[i] = simple_rand() % 2000;
    }
    
    /* Call functions to trigger selective scheduling */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_arrays(arr1, arr2, ARR_SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = checksum1;
    for (int i = 0; i < N; i++) {
        final_checksum += a[i] + b[i];
    }
    for (int i = 0; i < M; i++) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ARR_SIZE; i++) {
        final_checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    
    return 0;
}
