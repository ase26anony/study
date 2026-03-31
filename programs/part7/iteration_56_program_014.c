/* test_sel_sched_dump.c
 * Program designed to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp -o test test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_trigger = 1;

/* Simple deterministic pseudo-random generator */
static unsigned int simple_rand(unsigned int *seed) {
    *seed = *seed * 1103515245 + 12345;
    return (*seed >> 16) & 0x7FFF;
}

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int i, j;
    unsigned int rand_state = 42;
    
    /* Outer loop with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Complex arithmetic with array accesses */
        int idx = (i * g_trigger) % N;
        s += a[idx] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional arithmetic in this path */
            a[i] = simple_rand(&rand_state) % 100;
        } else if (s < -500) {
            s = s / 2;
            /* Different operations in this path */
            b[i] = simple_rand(&rand_state) % 50;
        } else {
            /* Third path with its own operations */
            s = s * 3 / 2;
            __asm__ volatile("" : : : "memory");  /* Memory barrier */
        }
        
        /* Switch statement for more basic blocks */
        switch (i % 4) {
            case 0:
                s += i * 2;
                break;
            case 1:
                s -= i * 3;
                /* Inline assembly to create dependencies */
                __asm__ volatile("" : : : "memory");
                break;
            case 2:
                s = s ^ i;
                break;
            default:
                s = s | (i << 2);
                break;
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < M; ++j) {
            /* Complex addressing mode */
            int inner_idx = (i + j) % M;
            c[inner_idx] += s * j;
            
            /* More conditionals inside inner loop */
            if (c[inner_idx] > 10000) {
                c[inner_idx] = c[inner_idx] % 1000;
                g_volatile_counter++;
            }
            
            /* Additional arithmetic to increase register pressure */
            c[inner_idx] = c[inner_idx] + (a[i] * b[j % N]) / (j + 1);
        }
        
        /* Volatile access to prevent dead code elimination */
        if (g_trigger) {
            s += g_volatile_counter;
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
int process_arrays(int *arr1, int *arr2, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Data-dependent array access */
        int idx1 = (i * 7) % size;
        int idx2 = (i * 13) % size;
        
        /* Complex expression with multiple operations */
        int temp = arr1[idx1] * arr2[idx2];
        temp = temp + (arr1[idx2] << 2);
        temp = temp - (arr2[idx1] >> 1);
        
        /* Conditional with both paths having operations */
        if (temp % 3 == 0) {
            sum += temp * 2;
            __asm__ volatile("" : : : "memory");
        } else if (temp % 3 == 1) {
            sum += temp / 2;
            arr1[i] = temp;
        } else {
            sum += temp;
            arr2[i] = sum;
        }
        
        /* Loop-carried dependency */
        arr1[(i + 1) % size] += sum % 100;
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int i;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 123456789;
    for (i = 0; i < N; ++i) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
    }
    for (i = 0; i < M; ++i) {
        c[i] = simple_rand(&seed) % 500;
    }
    
    /* Call the complex function to trigger selective scheduling */
    int result1 = compute_checksum(a, b, c, N, M);
    
    /* Process arrays with different pattern */
    int result2 = process_arrays(a, b, N);
    
    /* Final computation to ensure no dead code */
    int final_result = 0;
    for (i = 0; i < N; ++i) {
        final_result += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        final_result += c[i];
    }
    
    final_result = final_result + result1 + result2;
    
    printf("Result: %d\n", final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
