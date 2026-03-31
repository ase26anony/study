#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = ITERATIONS;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18;
        
        /* Force register pressure with many live variables */
        v1 = a[outer % ARRAY_SIZE];
        v2 = b[outer % ARRAY_SIZE];
        v3 = c[outer % ARRAY_SIZE];
        v4 = d[outer % ARRAY_SIZE];
        
        /* Complex interdependent calculations with immediate constants */
        /* These constants are rematerialization candidates */
        v5 = v1 + 1;          /* Candidate for remat: constant 1 */
        v6 = v2 * 2;          /* Candidate for remat: constant 2 */
        v7 = v3 & 0xFF;       /* Candidate for remat: constant 0xFF */
        v8 = v4 | 0x80;       /* Candidate for remat: constant 0x80 */
        v9 = v5 - 3;          /* Candidate for remat: constant 3 */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* More calculations creating long live ranges */
        v10 = v6 + v7;
        v11 = v8 - v9;
        v12 = v10 * v11;
        
        /* Conditional branch creating multiple basic blocks */
        if (v12 & 1) {
            /* Different width operations for partial register dependencies */
            volatile char c1 = (char)v1;
            volatile short s1 = (short)v2;
            volatile long l1 = (long)v3;
            
            v13 = c1 + s1 + l1;
            v14 = v13 * 4;    /* Candidate for remat: constant 4 */
            
            /* Nested conditionals */
            if (v14 > 1000) {
                v15 = v14 / 2;  /* Candidate for remat: constant 2 */
            } else {
                v15 = v14 * 3;  /* Candidate for remat: constant 3 */
            }
        } else {
            volatile char c2 = (char)v4;
            volatile short s2 = (short)v5;
            volatile long l2 = (long)v6;
            
            v13 = c2 - s2 + l2;
            v14 = v13 << 1;   /* Candidate for remat: constant 1 (shift) */
            
            if (v14 < 500) {
                v15 = v14 | 0x7F;  /* Candidate for remat: constant 0x7F */
            } else {
                v15 = v14 & 0x3F;  /* Candidate for remat: constant 0x3F */
            }
        }
        
        /* More calculations keeping variables live */
        v16 = v12 + v15;
        v17 = v16 ^ 0x55;     /* Candidate for remat: constant 0x55 */
        v18 = v17 % 7;        /* Candidate for remat: constant 7 */
        
        /* Address computation with loop-invariant base - prime remat candidate */
        int idx = (outer * 17) % ARRAY_SIZE;  /* 17 is prime, prevents optimization */
        volatile int *ptr = &a[idx];
        v18 += *ptr;
        
        /* Final accumulation with memory barrier */
        asm volatile("" : : : "memory");
        result += v18;
        
        /* Force spill/reload by using all variables again */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large volatile arrays to prevent optimization */
    static volatile int array1[ARRAY_SIZE];
    static volatile int array2[ARRAY_SIZE];
    static volatile int array3[ARRAY_SIZE];
    static volatile int array4[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
        array4[i] = rand() % 1000;
    }
    
    /* Call the high-pressure loop multiple times */
    volatile int total = 0;
    for (int run = 0; run < 5; run++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array1[i] += run;
            array2[i] -= run;
        }
    }
    
    return total;
}

int main(void) {
    srand(42);  /* Fixed seed for reproducibility */
    
    volatile int checksum = setup_and_run();
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional runs to ensure ER sees enough pressure */
    for (int extra = 0; extra < 3; extra++) {
        checksum += setup_and_run();
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
