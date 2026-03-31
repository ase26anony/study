/* sel_sched_dump_test.c
 * Test program to trigger selective scheduling RTL dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp sel_sched_dump_test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int simple_rand(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Function with complex loop nest to trigger selective scheduling */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M) {
    volatile int trigger = 0;  /* Prevent optimization */
    int s = 0;
    int result = 0;
    
    /* Outer loop with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional computation in this path */
            for (int k = 0; k < 5; ++k) {
                __asm__ volatile("" : : : "memory");  /* Memory barrier */
                s += k * trigger;
            }
        } else if (s < -500) {
            s = 500;
            /* Different computation path */
            for (int k = 0; k < 3; ++k) {
                __asm__ volatile("" : : : "memory");
                s -= k * (trigger + 1);
            }
        } else {
            /* Default path with switch statement */
            switch (i % 4) {
                case 0:
                    s += i * 2;
                    break;
                case 1:
                    s -= i;
                    break;
                case 2:
                    s *= 2;
                    break;
                case 3:
                    s /= 2;
                    break;
            }
        }
        
        /* Inner loop with more complex dependencies */
        for (int j = 0; j < M; ++j) {
            /* Variable array index with volatile influence */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* More arithmetic operations */
            c[idx] -= (i * j) / 3;
            c[idx] += (s % 7) * 2;
            
            /* Memory barrier to prevent reordering */
            __asm__ volatile("" : : : "memory");
            
            /* Nested conditional */
            if (c[idx] > 10000) {
                c[idx] = 10000;
            } else if (c[idx] < -10000) {
                c[idx] = -10000;
            }
        }
        
        /* Update volatile to prevent dead code elimination */
        trigger = (trigger + i) % 17;
        
        /* Accumulate result */
        result += s;
    }
    
    return result;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline, cold))
static void process_arrays(int *arr1, int *arr2, int size) {
    volatile int control = 1;
    
    for (int i = 0; i < size; ++i) {
        /* Complex dependency chain */
        int val = arr1[i];
        
        /* Loop with variable bounds */
        for (int k = 0; k < (i % 8) + 1; ++k) {
            val = (val * 3 + 7) % 1024;
            __asm__ volatile("" : : : "memory");
        }
        
        /* Conditional store */
        if (val > 512) {
            arr2[i] = val - 512;
        } else {
            arr2[i] = val + arr1[(i + control) % size];
        }
        
        control = (control * 3) % 19;
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
    
    /* Initialize with deterministic pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < N; ++i) {
        a[i] = (int)(simple_rand(&seed) % 1000) - 500;
        b[i] = (int)(simple_rand(&seed) % 1000) - 500;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = (int)(simple_rand(&seed) % 100) - 50;
    }
    
    for (int i = 0; i < ARR_SIZE; ++i) {
        arr1[i] = (int)(simple_rand(&seed) % 2000) - 1000;
        arr2[i] = 0;
    }
    
    /* Call functions with complex loop nests */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_arrays(arr1, arr2, ARR_SIZE);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = checksum1;
    for (int i = 0; i < M; ++i) {
        final_checksum += c[i];
    }
    for (int i = 0; i < ARR_SIZE; ++i) {
        final_checksum += arr2[i];
    }
    
    printf("Result: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    
    return 0;
}
