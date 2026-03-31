#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int i = 0; i < bound; i++) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Use at least 15 different integer variables with complex dependencies */
        volatile int v1 = arr1[i] & 0xFF;          /* Candidate for remat: 0xFF constant */
        volatile int v2 = v1 + arr2[i];            /* Use v1 immediately */
        volatile int v3 = v2 * 2;                  /* Candidate for remat: *2 operation */
        volatile int v4 = arr3[i] | 0x55;          /* Candidate: 0x55 constant */
        volatile int v5 = v3 - v4;
        volatile int v6 = v5 + 1;                  /* Candidate: +1 constant */
        volatile int v7 = v6 & arr1[(i + 1) % SIZE];
        volatile int v8 = v7 * 3;                  /* Candidate: *3 constant */
        volatile int v9 = v8 | v2;
        volatile int v10 = v9 - 5;                 /* Candidate: -5 constant */
        volatile int v11 = v10 + arr2[(i + 2) % SIZE];
        volatile int v12 = v11 * 7;                /* Candidate: *7 constant */
        volatile int v13 = v12 & 0xAA;             /* Candidate: 0xAA constant */
        volatile int v14 = v13 + v4;
        volatile int v15 = v14 - 9;                /* Candidate: -9 constant */
        
        /* Create multiple basic blocks with conditional branches */
        if (v1 > 128) {
            /* Different computation path with more constants */
            v3 = v1 * 4;                           /* Candidate: *4 constant */
            v5 = v3 + 8;                           /* Candidate: +8 constant */
            v7 = v5 & 0xCC;                        /* Candidate: 0xCC constant */
            asm volatile("" : : : "memory");       /* Barrier in conditional path */
        } else {
            /* Alternative path with different constants */
            v3 = v1 * 6;                           /* Candidate: *6 constant */
            v5 = v3 - 3;                           /* Candidate: -3 constant */
            v7 = v5 | 0x33;                        /* Candidate: 0x33 constant */
        }
        
        /* More computations creating register pressure */
        volatile int w1 = v15 + v7;
        volatile int w2 = w1 * 11;                 /* Candidate: *11 constant */
        volatile int w3 = w2 & arr3[(i + 3) % SIZE];
        volatile int w4 = w3 + 13;                 /* Candidate: +13 constant */
        volatile int w5 = w4 - v10;
        volatile int w6 = w5 * 17;                 /* Candidate: *17 constant */
        
        /* Use different data types to create partial register dependencies */
        volatile char c1 = (char)(w6 & 0xFF);
        volatile short s1 = (short)(w6 >> 8);
        volatile long l1 = (long)w6 * 19L;         /* Candidate: 19L constant */
        
        /* Complex address computation with loop-invariant components */
        /* This creates remat candidates for address calculations */
        int idx = (i * 2 + 1) % SIZE;             /* Candidate: *2, +1 constants */
        volatile int addr_calc = arr1[idx] + arr2[idx * 3 % SIZE]; /* *3 constant */
        
        /* Mix all results with more constants */
        result += (int)(l1 >> 4) + c1 + s1 + addr_calc + w6;
        
        /* Another conditional to split basic blocks further */
        if (v2 < 64) {
            volatile int extra1 = result * 2;      /* Candidate: *2 constant */
            volatile int extra2 = extra1 + 21;     /* Candidate: +21 constant */
            result = extra2 & 0x7FFFFFFF;
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(volatile int *a, volatile int *b, volatile int *c) {
    volatile int sum = 0;
    volatile int outer_bound = 50;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Vary the bound slightly to prevent loop unrolling */
        volatile int inner_bound = ITERATIONS + (outer % 3);
        
        /* Call the high pressure function multiple times */
        sum += high_pressure_loop(a, b, c, inner_bound);
        
        /* Modify arrays slightly to prevent complete optimization */
        a[outer % SIZE] = sum & 0xFF;
        b[outer % SIZE] = (sum >> 8) & 0xFF;
        c[outer % SIZE] = (sum >> 16) & 0xFF;
        
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    static volatile int array1[SIZE];
    static volatile int array2[SIZE];
    static volatile int array3[SIZE];
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
    }
    
    /* Memory barrier before computation */
    asm volatile("" : : : "memory");
    
    /* Run the high register pressure computation */
    volatile int checksum = setup_and_run(array1, array2, array3);
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
