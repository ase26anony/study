/* sel-sched-test.c - Program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 20
#define LIMIT 1000

/* Function attributes to prevent optimization */
__attribute__((noinline, cold))
static int compute_checksum(int *a, int *b, int *c, int size_a, int size_c) {
    volatile int trigger = 0;  /* Volatile to prevent elimination */
    int s = 0;
    int i, j;
    
    /* Complex loop nest with register pressure */
    for (i = 0; i < size_a; ++i) {
        /* Artificial dependency via volatile */
        trigger = i % 2;
        
        /* Multiple arithmetic operations */
        s += a[i] * b[i];
        s -= (i * 3) / 2;
        s ^= (i << 2);
        
        /* Conditional with multiple basic blocks */
        if (s > LIMIT) {
            s = 0;
            /* Additional operations in this path */
            a[i] = s + trigger;
        } else if (s < -LIMIT) {
            s = LIMIT / 2;
            b[i] = s - trigger;
        } else {
            /* Third path with different operations */
            s = (s * 2) % LIMIT;
            __asm__ volatile("" : : : "memory");  /* Memory barrier */
        }
        
        /* Switch statement for more basic blocks */
        switch (i % 4) {
            case 0:
                s += 5;
                break;
            case 1:
                s -= 3;
                break;
            case 2:
                s *= 2;
                break;
            case 3:
                s /= 2;
                break;
        }
        
        /* Inner loop with array accesses */
        for (j = 0; j < size_c; ++j) {
            /* Complex addressing with volatile */
            int idx = (j + trigger) % size_c;
            c[idx] += s * j;
            c[idx] -= i * 2;
            
            /* More conditionals inside inner loop */
            if (c[idx] > 10000) {
                c[idx] = 10000;
            } else if (c[idx] < -10000) {
                c[idx] = -10000;
            }
            
            /* Another memory barrier */
            __asm__ volatile("" : : : "memory");
        }
        
        /* Additional arithmetic to increase register pressure */
        s = (s * 7 + 11) % 997;
    }
    
    return s;
}

/* Another non-inlineable function with different pattern */
__attribute__((noinline, cold))
static void process_arrays(int *arr1, int *arr2, int size) {
    volatile int v = 1;
    int i, temp;
    
    for (i = 0; i < size; i += 2) {
        /* Complex pointer arithmetic */
        int *p1 = &arr1[i];
        int *p2 = &arr2[i % (size/2)];
        
        /* Multiple operations with dependencies */
        temp = *p1 * 3 + *p2 * 2;
        *p1 = temp - v;
        
        /* Nested if-else chain */
        if (temp > 500) {
            *p2 = temp / 2;
            v = 2;
        } else if (temp > 200) {
            *p2 = temp / 3;
            v = 3;
        } else if (temp > 50) {
            *p2 = temp / 4;
            v = 4;
        } else {
            *p2 = temp;
            v = 5;
        }
        
        /* More arithmetic */
        arr1[i+1] = (arr1[i+1] + temp) * v;
        __asm__ volatile("" : : : "memory");
    }
}

int main(void) {
    int a[N], b[N], c[M];
    int i, result;
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (i = 0; i < N; ++i) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    for (i = 0; i < M; ++i) {
        c[i] = rand() % 50;
    }
    
    /* Call the complex function to trigger selective scheduling */
    result = compute_checksum(a, b, c, N, M);
    
    /* Process arrays with different pattern */
    process_arrays(a, b, N);
    
    /* Compute final checksum to prevent optimization */
    int checksum = result;
    for (i = 0; i < N; ++i) {
        checksum += a[i] + b[i];
    }
    for (i = 0; i < M; ++i) {
        checksum += c[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
