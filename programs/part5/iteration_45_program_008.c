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
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables with different types to create partial reg dependencies */
            volatile char v1;
            volatile short v2;
            volatile int v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
            volatile long v16, v17;
            
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
            
            /* Chain of dependent computations using immediate constants */
            v1 = (char)(a[i] & 0xFF);                    /* Candidate for remat: mask constant */
            v2 = (short)(v1 + 1);                        /* Candidate: +1 constant */
            v3 = v2 * 2;                                 /* Candidate: *2 constant */
            v4 = v3 | 0x7F;                              /* Candidate: bitmask constant */
            v5 = v4 - b[i];
            v6 = v5 & 0x3F;                              /* Candidate: mask constant */
            v7 = v6 + c[i];
            v8 = v7 * 3;                                 /* Candidate: *3 constant */
            v9 = v8 ^ 0x55;                              /* Candidate: XOR constant */
            v10 = v9 + d[i];
            v11 = v10 >> 2;                              /* Candidate: shift constant */
            v12 = v11 * 5;                               /* Candidate: *5 constant */
            v13 = v12 + 0x1000;                          /* Candidate: large constant */
            v14 = v13 & 0xFFF;                           /* Candidate: mask constant */
            v15 = v14 - 4096;                            /* Candidate: subtraction constant */
            v16 = (long)v15 * 7L;                        /* Candidate: *7 constant */
            v17 = v16 | 0x80000000L;                     /* Candidate: bitmask constant */
            
            /* Complex conditional with multiple basic blocks */
            if (v17 & 1) {
                /* Use address arithmetic with loop-invariant base + immediate offset */
                volatile int *ptr = &a[i];
                v3 = *(ptr + 1);                         /* Candidate: address computation */
                v4 = v3 + (int)(ptr - a);                /* Candidate: pointer difference */
            } else {
                v4 = v17 & 0x7FFFFFFF;
            }
            
            /* Another conditional block */
            if (v4 > 1000) {
                v5 = v4 * 9;                             /* Candidate: *9 constant */
                v6 = v5 + 0x2000;                        /* Candidate: large constant */
            } else {
                v6 = v4 / 2;                             /* Candidate: division by 2 */
            }
            
            /* More computations with immediate constants */
            v7 = v6 + 42;                                /* Candidate: +42 constant */
            v8 = v7 * 11;                                /* Candidate: *11 constant */
            v9 = v8 & 0x3FF;                             /* Candidate: mask constant */
            v10 = v9 ^ 0x1FF;                            /* Candidate: XOR constant */
            v11 = v10 << 3;                              /* Candidate: shift constant */
            v12 = v11 + 0x800;                           /* Candidate: large constant */
            v13 = v12 - 2048;                            /* Candidate: subtraction constant */
            v14 = v13 | 0x400;                           /* Candidate: bitmask constant */
            v15 = v14 * 13;                              /* Candidate: *13 constant */
            
            /* Artificial dependency to keep all values live */
            result ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 
                    ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ (int)v16 ^ (int)v17;
            
            /* Another memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Modify array elements to prevent loop-invariant code motion */
        a[outer % ARRAY_SIZE] ^= result;
        b[outer % ARRAY_SIZE] += outer;
    }
    
    return result;
}

/* Separate function to create more register pressure in caller-save context */
__attribute__((noinline, noipa))
static volatile int process_results(volatile int x, volatile int y, 
                                   volatile int z, volatile int w) {
    volatile int t1 = x * 17;      /* Candidate: *17 constant */
    volatile int t2 = y + 0x1234;  /* Candidate: large constant */
    volatile int t3 = z & 0xABCD;  /* Candidate: mask constant */
    volatile int t4 = w ^ 0xDEAD;  /* Candidate: XOR constant */
    volatile int t5 = t1 | t2;
    volatile int t6 = t3 ^ t4;
    volatile int t7 = t5 + t6;
    volatile int t8 = t7 * 19;     /* Candidate: *19 constant */
    volatile int t9 = t8 - 0x1000; /* Candidate: subtraction constant */
    volatile int t10 = t9 >> 4;    /* Candidate: shift constant */
    
    asm volatile ("" : : : "memory");
    return t10;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
        array4[i] = rand();
    }
    
    /* Create register pressure in main as well */
    volatile int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    volatile int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    
    for (int j = 0; j < 100; j++) {
        tmp1 = array1[j % ARRAY_SIZE] + j;
        tmp2 = array2[j % ARRAY_SIZE] * 2;      /* Candidate: *2 constant */
        tmp3 = array3[j % ARRAY_SIZE] & 0xFF;   /* Candidate: mask constant */
        tmp4 = array4[j % ARRAY_SIZE] ^ 0xAA;   /* Candidate: XOR constant */
        tmp5 = tmp1 + tmp2;
        tmp6 = tmp3 | tmp4;
        tmp7 = tmp5 * tmp6;
        tmp8 = tmp7 + 0x100;                    /* Candidate: large constant */
        
        acc1 ^= tmp1;
        acc2 += tmp2;
        acc3 |= tmp3;
        acc4 &= tmp4;
    }
    
    /* Call the high-pressure function */
    volatile int result = high_pressure_loop(array1, array2, array3, array4);
    
    /* Process results with more constants */
    volatile int final = process_results(result, acc1, acc2, acc3);
    
    printf("Checksum: %d\n", final ^ acc4);
    return 0;
}
