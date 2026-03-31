/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define LIMIT 1000000

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, volatile int *trigger) {
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        int idx = i + *trigger;
        
        /* Multiple arithmetic operations */
        s += a[idx % N] * b[idx % N];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: more operations */
            for (j = 0; j < M/2; ++j) {
                c[j] += s * j;
                /* Inline assembly barrier to prevent optimization */
                __asm__ volatile("" : : : "memory");
            }
        } else if (s > LIMIT/2) {
            /* Path B: different operations */
            s = s / 2;
            for (j = M/2; j < M; ++j) {
                c[j] += s * (j - M/2);
                __asm__ volatile("" : : : "memory");
            }
        } else {
            /* Path C: yet another set of operations */
            s = s * 2;
            for (j = 0; j < M; j += 2) {
                c[j] += s * (j + 1);
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                a[i] = b[i] + 1;
                break;
            case 1:
                a[i] = b[i] - 1;
                break;
            case 2:
                a[i] = b[i] * 2;
                break;
            default:
                a[i] = b[i] / 2;
                break;
        }
        
        /* More arithmetic with variable indices */
        int k = (i * 7) % N;
        b[k] = (b[k] + a[i]) % 1000;
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *a, int *b, int *c, volatile int *trigger) {
    int temp = 0;
    
    /* Nested loops with complex dependencies */
    for (int i = 0; i < N/2; ++i) {
        /* Volatile access affects scheduling */
        int offset = *trigger;
        
        for (int j = 0; j < M; ++j) {
            /* Complex addressing calculations */
            int idx = (i * j + offset) % N;
            
            /* Multiple operations in one iteration */
            temp = a[idx] * 3 + b[idx] * 5 - c[j] * 2;
            
            /* Conditional store */
            if (temp > 0) {
                c[j] = temp % 100;
            } else {
                c[j] = (-temp) % 100;
            }
            
            /* Cross-iteration dependency */
            a[idx] = b[idx] + temp;
            b[idx] = c[j] - temp;
            
            /* Prevent loop unrolling */
            if (j % 8 == 0) {
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Outer loop dependency */
        *trigger = (*trigger + i) % 100;
    }
}

int main(void) {
    /* Initialize with deterministic pseudo-random values */
    int a[N], b[N], c[M];
    volatile int trigger = 42;  /* Volatile to prevent optimization */
    
    srand(12345);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    
    /* Call functions designed to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, &trigger);
    process_arrays(a, b, c, &trigger);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    final_sum += result1 + trigger;
    
    printf("Result: %d\n", final_sum);
    return 0;
}
