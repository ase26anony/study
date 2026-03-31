#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define UNROLL_FACTOR 8

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int n) {
    volatile int result = 0;
    volatile int outer_bound = n % 256 + 100; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Declare many variables in nested scope to create register pressure */
        {
            volatile int v1 = arr1[outer % ARRAY_SIZE];
            volatile int v2 = arr2[outer % ARRAY_SIZE];
            volatile int v3 = arr3[outer % ARRAY_SIZE];
            
            /* Immediate constants that are rematerialization candidates */
            volatile int c1 = 1;    /* Candidate for remat */
            volatile int c2 = 2;    /* Candidate for remat */
            volatile int c4 = 4;    /* Candidate for remat */
            volatile int c8 = 8;    /* Candidate for remat */
            volatile int c16 = 16;  /* Candidate for remat */
            
            /* Complex interdependent calculations with different data types */
            volatile char  ch1 = (v1 & 0xFF);
            volatile short sh1 = (v2 & 0xFFFF);
            volatile long  lg1 = v3;
            
            /* Chain of dependent operations creating long live ranges */
            volatile int t1 = v1 + c1;      /* Uses immediate constant */
            volatile int t2 = t1 * c2;      /* Uses immediate constant */
            volatile int t3 = t2 - v2;
            volatile int t4 = t3 & c4;      /* Uses immediate constant */
            volatile int t5 = t4 | sh1;
            volatile int t6 = t5 ^ ch1;
            volatile int t7 = t6 << (c8 >> 3);  /* Complex immediate expression */
            volatile int t8 = t7 >> (c16 >> 4); /* Another immediate expression */
            
            /* Address computation with loop-invariant base - prime for remat */
            volatile int *base_ptr = arr1;
            volatile int idx = outer * 2;
            volatile int *addr1 = base_ptr + idx;  /* base_ptr is remat candidate */
            volatile int *addr2 = base_ptr + idx + c1; /* Another remat candidate */
            
            /* More variables to increase pressure */
            volatile int u1 = *addr1;
            volatile int u2 = *addr2;
            volatile int u3 = u1 + u2;
            volatile int u4 = u3 * t8;
            volatile int u5 = u4 & lg1;
            
            /* Conditional branches creating multiple basic blocks */
            if (u5 & c1) {  /* Immediate constant in condition */
                volatile int w1 = u5 + c2;
                volatile int w2 = w1 * c4;
                result += w2;
            } else {
                volatile int w3 = u5 - c8;
                volatile int w4 = w3 / c16;
                result -= w4;
            }
            
            /* Nested conditional with more register usage */
            if (t4 > 0) {
                volatile int z1 = t4 * 3;  /* Immediate constant */
                volatile int z2 = z1 + 7;  /* Immediate constant */
                volatile int z3 = z2 - 11; /* Immediate constant */
                result ^= z3;
            }
            
            /* Use all variables to keep them live */
            asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), 
                         "r"(ch1), "r"(sh1), "r"(lg1),
                         "r"(t1), "r"(t2), "r"(t3), "r"(t4),
                         "r"(t5), "r"(t6), "r"(t7), "r"(t8),
                         "r"(u1), "r"(u2), "r"(u3), "r"(u4), "r"(u5) : "memory");
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more call context */
__attribute__((noinline, noipa))
static volatile int process_arrays(volatile int *a, volatile int *b, volatile int *c, int size) {
    volatile int total = 0;
    
    /* Inner loop with manual unrolling to create more register pressure */
    for (int i = 0; i < size; i += UNROLL_FACTOR) {
        volatile int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
        volatile int sum5 = 0, sum6 = 0, sum7 = 0, sum8 = 0;
        
        /* Unrolled computations with immediate constants */
        sum1 = a[i] + 1;      /* Immediate constant */
        sum2 = b[i+1] * 2;    /* Immediate constant */
        sum3 = c[i+2] & 4;    /* Immediate constant */
        sum4 = a[i+3] | 8;    /* Immediate constant */
        sum5 = b[i+4] ^ 16;   /* Immediate constant */
        sum6 = c[i+5] + 32;   /* Immediate constant */
        sum7 = a[i+6] - 64;   /* Immediate constant */
        sum8 = b[i+7] * 128;  /* Immediate constant */
        
        /* Complex dependency chain */
        volatile int tmp1 = sum1 + sum2;
        volatile int tmp2 = tmp1 * sum3;
        volatile int tmp3 = tmp2 - sum4;
        volatile int tmp4 = tmp3 & sum5;
        volatile int tmp5 = tmp4 | sum6;
        volatile int tmp6 = tmp5 ^ sum7;
        volatile int tmp7 = tmp6 + sum8;
        
        total += tmp7;
        
        /* Force spill/reload behavior with volatile */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

int main(void) {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Create extreme register pressure scenario */
    volatile int result1 = high_pressure_loop(array1, array2, array3, ARRAY_SIZE);
    
    /* Additional processing to increase overall pressure */
    volatile int result2 = process_arrays(array1, array2, array3, ARRAY_SIZE);
    
    volatile int final_result = result1 ^ result2;
    
    printf("Checksum: %d\n", final_result);
    
    return 0;
}
