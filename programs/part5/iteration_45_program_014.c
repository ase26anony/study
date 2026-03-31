#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 50; /* volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Declare many variables in nested scope to create register pressure */
        {
            volatile int v1 = arr1[outer % ARRAY_SIZE];
            volatile int v2 = arr2[outer % ARRAY_SIZE];
            volatile int v3 = arr3[outer % ARRAY_SIZE];
            volatile int v4 = arr4[outer % ARRAY_SIZE];
            
            /* Immediate constants - candidates for rematerialization */
            const int c1 = 1;      /* Will likely become REG with constant value */
            const int c2 = 2;
            const int c3 = 3;
            const int c4 = 4;
            const int c5 = 5;
            
            /* More variables with different types to create partial reg dependencies */
            volatile char c_val1, c_val2;
            volatile short s_val1, s_val2;
            volatile long l_val1, l_val2;
            
            /* Complex interdependent calculations */
            v1 = v1 + c1;          /* Uses immediate constant - remat candidate */
            v2 = v2 * c2;          /* Another immediate constant use */
            v3 = v3 - c3;
            v4 = v4 | c4;
            
            /* Create data dependencies */
            int t1 = v1 + v2;
            int t2 = v3 - v4;
            int t3 = t1 * t2;
            int t4 = t3 & 0xFF;
            int t5 = t4 | 0x55;
            
            /* Mix operations with different widths */
            c_val1 = (char)t5;
            s_val1 = (short)(t5 + c5);  /* Another immediate constant */
            l_val1 = (long)(t5 * 2);    /* Constant multiplication candidate */
            
            /* Conditional branches create multiple basic blocks */
            if (v1 & 1) {
                t1 = t1 + c1;           /* Reuse immediate in different block */
                c_val2 = (char)(t1 % 256);
                asm volatile("" : : : "memory"); /* Barrier */
            } else {
                t2 = t2 - c2;           /* Same constant, different block */
                s_val2 = (short)(t2 % 65536);
            }
            
            /* More arithmetic with constants */
            if (v2 > 100) {
                t3 = t3 / c3;
                l_val2 = (long)(t3 * 3);  /* Another constant */
            }
            
            /* Address computation with loop-invariant base - remat candidate */
            int *base_ptr = (int*)arr1;
            volatile int idx = outer % 256;
            int addr_calc = (int)(base_ptr + idx);  /* Base + offset computation */
            
            /* Use the computed address */
            if (addr_calc & 1) {
                t4 = t4 ^ c4;
            }
            
            /* Final computation using all variables */
            result += t1 + t2 + t3 + t4 + t5 + c_val1 + s_val1 + l_val1 + 
                     c_val2 + s_val2 + l_val2 + addr_calc;
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Inner loop with more register pressure */
        for (volatile int inner = 0; inner < 10; inner++) {
            /* More variables in inner scope */
            volatile int w1 = result;
            volatile int w2 = outer;
            volatile int w3 = inner;
            
            /* More immediate constant uses */
            w1 = w1 + 7;      /* Different constant */
            w2 = w2 * 8;
            w3 = w3 - 9;
            
            /* Complex expression that might be rematerialized */
            int expr1 = (w1 << 2) + (w2 >> 1);  /* Shift by constants */
            int expr2 = (w3 & 0xF) | 0x10;      /* Bitmask constants */
            
            result = expr1 + expr2;
            
            /* Conditional with constant */
            if (inner % 3 == 0) {  /* Modulo with constant */
                result += 11;       /* Another constant */
            }
        }
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
        array4[i] = rand() % 1000;
    }
    
    /* Call the high-pressure function many times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        if (iter % 100 == 0) {
            array1[iter % ARRAY_SIZE] = rand() % 1000;
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
