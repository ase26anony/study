/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_trigger = 1;

/* Function attributes to prevent inlining and influence scheduling */
__attribute__((noinline, cold))
int compute_checksum(int *a, int *b, int *c, int N, int M) {
    int s = 0;
    int i, j;
    
    /* Complex loop nest with high register pressure */
    for (i = 0; i < N; ++i) {
        /* Use volatile in condition to prevent dead code elimination */
        if (g_volatile_trigger) {
            /* Multiple arithmetic operations with dependencies */
            s += a[i] * b[i];
            
            /* Conditional branch creating multiple basic blocks */
            if (s > 1000) {
                s = 0;
                /* Artificial memory barrier */
                __asm__ volatile("" : : : "memory");
            } else if (s < -1000) {
                s = 100;
                /* Another memory barrier */
                __asm__ volatile("" : : : "memory");
            } else {
                /* Default path with more operations */
                s = s * 2 - a[i];
            }
            
            /* Switch-like structure for multiple basic blocks */
            switch (i % 4) {
                case 0:
                    s += i * 3;
                    break;
                case 1:
                    s -= i * 2;
                    break;
                case 2:
                    s *= (i % 10) + 1;
                    break;
                default:
                    s = s / ((i % 5) + 1);
                    break;
            }
            
            /* Inner loop with more operations */
            for (j = 0; j < M; ++j) {
                /* Complex array access pattern */
                int idx = (i + j) % M;
                c[idx] += s * j;
                
                /* More conditional logic */
                if (c[idx] > 5000) {
                    c[idx] = c[idx] % 1000;
                    g_volatile_counter++;
                }
                
                /* Additional arithmetic to increase register pressure */
                c[idx] = c[idx] * 2 - b[i % N];
            }
            
            /* Use volatile variable to prevent optimization */
            if (g_volatile_counter > 100) {
                g_volatile_counter = 0;
            }
        }
    }
    
    return s;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline))
void process_arrays(int *a, int *b, int *c, int N, int M) {
    int temp1 = 0, temp2 = 0, temp3 = 0;
    
    for (int i = 0; i < N; i += 2) {
        /* Unrolled operations */
        temp1 = a[i] * b[i] + c[i % M];
        temp2 = a[i+1] * b[i+1] - c[(i+1) % M];
        
        /* Complex dependency chain */
        for (int k = 0; k < 3; ++k) {
            temp3 = temp1 * temp2 + k;
            temp1 = temp2 - temp3;
            temp2 = temp3 * 2;
            
            /* Memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Store results with volatile access */
        a[i] = temp1 + g_volatile_trigger;
        a[i+1] = temp2 - g_volatile_trigger;
    }
}

int main(void) {
    const int N = 100;
    const int M = 50;
    int *a, *b, *c;
    int result;
    
    /* Allocate and initialize arrays with pseudo-random values */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(M * sizeof(int));
    
    /* Simple deterministic initialization */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 13 + 7) % 100;
        b[i] = (i * 17 + 11) % 100;
    }
    for (int i = 0; i < M; ++i) {
        c[i] = (i * 19 + 13) % 100;
    }
    
    /* Call complex functions to trigger selective scheduling */
    result = compute_checksum(a, b, c, N, M);
    process_arrays(a, b, c, N, M);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += a[i] + b[i];
    }
    for (int i = 0; i < M; ++i) {
        final_sum += c[i];
    }
    final_sum += result + g_volatile_counter;
    
    printf("Result: %d, Final sum: %d\n", result, final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
