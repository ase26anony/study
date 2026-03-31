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
            s = 0;
            /* Path A: more arithmetic */
            a[i] = s * i;
        } else if (s > 500) {
            s = s / 2;
            /* Path B: different operations */
            b[i] = s + i;
        } else {
            s = s * 3;
            /* Path C: yet another path */
            c[i % M] = s - i;
        }
        
        /* Inner loop with artificial dependencies */
        for (j = 0; j < M; ++j) {
            /* Inline assembly to create dependencies and prevent optimization */
            __asm__ volatile("" : : : "memory");
            
            /* Complex arithmetic with multiple operations */
            int temp = c[j] * j + s;
            c[j] = temp % 100;
            
            /* Another conditional inside inner loop */
            if ((i + j) % 7 == 0) {
                c[j] += a[i % N];
            } else if ((i + j) % 7 == 1) {
                c[j] -= b[j % N];
            } else {
                c[j] *= 2;
            }
            
            /* Use volatile variable in computation */
            c[j] += g_volatile_trigger;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += a[i % N] * 2;
                break;
            case 1:
                s -= b[i % N] / 2;
                break;
            case 2:
                s = s ^ a[i % N];
                break;
            case 3:
                s = s | b[i % N];
                break;
        }
        
        /* Update volatile to create side effects */
        g_volatile_counter++;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline))
void process_arrays(int *arr1, int *arr2, int size) {
    int i;
    volatile int local_volatile = 0;
    
    for (i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int x = arr1[i];
        int y = arr2[i];
        
        /* Multiple arithmetic operations in sequence */
        x = x * y + i;
        y = y * x - i;
        x = x ^ y;
        y = y | x;
        
        /* Conditional with both paths having work */
        if (x > y) {
            arr1[i] = x % 100;
            arr2[i] = y % 50;
        } else {
            arr1[i] = y % 75;
            arr2[i] = x % 25;
        }
        
        /* Memory barrier */
        __asm__ volatile("" : : : "memory");
        
        /* Use volatile */
        local_volatile += i;
    }
    
    g_volatile_trigger = local_volatile % 100;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int i, result;
    
    /* Allocate and initialize arrays with pseudo-random values */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    
    /* Simple deterministic initialization */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 13 + 7) % 100;
        b[i] = (i * 17 + 11) % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = (i * 19 + 13) % 100;
    }
    
    /* Call the complex computation function */
    result = compute_checksum(a, b, c, N, M);
    
    /* Process arrays with another pattern */
    process_arrays(a, b, N);
    
    /* Compute final checksum to prevent optimization */
    int checksum = 0;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    checksum += result;
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
