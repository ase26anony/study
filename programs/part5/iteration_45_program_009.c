#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    volatile int i, j;
    
    /* Outer loop with volatile bound to prevent optimization */
    for (i = 0; i < bound; i++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Inner loop creating extreme register pressure */
        for (j = 0; j < ITERS; j++) {
            /* Complex interdependent calculations with immediate constants */
            /* These constants are rematerialization candidates */
            v1 = arr1[j & (SIZE-1)] + 1;      /* Immediate +1 */
            v2 = arr2[j & (SIZE-1)] * 2;      /* Immediate *2 */
            v3 = v1 & 0xFF;                   /* Immediate mask */
            v4 = v2 | 0x80;                   /* Immediate OR */
            v5 = v3 - 7;                      /* Immediate -7 */
            v6 = v4 ^ 0x55;                   /* Immediate XOR */
            
            /* More variables with different data widths */
            volatile char c1 = (v5 & 0xFF) + 1;
            volatile short s1 = (v6 & 0xFFFF) * 2;
            volatile long l1 = (long)v5 * (long)v6;
            
            v7 = c1 + s1;
            v8 = (int)l1 & 0x7FFFFFFF;
            v9 = v7 * 3;                      /* Immediate *3 */
            v10 = v8 / 4;                     /* Immediate /4 */
            
            /* Address computation with loop-invariant base */
            /* This creates rematerialization candidates for address arithmetic */
            volatile int *ptr1 = arr1 + (i & 0xF);
            volatile int *ptr2 = arr2 + (j & 0xF);
            v11 = *ptr1 + *ptr2;
            v12 = v11 << 2;                   /* Immediate shift */
            
            /* More calculations with immediate constants */
            v13 = v9 + v10;
            v14 = v12 - v13;
            v15 = v14 & 0x0F0F0F0F;           /* Large immediate mask */
            v16 = v15 | 0x01010101;           /* Large immediate OR */
            v17 = v16 ^ 0xFFFFFFFF;           /* Immediate XOR */
            v18 = v17 + 42;                   /* Immediate +42 */
            v19 = v18 * 100;                  /* Immediate *100 */
            v20 = v19 % 97;                   /* Immediate %97 */
            
            /* Conditional branches creating multiple basic blocks */
            if (v20 & 1) {
                /* Use different immediate in this path */
                result += v20 + 256;          /* Immediate +256 */
            } else {
                result += v20 - 128;          /* Immediate -128 */
            }
            
            /* Another conditional with volatile check */
            if (arr3[j & (SIZE-1)] > 0) {
                /* More calculations with immediate constants */
                v1 = v20 * 8;                 /* Immediate *8 */
                v2 = v1 + 16;                 /* Immediate +16 */
                result ^= v2;
            }
            
            /* Force spill/reload behavior with memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        arr1[i & (SIZE-1)] = result & 0xFF;
        arr2[i & (SIZE-1)] = (result >> 8) & 0xFF;
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    volatile int array1[SIZE];
    volatile int array2[SIZE];
    volatile int array3[SIZE];
    volatile int i, seed = 42;
    
    /* Initialize with pseudo-random data */
    srand(seed);
    for (i = 0; i < SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Volatile bound to prevent loop unrolling from reducing pressure */
    volatile int bound = 50;
    
    /* Call the high pressure function */
    return high_pressure_loop(array1, array2, array3, bound);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run multiple times to ensure ER activation */
    for (int run = 0; run < 3; run++) {
        checksum += setup_and_run();
        
        /* Memory barrier between runs */
        asm volatile("" : : : "memory");
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
