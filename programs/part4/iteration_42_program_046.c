/* Test case to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Non-inlineable function to stress the selective scheduler */
static void __attribute__((noinline,noipa))
stress_sched(int *result) {
    volatile int seed = 42; /* volatile to create scheduling barriers */
    int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    int i, j;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr[i] = i * 0.5f;
    }
    
    /* Complex inner loop with high ILP potential */
    for (j = 0; j < ITERS; j++) {
        int idx = j % SIZE;
        int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
        float ft1, ft2, ft3, ft4;
        
        /* Chain of dependent integer operations */
        t1 = arr1[idx] + seed;
        t2 = t1 * arr2[(idx + 1) % SIZE];
        t3 = t2 - (arr1[(idx + 2) % SIZE] >> 3);
        t4 = t3 ^ (t1 & 0xFF);
        t5 = t4 * 1103515245 + 12345;
        
        /* Floating-point operations mixed in */
        ft1 = farr[idx] * 1.5f;
        ft2 = ft1 + farr[(idx + 3) % SIZE];
        ft3 = ft2 * 0.75f;
        ft4 = ft3 - (float)t5 * 0.001f;
        
        /* More integer operations with dependencies */
        t6 = (int)ft4 * 7;
        t7 = t6 + t5;
        t8 = t7 ^ t3;
        t9 = t8 * 1664525 + 1013904223;
        t10 = t9 % 65536;
        
        /* Conditional execution with side effects */
        if (t10 > 32768) {
            /* Branch 1: complex operations */
            arr1[idx] = t10 + t5;
            arr2[(idx + 4) % SIZE] = t8 * 3;
            farr[idx] = ft4 * 2.0f;
            
            /* Additional computation in this branch */
            t7 = t7 * 2 - t6;
            arr1[(idx + 5) % SIZE] += t7;
        } else {
            /* Branch 2: different operations */
            arr1[idx] = t10 - t5;
            arr2[(idx + 4) % SIZE] = t8 / 3;
            farr[idx] = ft4 * 0.5f;
            
            /* Different computation path */
            t9 = t9 ^ 0xAAAA;
            arr2[(idx + 6) % SIZE] -= t9;
        }
        
        /* Inline assembly as scheduling boundary */
        asm volatile("" ::: "memory");
        
        /* Use values computed much earlier in the loop */
        arr1[(idx + 7) % SIZE] = t3 + t6;  /* t3 computed early */
        arr2[(idx + 8) % SIZE] = t4 ^ t9;  /* t4 computed early */
        farr[(idx + 9) % SIZE] = ft1 + ft3; /* ft1 computed early */
        
        /* More operations extending live ranges */
        int t11 = arr1[(idx + 10) % SIZE] * 3;
        int t12 = arr2[(idx + 11) % SIZE] + t11;
        float ft5 = farr[(idx + 12) % SIZE] * 2.0f;
        
        /* Final store with complex addressing */
        arr1[(idx + t12 % 16) % SIZE] = t10 + t12;
        arr2[(idx + (int)ft5 % 16) % SIZE] = t8 + t11;
        
        /* Update seed for next iteration */
        seed = t10;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += arr1[i] + arr2[i] + (int)farr[i];
    }
    *result = sum;
}

/* Another function with different pattern to increase scheduling complexity */
static void __attribute__((noinline,noipa))
stress_sched2(int *result) {
    int matrix[16][16];
    int i, j, k;
    
    /* Initialize matrix */
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Matrix operations with dependencies */
    for (k = 0; k < 1000; k++) {
        for (i = 1; i < 15; i++) {
            for (j = 1; j < 15; j++) {
                /* Stencil computation with multiple dependencies */
                int val = (matrix[i-1][j] + matrix[i+1][j] +
                          matrix[i][j-1] + matrix[i][j+1]) / 4;
                
                /* Conditional with arithmetic */
                if (val > 128) {
                    matrix[i][j] = (val * 3) / 2;
                    matrix[i][j] ^= 0xFF;
                } else {
                    matrix[i][j] = (val * 5) / 4;
                    matrix[i][j] &= 0x7F;
                }
                
                /* Inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Cross-iteration dependency */
                matrix[i][j] += matrix[(i + k) % 16][(j + 1) % 16];
            }
        }
    }
    
    /* Compute result */
    int sum = 0;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            sum += matrix[i][j];
        }
    }
    *result = sum;
}

int main() {
    int result1 = 0, result2 = 0;
    
    /* Call stress functions multiple times */
    for (int i = 0; i < 10; i++) {
        stress_sched(&result1);
        stress_sched2(&result2);
    }
    
    printf("Results: %d %d\n", result1, result2);
    return 0;
}
