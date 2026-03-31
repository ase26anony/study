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
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > 1000) {
            s = 0;
            /* Additional operations in this path */
            for (int k = 0; k < 5; ++k) {
                s += k * trigger;
            }
        } else if (s < -500) {
            s = 100;
            /* Different operations in else-if path */
            for (int k = 0; k < 3; ++k) {
                s -= k * trigger;
            }
        } else {
            /* Third path with switch statement */
            switch (i % 4) {
                case 0:
                    s += trigger * 2;
                    break;
                case 1:
                    s -= trigger;
                    break;
                case 2:
                    s *= 2;
                    break;
                default:
                    s /= 2;
                    break;
            }
        }
        
        /* Inner loop with memory accesses */
        for (int j = 0; j < M; ++j) {
            /* Complex addressing with volatile dependency */
            int idx = (j + trigger) % M;
            c[idx] += s * j;
            
            /* Memory barrier to prevent reordering */
            __asm__ volatile("" : : : "memory");
            
            /* Additional arithmetic */
            result += c[idx] % 256;
        }
        
        /* More arithmetic with dependencies */
        a[i] = (a[i] + s) % 1000;
        b[i] = (b[i] - s) % 1000;
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return result;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static int secondary_computation(int *arr, int size) {
    volatile int seed = 42;
    int sum = 0;
    
    /* Loop with data-dependent branches */
    for (int i = 0; i < size; ++i) {
        int val = arr[i];
        
        /* Nested conditionals */
        if (val > 0) {
            if (val > 100) {
                sum += val * 2;
            } else {
                sum += val / 2;
            }
        } else {
            sum -= (-val) * 3;
        }
        
        /* Modulo operation with variable divisor */
        arr[i] = sum % (seed + i + 1);
        
        /* Prevent optimization */
        __asm__ volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int ARR_SIZE = 200;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(M * sizeof(int));
    int *arr = (int*)malloc(ARR_SIZE * sizeof(int));
    
    if (!a || !b || !c || !arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(12345);  /* Fixed seed for reproducibility */
    
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        c[i] = rand() % 500;
    }
    
    for (int i = 0; i < ARR_SIZE; ++i) {
        arr[i] = rand() % 1000 - 500;  /* Range: -500 to 499 */
    }
    
    /* Call the complex computation functions */
    int checksum1 = compute_checksum(a, b, c, N, M);
    int checksum2 = secondary_computation(arr, ARR_SIZE);
    
    /* Final computation to ensure nothing is optimized away */
    int final_result = checksum1 + checksum2;
    
    /* Additional computation mixing all arrays */
    for (int i = 0; i < 10; ++i) {
        final_result += a[i % N] + b[i % N] + c[i % M] + arr[i % ARR_SIZE];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr);
    
    return 0;
}
