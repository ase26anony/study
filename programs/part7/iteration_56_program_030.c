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
    
    /* Complex loop nest with register pressure */
    for (i = 0; i < N; ++i) {
        /* Array accesses with variable indices */
        s += a[i] * b[i];
        
        /* Conditional branch creating multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Path A: reset branch */
            a[i] = s;
        } else if (s < -LIMIT) {
            s = 1;
            /* Path B: negative overflow branch */
            b[i] = s;
        } else {
            /* Path C: normal path */
            if (i % 2 == 0) {
                s = s * 2;
            } else {
                s = s / 2;
            }
        }
        
        /* Use volatile variable to prevent optimization */
        if (*trigger > i) {
            s += *trigger;
        }
        
        /* Inner loop with complex dependencies */
        for (j = 0; j < M; ++j) {
            /* Artificial dependency via inline assembly */
            __asm__ volatile("" : : : "memory");
            
            /* Complex arithmetic with array access */
            c[j] += s * j;
            
            /* More conditionals in inner loop */
            if (c[j] > 1000) {
                c[j] = c[j] % 1000;
            }
        }
        
        /* Switch statement for additional basic blocks */
        switch (i % 4) {
            case 0:
                s += 1;
                break;
            case 1:
                s -= 1;
                break;
            case 2:
                s *= 2;
                break;
            case 3:
                s /= 2;
                break;
        }
        
        /* Another memory barrier */
        __asm__ volatile("" : : : "memory");
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_array(int *arr, volatile int *flag) {
    int sum = 0;
    int i;
    
    for (i = 0; i < N; ++i) {
        /* Complex addressing */
        int idx = (i * 13 + 7) % N;
        
        /* Conditional with side effects */
        if (arr[idx] > *flag) {
            arr[idx] = arr[idx] - *flag;
            sum += arr[idx];
        } else {
            arr[idx] = arr[idx] + *flag;
            sum -= arr[idx];
        }
        
        /* Nested loop with variable bound */
        for (int k = 0; k < (i % 8) + 1; ++k) {
            arr[idx] ^= k;
            __asm__ volatile("" : : : "memory");
        }
    }
    
    /* Use the result */
    *flag = sum;
}

int main(void) {
    int a[N], b[N], c[M];
    volatile int trigger = 42;  /* Volatile to prevent optimization */
    int i, result;
    
    /* Initialize with pseudo-random but deterministic values */
    srand(12345);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 1000;
    }
    
    /* Call the complex computation function */
    result = compute_checksum(a, b, c, &trigger);
    
    /* Process arrays with another pattern */
    process_array(a, &trigger);
    process_array(b, &trigger);
    
    /* Compute final checksum to ensure no optimization */
    int checksum = result + trigger;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
