#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent optimization of the core computation */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Create many live variables with overlapping ranges */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Force memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Inner loop with extreme register pressure */
        for (volatile int i = 0; i < ARRAY_SIZE; i++) {
            /* Load data with volatile to force real memory accesses */
            v1 = a[i] + 1;  /* Immediate constant candidate for remat */
            v2 = b[i] * 2;  /* Another immediate constant */
            v3 = c[i] & 0xFF; /* Mask operation */
            v4 = d[i] | 0x80;
            
            /* Create complex dependency chain */
            v5 = v1 + v2;
            v6 = v3 - v4;
            v7 = v5 * v6;
            v8 = v7 & 0x3F;  /* Another immediate */
            v9 = v8 | 0x40;
            
            /* More operations with different widths to create partial reg deps */
            volatile char c1 = (char)(v9 & 0xFF);
            volatile short s1 = (short)(v9 >> 8);
            volatile long l1 = (long)v9 * 256L;
            
            v10 = (int)c1 + (int)s1 + (int)(l1 & 0xFFFF);
            v11 = v10 * 3;  /* Immediate constant */
            v12 = v11 / 4;  /* Another immediate */
            v13 = v12 << 2; /* Shift immediate */
            v14 = v13 >> 1;
            
            /* Conditional branch creating multiple basic blocks */
            if (v14 & 1) {  /* Volatile check */
                v15 = v14 + 5;  /* Different immediate */
                v16 = v15 * 6;
                v17 = v16 & 0x7F;
            } else {
                v15 = v14 - 3;
                v16 = v15 * 7;
                v17 = v16 | 0x3F;
            }
            
            /* More arithmetic with immediate constants */
            v18 = v17 + 8;
            v19 = v18 * 9;
            v20 = v19 % 10;
            
            /* Address computation with loop-invariant base - 
               prime candidate for rematerialization */
            volatile int *ptr = &a[0];  /* Loop invariant */
            volatile int idx = i * sizeof(int);
            volatile int offset = (int)((uintptr_t)ptr + idx);
            
            /* Use the computed address */
            v20 += offset & 0xFF;
            
            /* Accumulate result with volatile to prevent dead code elimination */
            result += v20;
            
            /* Artificial dependency between iterations */
            a[i] = (v20 & 1) ? b[i] : c[i];
            
            /* Memory barrier to prevent fusion */
            asm volatile("" : : : "memory");
        }
        
        /* Scramble data between outer iterations */
        for (volatile int j = 0; j < ARRAY_SIZE/4; j++) {
            volatile int temp = a[j];
            a[j] = b[j];
            b[j] = c[j];
            c[j] = d[j];
            d[j] = temp;
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate and initialize arrays with volatile */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    for (volatile int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
        array4[i] = rand() % 256;
    }
    
    /* Call the high pressure loop multiple times */
    volatile int total = 0;
    for (volatile int run = 0; run < 10; run++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify data between runs */
        for (volatile int i = 0; i < ARRAY_SIZE; i++) {
            array1[i] = (array1[i] * 13 + 17) & 0xFF;
            array2[i] = (array2[i] * 29 + 41) & 0xFF;
            array3[i] = (array3[i] * 37 + 53) & 0xFF;
            array4[i] = (array4[i] * 61 + 73) & 0xFF;
        }
    }
    
    return total;
}

int main(void) {
    /* Seed RNG for reproducibility */
    srand(42);
    
    /* Run the computation */
    volatile int checksum = setup_and_run();
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
