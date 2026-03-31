/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_limit = 1000;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    volatile int local_volatile = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        int idx = (i * 7 + 3) % N;
        s += a[idx] * b[i];
        
        /* Artificial dependency via inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Conditional with multiple basic blocks */
        if (s > g_volatile_limit) {
            /* Path A: Reset and do more computation */
            s = 0;
            local_volatile += i;
            for (int k = 0; k < 5; ++k) {
                s += k * a[(i + k) % N];
            }
        } else if (s < -g_volatile_limit) {
            /* Path B: Different computation pattern */
            s = s / 2;
            local_volatile -= i;
            for (int k = 0; k < 3; ++k) {
                s -= b[(i + k) % N] * k;
            }
        } else {
            /* Path C: Yet another computation pattern */
            s = s * 2 - 1;
            local_volatile ^= i;
            
            /* Switch statement for additional basic blocks */
            switch (i % 4) {
                case 0:
                    s += a[i] * 3;
                    break;
                case 1:
                    s -= b[i] * 2;
                    break;
                case 2:
                    s *= 2;
                    break;
                default:
                    s = s >> 1;
                    break;
            }
        }
        
        /* Inner loop with more computations */
        for (int j = 0; j < M; ++j) {
            /* Variable array index with volatile component */
            int c_idx = (j + local_volatile) % M;
            c[c_idx] += s * j;
            
            /* Another memory barrier to prevent optimization */
            __asm__ volatile("" : : : "memory");
            
            /* More conditional logic */
            if (c[c_idx] > 10000) {
                c[c_idx] = c[c_idx] % 1000;
            }
        }
        
        /* Update volatile counter */
        g_volatile_counter++;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int secondary_computation(int *arr, int size) {
    int result = 0;
    volatile int v = 1;
    
    for (int i = 0; i < size; ++i) {
        /* Complex index calculation */
        int idx = (i * 13 + 7) % size;
        
        /* Nested if-else chain */
        if (arr[idx] > 0) {
            result += arr[idx] * i;
            v = v * 2;
        } else if (arr[idx] < 0) {
            result -= arr[idx] * (size - i);
            v = v / 2;
        } else {
            result ^= i;
            v = v + 1;
        }
        
        /* Small inner loop */
        for (int j = 0; j < 3; ++j) {
            result += (i * j) & 0xFF;
        }
        
        __asm__ volatile("" : : : "memory");
    }
    
    return result;
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
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000 - 500;
        b[i] = rand() % 1000 - 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = 0;
    }
    
    for (int i = 0; i < ARR_SIZE; ++i) {
        arr[i] = rand() % 2000 - 1000;
    }
    
    /* Call the complex computation functions */
    int checksum1 = compute_checksum(a, b, c, N, M);
    int checksum2 = secondary_computation(arr, ARR_SIZE);
    
    /* Compute final result to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < M; ++i) {
        final_result += c[i];
    }
    final_result += checksum1 + checksum2;
    
    /* Use volatile variable to ensure side effects */
    final_result += g_volatile_counter;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
