#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Create many live variables with different types to force pseudo-registers */
            volatile char v1 = arr1[i] & 0xFF;
            volatile short v2 = arr2[i] & 0xFFFF;
            volatile int v3 = arr1[i] ^ arr2[i];
            volatile long v4 = (long)arr1[i] * (long)arr2[i];
            volatile int v5 = v3 + v1;  /* Uses immediate constant +1 candidate */
            volatile int v6 = v2 * 2;   /* Immediate constant *2 candidate */
            volatile int v7 = v4 >> 3;  /* Immediate constant >>3 candidate */
            volatile int v8 = v5 | 0x7F; /* Immediate constant |0x7F candidate */
            volatile int v9 = v6 & 0xFF00;
            volatile int v10 = v7 + 1;   /* Another +1 immediate */
            volatile int v11 = v8 * 3;   /* *3 immediate */
            volatile int v12 = v9 - 4;   /* -4 immediate */
            volatile int v13 = v10 ^ 0x55; /* XOR immediate */
            volatile int v14 = v11 << 2;  /* <<2 immediate */
            volatile int v15 = v12 % 7;   /* %7 immediate */
            
            /* Complex interdependent calculations creating def-use chains */
            volatile int t1 = v1 + v2;
            volatile int t2 = t1 * v3;
            volatile int t3 = t2 & v4;
            volatile int t4 = t3 | v5;
            volatile int t5 = t4 ^ v6;
            volatile int t6 = t5 + v7;
            volatile int t7 = t6 - v8;
            volatile int t8 = t7 * v9;
            volatile int t9 = t8 & v10;
            volatile int t10 = t9 | v11;
            volatile int t11 = t10 ^ v12;
            volatile int t12 = t11 + v13;
            volatile int t13 = t12 - v14;
            volatile int t14 = t13 * v15;
            
            /* Address computation with loop-invariant base - remat candidate */
            volatile int *base_ptr = arr3;
            volatile int offset = i * sizeof(int);
            volatile int *addr = (volatile int *)((char *)base_ptr + offset);
            
            /* Conditional branches creating multiple basic blocks */
            if (v1 & 0x1) {
                /* Use immediate constants in different basic block */
                t14 += 256;  /* +256 immediate */
                *addr = t14;
                asm volatile("" : : : "memory"); /* Barrier in conditional */
            } else if (v2 & 0x2) {
                t14 *= 2;    /* *2 immediate in else block */
                *addr = t14 >> 1;
            } else {
                t14 &= 0xFF; /* &0xFF immediate in else block */
                *addr = t14 | 0x80;
            }
            
            /* More computations to extend live ranges */
            volatile int u1 = t14 + arr4[i];
            volatile int u2 = u1 * (i + 1);  /* i+1 immediate candidate */
            volatile int u3 = u2 & 0xAAAAAAAA;
            volatile int u4 = u3 | 0x55555555;
            volatile int u5 = u4 ^ t14;
            
            /* Nested conditional with more immediates */
            if (u5 > 1000) {
                u5 -= 100;  /* -100 immediate */
                result += u5 * 2;  /* *2 immediate */
            } else if (u5 < -1000) {
                u5 += 200;  /* +200 immediate */
                result += u5 / 4;  /* /4 immediate */
            } else {
                u5 &= 0xFFF;  /* &0xFFF immediate */
                result += u5 | 0x1000;
            }
            
            /* Force spill/reload behavior with volatile stores */
            arr1[i] = u5;
            arr2[i] = result;
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Modify arrays to prevent loop-invariant code motion */
        for (int j = 0; j < ARRAY_SIZE; j += 64) {
            arr3[j] = outer * j;
            arr4[j] = arr3[j] ^ arr1[j];
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate arrays with volatile to force real memory ops */
    volatile int *arr1 = malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr4 = malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand();
        arr2[i] = rand();
        arr3[i] = rand();
        arr4[i] = rand();
    }
    
    /* Call high pressure loop multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += high_pressure_loop(arr1, arr2, arr3, arr4);
        
        /* Shuffle data to prevent optimization */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr1[i] ^= total;
            arr2[i] += iter;
        }
    }
    
    free((void *)arr1);
    free((void *)arr2);
    free((void *)arr3);
    free((void *)arr4);
    
    return total;
}

int main(void) {
    volatile int checksum = setup_and_run();
    printf("Checksum: %d\n", checksum);
    return 0;
}
