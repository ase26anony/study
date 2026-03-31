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
    volatile int outer_bound = 10; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                volatile int v1 = a[i] + outer;      /* Candidate for remat: outer + immediate */
                volatile int v2 = b[i] * 2;          /* Candidate: multiplication by immediate 2 */
                volatile int v3 = c[i] & 0xFF;       /* Candidate: bitmask immediate */
                volatile int v4 = d[i] | 0x80;       /* Candidate: bitwise OR with immediate */
                volatile int v5 = v1 + 1;            /* Candidate: addition with immediate 1 */
                volatile int v6 = v2 - 1;            /* Candidate: subtraction with immediate 1 */
                volatile int v7 = v3 << 2;           /* Candidate: shift by immediate 2 */
                volatile int v8 = v4 >> 1;           /* Candidate: shift by immediate 1 */
                volatile int v9 = v5 * 3;            /* Candidate: multiplication by immediate 3 */
                volatile int v10 = v6 / 2;           /* Candidate: division by immediate 2 */
                volatile int v11 = v7 % 5;           /* Candidate: modulo with immediate 5 */
                volatile int v12 = v8 ^ 0x7F;        /* Candidate: XOR with immediate */
                volatile int v13 = v9 + v10;
                volatile int v14 = v11 * v12;
                volatile int v15 = v13 & v14;
                
                /* Complex conditional with multiple basic blocks */
                if (v1 & 1) {
                    /* Use different width operations to create partial register dependencies */
                    volatile char c1 = (v2 & 0xFF);
                    volatile short s1 = (v3 & 0xFFFF);
                    volatile long l1 = v4 * v5;
                    
                    /* Memory barrier to prevent reordering */
                    asm volatile ("" : : : "memory");
                    
                    v15 += c1 + s1 + l1;
                    
                    /* Nested conditional for more basic blocks */
                    if (v6 > 1000) {
                        volatile int v16 = v7 * 4;   /* Another immediate constant */
                        volatile int v17 = v8 + 8;   /* Another immediate constant */
                        v15 += v16 - v17;
                    }
                } else {
                    /* Alternative path with different computations */
                    volatile int v18 = v9 / 4;       /* Division by immediate */
                    volatile int v19 = v10 * 8;      /* Multiplication by immediate */
                    v15 -= v18 + v19;
                }
                
                /* More arithmetic with immediate constants */
                volatile int v20 = v15 + 16;         /* Candidate for remat */
                volatile int v21 = v20 * 32;         /* Candidate for remat */
                volatile int v22 = v21 & 0x3F;       /* Candidate for remat */
                
                /* Final computation with result accumulation */
                result += v22;
                
                /* Force register pressure by using all variables again */
                asm volatile ("" 
                    : "+r" (v1), "+r" (v2), "+r" (v3), "+r" (v4), "+r" (v5),
                      "+r" (v6), "+r" (v7), "+r" (v8), "+r" (v9), "+r" (v10),
                      "+r" (v11), "+r" (v12), "+r" (v13), "+r" (v14), "+r" (v15)
                    : 
                    : "memory");
            }
        }
        
        /* Modify arrays to prevent loop-invariant code motion */
        asm volatile ("" : : "r" (a), "r" (b), "r" (c), "r" (d) : "memory");
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large volatile arrays to force memory operations */
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
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly each iteration */
        array1[iter % ARRAY_SIZE] = rand() % 1000;
        array2[iter % ARRAY_SIZE] = rand() % 1000;
    }
    
    return total;
}

int main(void) {
    /* Seed random number generator */
    srand(42);
    
    /* Run the computation */
    volatile int result = setup_and_run();
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
