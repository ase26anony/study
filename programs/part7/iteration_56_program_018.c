/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int i, j;
    
    /* Complex loop with register pressure and dependencies */
    for (i = 0; i < N; ++i) {
        /* Array accesses with variable indices */
        int idx = (i + g_volatile_counter) % N;
        s += a[idx] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            /* Path A: Reset and do some computation */
            s = s % 100;
            c[i % M] += s * i;
            /* Inline assembly to create artificial dependencies */
            __asm__ volatile("" : : : "memory");
        } else if (s > 500) {
            /* Path B: Different computation */
            s = s * 2;
            c[(i + 1) % M] -= s;
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        } else {
            /* Path C: Default path */
            s = s + i;
            c[i % M] = s ^ b[i];
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Complex arithmetic with multiple dependencies */
            int temp = c[j] * j + s;
            c[j] = temp - (temp / (j + 1)) * (j + 1);
            
            /* Switch statement for additional basic blocks */
            switch (j % 4) {
                case 0:
                    c[j] += a[i % N];
                    break;
                case 1:
                    c[j] -= b[i % N];
                    break;
                case 2:
                    c[j] *= 2;
                    break;
                default:
                    c[j] ^= 0x55;
                    break;
            }
            
            /* Use volatile variable in computation */
            if (g_volatile_trigger) {
                c[j] += g_volatile_counter;
            }
        }
        
        /* More arithmetic to increase register pressure */
        s = (s * 13 + 17) % 1000;
        a[i] = (a[i] + s) & 0xFF;
        b[i] = (b[i] - s) & 0xFF;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int secondary_computation(int *arr, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Nested conditionals */
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                arr[i] = arr[i] * 3 + 1;
            } else {
                arr[i] = arr[i] / 2;
            }
            __asm__ volatile("" : : : "memory");
        } else {
            arr[i] = arr[i] ^ (arr[i] >> 1);
        }
        
        /* Complex expression with multiple operations */
        sum += arr[i] * i - (arr[i] % (i + 1));
        
        /* Volatile access */
        if (g_volatile_trigger && (i % 10 == 0)) {
            sum += g_volatile_counter;
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int i;
    
    /* Initialize arrays with pseudo-random values */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    
    /* Simple deterministic RNG */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    
    /* Call the complex computation function */
    int result1 = compute_checksum(a, b, c, N, M);
    
    /* Call secondary computation */
    int result2 = secondary_computation(a, N);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = result1 + result2;
    for (i = 0; i < N; ++i) {
        final_checksum += a[i];
    }
    for (i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
