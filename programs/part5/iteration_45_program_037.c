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
    volatile int outer_bound = 100; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* Immediate constants - candidates for rematerialization */
                const int IMM1 = 1;      /* Candidate for remat */
                const int IMM2 = 2;      /* Candidate for remat */
                const int IMM3 = 3;      /* Candidate for remat */
                const int IMM4 = 4;      /* Candidate for remat */
                const int IMM5 = 5;      /* Candidate for remat */
                
                /* Variables with different widths to create partial register dependencies */
                volatile char vc1, vc2;
                volatile short vs1, vs2;
                volatile int vi1, vi2, vi3, vi4, vi5, vi6, vi7, vi8, vi9, vi10;
                volatile long vl1, vl2;
                
                /* Complex interdependent calculations with immediate constants */
                vi1 = a[i] + IMM1;           /* Uses immediate - remat candidate */
                vi2 = b[i] - IMM2;           /* Uses immediate - remat candidate */
                vi3 = vi1 * vi2;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
                
                /* More calculations with different operations */
                vi4 = vi3 & 0xFF;            /* Bitwise with constant */
                vi5 = vi4 | IMM3;            /* OR with immediate */
                vi6 = vi5 << 2;              /* Shift with constant */
                
                /* Conditional branch creating basic block boundary */
                if (vi6 > 1000) {
                    vi7 = vi6 / IMM4;        /* Division with immediate */
                    vi8 = vi7 + c[i];
                } else {
                    vi7 = vi6 * IMM5;        /* Multiplication with immediate */
                    vi8 = vi7 - d[i];
                }
                
                /* More variables to increase pressure */
                vi9 = vi8 ^ 0xAAAA;          /* XOR with constant */
                vi10 = vi9 % 17;             /* Modulo with constant */
                
                /* Mix different data widths */
                vc1 = (char)(vi10 & 0xFF);
                vs1 = (short)(vi10 & 0xFFFF);
                vl1 = (long)vi10 * 100L;
                
                /* Address computation with loop-invariant base - remat candidate */
                int offset = i * sizeof(int);
                volatile int *ptr1 = a + offset / sizeof(int);
                volatile int *ptr2 = b + offset / sizeof(int);
                
                /* More calculations using pointers */
                vi2 = *ptr1 + *ptr2;         /* Uses address computation */
                vi3 = vi2 + (int)ptr1;       /* Mix pointer with integer */
                
                /* Another memory barrier */
                asm volatile("" : : : "memory");
                
                /* Final chain of dependent operations */
                vc2 = vc1 + 1;
                vs2 = vs1 - 2;
                vl2 = vl1 * 3;
                
                /* Use all variables to keep them live */
                result += vi1 + vi2 + vi3 + vi4 + vi5 + vi6 + vi7 + vi8 + vi9 + vi10;
                result += vc1 + vc2 + vs1 + vs2 + vl1 + vl2;
                
                /* Artificial dependency to prevent dead code elimination */
                a[i] = result & 1;
                b[i] = result & 2;
            }
            
            /* Additional scope with more variables */
            {
                volatile int extra1, extra2, extra3, extra4, extra5;
                volatile long extra6, extra7;
                
                extra1 = a[i] ^ b[i];
                extra2 = extra1 * 7;         /* Multiplication with constant */
                extra3 = extra2 >> 3;        /* Shift with constant */
                extra4 = extra3 & 0x7F;      /* AND with constant */
                extra5 = extra4 | 0x80;      /* OR with constant */
                extra6 = (long)extra5 * 11L;
                extra7 = extra6 / 13L;
                
                result += extra1 + extra2 + extra3 + extra4 + extra5 + extra6 + extra7;
                
                /* Force spill/reload behavior */
                for (volatile int j = 0; j < 2; j++) {
                    c[i] = extra1 + j;
                    d[i] = extra2 - j;
                }
            }
        }
        
        /* Modify outer bound to prevent loop unrolling */
        if (outer % 10 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
        array4[i] = rand() % 1000;
    }
    
    /* Execute high pressure loop */
    volatile int checksum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        checksum += high_pressure_loop(array1, array2, array3, array4);
        
        /* Occasionally modify arrays to prevent optimization */
        if (iter % 1000 == 0) {
            array1[iter % ARRAY_SIZE] = rand() % 1000;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
