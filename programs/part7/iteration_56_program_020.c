/* sel-sched-test.c
 * Test program to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -dA -dp sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int N, int M, volatile int *trigger) {
    int s = 0;
    int result = 0;
    
    /* Complex loop nest with high register pressure */
    for (int i = 0; i < N; ++i) {
        /* Artificial dependency through volatile */
        int idx = *trigger + i;
        
        /* Multiple arithmetic operations */
        s += a[idx % N] * b[idx % N];
        
        /* Conditional branch creating basic blocks */
        if (s > 1000) {
            s = 0;
            /* Inline assembly barrier to prevent optimization */
            __asm__ volatile("" : : : "memory");
        } else if (s > 500) {
            s = s / 2;
            __asm__ volatile("" : : : "memory");
        } else {
            s = s * 3;
            __asm__ volatile("" : : : "memory");
        }
        
        /* Switch statement for multiple basic blocks */
        switch (i % 4) {
            case 0:
                s += a[i % N];
                break;
            case 1:
                s -= b[i % N];
                break;
            case 2:
                s *= 2;
                break;
            case 3:
                s = s ^ a[i % N];
                break;
        }
        
        /* Inner loop with complex dependencies */
        for (int j = 0; j < M; ++j) {
            /* Variable array access with volatile influence */
            int j_idx = (j + *trigger) % M;
            c[j_idx] += s * j;
            
            /* More conditional logic */
            if (c[j_idx] > 10000) {
                c[j_idx] = c[j_idx] % 1000;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Nested condition */
            if (j % 2 == 0) {
                result += c[j_idx];
            } else {
                result -= c[j_idx];
            }
        }
        
        /* Update volatile to create side effects */
        *trigger = (*trigger + 1) % 16;
    }
    
    return result;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline))
static void process_array(int *arr, int size, volatile int *mod) {
    for (int i = 0; i < size; ++i) {
        /* Complex arithmetic chain */
        int val = arr[i];
        val = (val << (*mod % 8)) | (val >> (8 - (*mod % 8)));
        val = val ^ 0x55AA55AA;
        val = val + (*mod * i);
        
        /* Conditional store */
        if (val % 3 == 0) {
            arr[i] = val;
        } else if (val % 3 == 1) {
            arr[i] = val * 2;
        } else {
            arr[i] = val / 2;
        }
        
        /* Update volatile periodically */
        if (i % 7 == 0) {
            *mod = (*mod + 1) % 32;
            __asm__ volatile("" : : : "memory");
        }
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ARR_SIZE = 200;
    
    /* Initialize arrays with pseudo-random values */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *arr = (int*)malloc(ARR_SIZE * sizeof(int));
    
    /* Simple deterministic random generator */
    srand(42);
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 100;
    }
    for (int i = 0; i < ARR_SIZE; ++i) {
        arr[i] = rand() % 5000;
    }
    
    /* Volatile variable to prevent optimization */
    volatile int trigger = 0;
    
    /* Call complex computation functions */
    int checksum1 = compute_checksum(a, b, c, N, M, &trigger);
    
    /* Reset trigger and process another array */
    trigger = 5;
    process_array(arr, ARR_SIZE, &trigger);
    
    /* Compute final checksum */
    int checksum2 = 0;
    for (int i = 0; i < ARR_SIZE; ++i) {
        checksum2 += arr[i];
    }
    
    /* Ensure results are used */
    int final_result = checksum1 + checksum2;
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
