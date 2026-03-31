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
        /* Use volatile in condition to prevent dead code elimination */
        if (g_volatile_trigger) {
            /* Multiple arithmetic operations with dependencies */
            int idx = (i * 7 + 3) % N;
            s += a[idx] * b[i];
            s -= b[idx] * a[i];
            
            /* Conditional branch creating multiple basic blocks */
            if (s > 1000) {
                s = s % 1000;
                /* Inline assembly barrier to create artificial dependencies */
                __asm__ volatile("" : : : "memory");
            } else if (s < -1000) {
                s = -s % 500;
                __asm__ volatile("" : : : "memory");
            } else {
                s = s * 2 - s / 3;
            }
            
            /* Inner loop with more operations */
            for (j = 0; j < M; ++j) {
                /* Complex addressing with multiple operations */
                int jdx = (j + i) % M;
                c[jdx] += s * j;
                c[j] -= s * (i % 5);
                
                /* Switch statement for additional basic blocks */
                switch (j % 4) {
                    case 0:
                        c[j] += a[i] * 2;
                        break;
                    case 1:
                        c[j] -= b[i] * 3;
                        break;
                    case 2:
                        c[j] += s * 4;
                        break;
                    default:
                        c[j] -= (a[i] + b[i]) / 2;
                        __asm__ volatile("" : : : "memory");
                        break;
                }
                
                /* Prevent loop unrolling */
                g_volatile_counter++;
            }
            
            /* Another conditional with volatile */
            if (i % 7 == 0 && g_volatile_trigger) {
                s = s ^ (a[i] * b[i]);
                __asm__ volatile("" : : : "memory");
            }
        }
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_arrays(int *arr1, int *arr2, int *arr3, int size) {
    int temp = 0;
    volatile int v = 1;
    
    for (int i = 0; i < size; i++) {
        /* Complex dependency chain */
        int idx1 = (i * 3) % size;
        int idx2 = (i * 5) % size;
        
        arr3[i] = arr1[idx1] * arr2[idx2] - arr1[idx2] * arr2[idx1];
        
        /* Nested conditionals */
        if (arr3[i] > 0) {
            if (v) {
                arr3[i] = arr3[i] % 256;
                temp += arr3[i];
            }
        } else {
            arr3[i] = -arr3[i] % 128;
            temp -= arr3[i];
        }
        
        /* More arithmetic with volatile */
        if (i % 11 == 0 && v) {
            arr3[i] ^= temp;
            __asm__ volatile("" : : : "memory");
        }
    }
    
    /* Use temp to prevent dead code elimination */
    g_volatile_counter += temp;
}

int main(void) {
    const int N = 100;
    const int M = 50;
    const int SIZE = 200;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(M * sizeof(int));
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    int *arr3 = (int *)malloc(SIZE * sizeof(int));
    
    /* Simple deterministic initialization */
    srand(42);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = 0;
    }
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 500;
        arr2[i] = rand() % 500;
        arr3[i] = 0;
    }
    
    /* Call functions with complex loops */
    int checksum1 = compute_checksum(a, b, c, N, M);
    process_arrays(arr1, arr2, arr3, SIZE);
    
    /* Compute final checksum to prevent optimization */
    int final_checksum = checksum1;
    for (int i = 0; i < M; i++) {
        final_checksum += c[i];
    }
    for (int i = 0; i < SIZE; i++) {
        final_checksum += arr3[i];
    }
    final_checksum += g_volatile_counter;
    
    printf("Result: %d\n", final_checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
