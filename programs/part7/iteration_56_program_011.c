/* sel_sched_test.c - Test program to trigger selective scheduling RTL dumps */
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
        /* Array accesses with variable indices */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: Reset and continue */
            a[i] = s;
        } else if (s < -LIMIT) {
            s = 1;
            /* Path B: Set to 1 and continue */
            b[i] = s;
        } else {
            /* Path C: Normal processing */
            s = s * 2 - 1;
        }
        
        /* Artificial dependency via inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Inner loop with more arithmetic */
        for (j = 0; j < M; ++j) {
            /* Use volatile variable to prevent optimization */
            int idx = (*trigger + j) % M;
            c[idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 1000) {
                c[idx] = c[idx] % 1000;
            } else if (c[idx] < -1000) {
                c[idx] = -c[idx] % 1000;
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += i;
                break;
            case 1:
                s -= i * 2;
                break;
            case 2:
                s ^= i;
                break;
            case 3:
                s = (s << 1) | (s >> 31);
                break;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inline function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *a, int *b, int *c, volatile int *trigger) {
    int temp = 0;
    
    for (int i = 0; i < N; i += 2) {
        /* Complex addressing calculations */
        int idx1 = (i * 3) % N;
        int idx2 = (i * 5) % N;
        
        /* Mixed operations */
        temp = a[idx1] * b[idx2] - a[idx2] * b[idx1];
        
        /* Conditional with side effects */
        if (temp > 0) {
            for (int k = 0; k < 8; ++k) {
                c[(i + k) % M] += temp >> k;
            }
        } else {
            *trigger = *trigger + 1;
            for (int k = 0; k < 4; ++k) {
                c[(i + k * 2) % M] -= temp << k;
            }
        }
        
        /* More arithmetic with dependencies */
        a[i] = temp ^ *trigger;
        b[i] = temp & *trigger;
    }
}

int main(void) {
    /* Initialize with deterministic pseudo-random values */
    int a[N], b[N], c[M];
    volatile int trigger = 42;  /* Volatile to prevent optimization */
    
    srand(12345);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000 - 500;
        b[i] = rand() % 1000 - 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = 0;
    }
    
    /* Call the complex computation functions */
    int checksum1 = compute_checksum(a, b, c, &trigger);
    process_arrays(a, b, c, &trigger);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_checksum = checksum1;
    for (int i = 0; i < N; ++i) {
        final_checksum += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    final_checksum += trigger;
    
    printf("Result: %d\n", final_checksum);
    return 0;
}
