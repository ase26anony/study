/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Prevent dead code elimination */
    int s = 0;
    int result = 0;
    
    /* Complex loop nest with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        trigger = i;
        
        /* Multiple arithmetic operations with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            /* Path A: Reset and do more computation */
            s = 0;
            for (int j = 0; j < M; ++j) {
                c[j] += s * j;
                /* Memory barrier to prevent reordering */
                __asm__ volatile("" : : : "memory");
            }
        } else if (s > 500) {
            /* Path B: Different computation pattern */
            s = s / 2;
            for (int j = 0; j < M/2; ++j) {
                c[j] += s * (j + 1);
                __asm__ volatile("" : : : "memory");
            }
        } else {
            /* Path C: Yet another pattern */
            s = s * 2;
            for (int j = M/2; j < M; ++j) {
                c[j] += s * (j - 1);
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += a[i] % 17;
                break;
            case 1:
                s -= b[i] % 13;
                break;
            case 2:
                s ^= (a[i] ^ b[i]);
                break;
            case 3:
                s = (s << 3) | (s >> 29);  /* Rotate */
                break;
        }
        
        /* More arithmetic with variable indices */
        int idx = (s + i) % N;
        result += a[idx] + b[idx] + c[i % M];
        
        /* Prevent loop unrolling */
        __asm__ volatile("" : : : "memory");
    }
    
    return result;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static int secondary_computation(int *arr, int size) {
    volatile int guard = 0;
    int sum = 0;
    
    /* Loop with data-dependent branches */
    for (int i = 1; i < size; i++) {
        guard = arr[i];
        
        /* Complex conditional chain */
        if (arr[i] > arr[i-1]) {
            sum += arr[i] * 3;
        } else if (arr[i] < arr[i-1]) {
            sum -= arr[i] * 2;
        } else {
            sum ^= arr[i];
        }
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8); k++) {
            sum += (arr[i] << k);
            __asm__ volatile("" : : : "memory");
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    
    /* Simple deterministic initialization */
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = rand() % 1000;
    }
    
    /* Call the computation function */
    int checksum1 = compute_checksum(a, b, c, N, M);
    
    /* Secondary computation for more scheduling opportunities */
    int checksum2 = secondary_computation(a, N);
    
    /* Final result to prevent optimization */
    int final_result = checksum1 + checksum2;
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
