/* sel-sched-trigger.c
 * Designed to trigger selective scheduling RTL dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test sel-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int my_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
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
        
        /* Artificial dependency via inline assembly */
        __asm__ volatile("" : : : "memory");
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = s % 1000;
            /* More arithmetic operations */
            s = s * 2 - (i & 0xF);
        } else if (s < -500) {
            s = -s / 2;
            /* Additional operation to increase complexity */
            s += (i * 3) % 17;
        } else {
            /* Third path with different operations */
            s = s + (i % 5) * 7;
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s = s ^ 0xAAAA;
                break;
            case 1:
                s = s + (s >> 3);
                break;
            case 2:
                s = s * 3;
                break;
            default:
                s = s - (s << 2);
                break;
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Variable index with volatile */
            int j_idx = (j + volatile_mod) % M;
            c[j_idx] += s * j;
            
            /* More arithmetic to increase register pressure */
            c[j_idx] = c[j_idx] ^ (i * j);
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
            
            /* Conditional in inner loop */
            if ((i * j) % 11 == 0) {
                c[j_idx] = c[j_idx] >> 1;
            } else {
                c[j_idx] = c[j_idx] << 1;
            }
        }
        
        /* Update volatile to prevent loop unrolling */
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
        int idx = (i * 7 + 3) % size;
        
        /* Nested conditionals */
        if (arr[idx] > 0) {
            sum += arr[idx] * i;
            if (sum > 10000) {
                sum = sum % 10000;
                __asm__ volatile("" : : : "memory");
            }
        } else if (arr[idx] < 0) {
            sum -= (-arr[idx]) << (i & 3);
        } else {
            sum += i * i;
        }
        
        /* Switch with more cases */
        switch (i % 6) {
            case 0: sum += 1; break;
            case 1: sum -= 2; break;
            case 2: sum *= 3; break;
            case 3: sum /= 2; break;
            case 4: sum ^= 0xFF; break;
            case 5: sum = ~sum; break;
        }
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int i;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; ++i) {
        a[i] = (int)my_rand() % 1000 - 500;
        b[i] = (int)my_rand() % 1000 - 500;
    }
    
    for (i = 0; i < M; ++i) {
        c[i] = (int)my_rand() % 1000 - 500;
    }
    
    /* Call the complex computation function */
    int result1 = compute_checksum(a, b, c, N, M);
    
    /* Call secondary computation */
    int result2 = secondary_computation(c, M);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = result1 + result2;
    
    /* Use array values to ensure they're not optimized away */
    for (i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    
    printf("Result: %d\n", final_checksum);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
