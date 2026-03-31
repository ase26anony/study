/* sel-sched-test.c - Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_limit = 1000;

/* Non-inlineable function with complex loop nest */
__attribute__((noinline, cold)) 
int compute_checksum(int* a, int* b, int* c, int N, int M) {
    int s = 0;
    int i, j;
    
    /* Complex loop with register pressure and dependencies */
    for (i = 0; i < N; ++i) {
        /* Array accesses with variable indices */
        int idx = (i + g_volatile_counter) % N;
        
        /* Multiple arithmetic operations */
        s += a[idx] * b[i];
        s -= b[idx] * a[i];
        s = s ^ (a[i] + b[idx]);
        
        /* Conditional branch creating multiple basic blocks */
        if (s > g_volatile_limit) {
            /* Path A: Reset and do more computation */
            s = 0;
            for (j = 0; j < 3; ++j) {
                s += j * a[(i + j) % N];
            }
        } else if (s < -g_volatile_limit) {
            /* Path B: Different computation pattern */
            s = s >> 2;
            for (j = 0; j < 2; ++j) {
                s += b[(i + j) % N] << j;
            }
        } else {
            /* Path C: Yet another computation pattern */
            s = s * 3 + 1;
            __asm__ volatile("" : : : "memory"); /* Memory barrier */
        }
        
        /* Inner loop with more register pressure */
        for (j = 0; j < M; ++j) {
            /* Complex addressing and computation */
            int mod_j = j % 8;
            c[j] += s * mod_j;
            c[j] -= a[i] * b[(i + j) % N];
            
            /* Another conditional inside inner loop */
            if ((i * j) % 7 == 0) {
                c[j] = c[j] ^ 0x55AA55AA;
            } else if ((i * j) % 5 == 0) {
                c[j] = c[j] >> 1;
            }
            
            /* Prevent loop optimization */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += a[i] * 2;
                break;
            case 1:
                s -= b[i] * 3;
                break;
            case 2:
                s = s ^ a[i];
                break;
            case 3:
                s = (s + b[i]) & 0xFF;
                break;
        }
        
        /* Update volatile to prevent dead code elimination */
        g_volatile_counter++;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
void process_arrays(int* arr1, int* arr2, int size) {
    int temp = 0;
    volatile int v = 1;
    
    for (int i = 0; i < size; i++) {
        /* Complex dependency chain */
        temp = temp * 1103515245 + 12345;
        arr1[i] = (temp >> 16) & 0x7FFF;
        
        /* Conditional with side effects */
        if (arr1[i] % 3 == v) {
            arr2[i] = arr1[i] * arr1[(i + 1) % size];
            v = arr2[i] % 7;
        } else {
            arr2[i] = arr1[i] + arr1[(i + size - 1) % size];
            __asm__ volatile("" : : : "memory");
        }
        
        /* Nested loop with small iteration count */
        for (int k = 0; k < 4; k++) {
            arr2[i] += (arr1[i] << k) - k;
        }
    }
}

int main() {
    const int N = 128;  /* Large enough for scheduling decisions */
    const int M = 64;
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(M * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (int i = 0; i < M; i++) {
        c[i] = 0;
    }
    
    /* Call complex computation function */
    int checksum = compute_checksum(a, b, c, N, M);
    
    /* Process arrays with different pattern */
    process_arrays(a, b, N);
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += a[i] + b[i];
    }
    for (int i = 0; i < M; i++) {
        final_sum += c[i];
    }
    final_sum += checksum;
    
    printf("Result: %d (volatile counter: %d)\n", final_sum, g_volatile_counter);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
