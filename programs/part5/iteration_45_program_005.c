#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* 15+ distinct integer variables with different types */
                volatile char v1 = arr1[i] & 0xFF;
                volatile short v2 = arr2[i] & 0xFFFF;
                volatile int v3 = arr3[i];
                volatile long v4 = arr4[i];
                
                /* Create complex dependency chain with immediate constants */
                /* These constants are rematerialization candidates */
                volatile int t1 = v1 + 1;          /* Candidate: constant 1 */
                volatile int t2 = v2 * 2;          /* Candidate: constant 2 */
                volatile int t3 = v3 & 0x7F;       /* Candidate: constant 0x7F */
                volatile int t4 = v4 | 0xFF00;     /* Candidate: constant 0xFF00 */
                
                /* More variables to increase pressure */
                volatile int t5 = t1 + t2;
                volatile int t6 = t3 - t4;
                volatile int t7 = t5 * 3;          /* Candidate: constant 3 */
                volatile int t8 = t6 / 4;          /* Candidate: constant 4 */
                volatile int t9 = t7 & 0x3F;       /* Candidate: constant 0x3F */
                volatile int t10 = t8 | 0x1F;      /* Candidate: constant 0x1F */
                
                /* Address computation with loop-invariant base + offset */
                /* This creates REG references for rematerialization */
                volatile int *ptr1 = (volatile int*)arr1;
                volatile int *ptr2 = (volatile int*)arr2;
                volatile int idx1 = i + 5;         /* Candidate: constant 5 */
                volatile int idx2 = i * 2;         /* Candidate: constant 2 */
                
                /* Conditional branches create multiple basic blocks */
                if (v1 & 1) {
                    /* Use immediate constants in different basic block */
                    volatile int t11 = t9 + 8;     /* Candidate: constant 8 */
                    volatile int t12 = t10 - 16;   /* Candidate: constant 16 */
                    volatile int val1 = ptr1[idx1] + 32;  /* Candidate: constant 32 */
                    volatile int val2 = ptr2[idx2] * 64;  /* Candidate: constant 64 */
                    
                    /* Memory barrier to prevent reordering */
                    asm volatile("" : : : "memory");
                    
                    t11 = t11 ^ val1;
                    t12 = t12 & val2;
                    result += t11 * t12;
                } else {
                    /* Alternative path with different constants */
                    volatile int t13 = t9 * 128;   /* Candidate: constant 128 */
                    volatile int t14 = t10 / 256;  /* Candidate: constant 256 */
                    volatile int val3 = ptr1[i] + 512;    /* Candidate: constant 512 */
                    volatile int val4 = ptr2[i] - 1024;   /* Candidate: constant 1024 */
                    
                    /* Another memory barrier */
                    asm volatile("" : : : "memory");
                    
                    t13 = t13 | val3;
                    t14 = t14 ^ val4;
                    result -= t13 + t14;
                }
                
                /* More arithmetic with constants to increase remat candidates */
                volatile int t15 = result + 2048;  /* Candidate: constant 2048 */
                volatile int t16 = t15 & 4096;     /* Candidate: constant 4096 */
                volatile int t17 = t16 | 8192;     /* Candidate: constant 8192 */
                volatile int t18 = t17 - 16384;    /* Candidate: constant 16384 */
                
                /* Final result accumulation with volatile to prevent elimination */
                result = t18;
                
                /* Force spill/reload behavior with periodic barrier */
                if ((i & 31) == 0) {
                    asm volatile("" : : : "memory");
                }
            }
        }
        
        /* Modify arrays slightly each outer iteration to prevent loop elimination */
        arr1[outer % ARRAY_SIZE] ^= 1;
        arr2[outer % ARRAY_SIZE] += 1;
    }
    
    return result;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr4 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand();
        arr2[i] = rand();
        arr3[i] = rand();
        arr4[i] = rand();
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += high_pressure_loop(arr1, arr2, arr3, arr4);
        
        /* Periodically modify data to prevent optimization */
        if ((iter & 255) == 0) {
            arr1[iter % ARRAY_SIZE] = rand();
        }
    }
    
    printf("Result checksum: %d\n", total);
    
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    free((void*)arr4);
    
    return 0;
}
