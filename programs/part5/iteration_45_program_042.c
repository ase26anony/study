#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = 10; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (volatile int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* Immediate constants that are rematerialization candidates */
                volatile int const1 = 1;      /* Candidate for remat */
                volatile int const2 = 2;      /* Candidate for remat */
                volatile int const3 = 4;      /* Candidate for remat */
                volatile int const7 = 7;      /* Candidate for remat */
                volatile int const11 = 11;    /* Candidate for remat */
                
                /* Variables with different widths to create partial register dependencies */
                volatile char vc1, vc2;
                volatile short vs1, vs2;
                volatile int vi1, vi2, vi3, vi4, vi5, vi6, vi7, vi8, vi9, vi10;
                volatile long vl1, vl2;
                
                /* Complex address arithmetic with loop-invariant base + offset */
                /* This creates REG RTX references for rematerialization */
                volatile int *addr1 = a + i;
                volatile int *addr2 = b + i;
                volatile int *addr3 = c + i;
                volatile int *addr4 = d + i;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
                
                /* Load data with address computation - creates REG references */
                vi1 = *addr1 + const1;        /* REG + immediate */
                vi2 = *addr2 * const2;        /* REG * immediate */
                vi3 = *addr3 & const3;        /* REG & immediate */
                vi4 = *addr4 | const7;        /* REG | immediate */
                
                /* Chain of dependent arithmetic operations */
                vi5 = vi1 + vi2;
                vi6 = vi3 - vi4;
                vi7 = vi5 * vi6;
                vi8 = vi7 + const11;          /* Another immediate constant */
                
                /* More operations with different data widths */
                vc1 = (char)(vi8 & 0xFF);
                vs1 = (short)(vi8 >> 8);
                vi9 = vi8 * vc1;
                vi10 = vi9 + vs1;
                
                /* Conditional branches create multiple basic blocks */
                if (vi10 & 0x1) {             /* Volatile check */
                    vl1 = (long)vi10 * const7;
                    vi2 = vi2 + const1;       /* Reuse vi2 with immediate */
                } else {
                    vl1 = (long)vi10 / const2;
                    vi3 = vi3 - const1;       /* Reuse vi3 with immediate */
                }
                
                /* Another conditional with immediate constant */
                if (vi8 > const11) {
                    vs2 = (short)(vi8 - const11);
                    vi4 = vi4 * const2;
                } else {
                    vs2 = (short)(const11 - vi8);
                    vi1 = vi1 / const2;
                }
                
                /* More arithmetic with overlapping live ranges */
                vl2 = vl1 + (long)vs2;
                vc2 = (char)(vl2 & 0xFF);
                
                /* Final computation mixing all variables */
                result += (int)(vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + 
                               vi7 + vi8 + vi9 + vi10 + vc1 + vc2 + 
                               vs1 + vs2 + vl1 + vl2);
                
                /* Another memory barrier */
                asm volatile("" : : : "memory");
            }
        }
        
        /* Modify volatile bound to prevent loop unrolling */
        if (outer == 5) {
            asm volatile("" : : : "memory");
            outer_bound = outer_bound + 0; /* No-op but volatile prevents optimization */
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large arrays to create memory pressure */
    static volatile int array1[ARRAY_SIZE];
    static volatile int array2[ARRAY_SIZE];
    static volatile int array3[ARRAY_SIZE];
    static volatile int array4[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        array4[i] = rand() % 100;
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        array1[iter % ARRAY_SIZE] = rand() % 100;
    }
    
    return total;
}

int main(void) {
    srand(42); /* Deterministic seed for reproducibility */
    
    volatile int checksum = setup_and_run();
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional computation to keep registers live */
    volatile int final = 0;
    for (int i = 0; i < 1000; i++) {
        final += checksum * i;
    }
    
    printf("Final: %d\n", final);
    
    return 0;
}
