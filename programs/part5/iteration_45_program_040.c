#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                       volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = 10; /* volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (volatile int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* Immediate constants that are rematerialization candidates */
                volatile int c1 = 1;   /* Candidate for remat */
                volatile int c2 = 2;   /* Candidate for remat */
                volatile int c3 = 4;   /* Candidate for remat */
                volatile int c4 = 8;   /* Candidate for remat */
                volatile int c5 = 16;  /* Candidate for remat */
                
                /* Variables with complex dependencies */
                volatile int v1 = a[i] + c1;      /* Uses immediate constant */
                volatile int v2 = b[i] * c2;      /* Uses immediate constant */
                volatile int v3 = v1 & v2;
                volatile int v4 = c[i] | c3;      /* Uses immediate constant */
                volatile int v5 = d[i] ^ c4;      /* Uses immediate constant */
                volatile int v6 = v3 + v4;
                volatile int v7 = v5 - v6;
                volatile int v8 = v7 * c5;        /* Uses immediate constant */
                volatile int v9 = v8 >> 2;
                volatile int v10 = v9 << 1;
                volatile int v11 = v10 % 17;
                volatile int v12 = v11 * 3;
                volatile int v13 = v12 + 7;
                volatile int v14 = v13 - 5;
                volatile int v15 = v14 & 0xFF;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
                
                /* Conditional branch creating multiple basic blocks */
                if (v15 & 1) {
                    /* More arithmetic with different widths */
                    volatile char c16 = (v15 >> 1) & 0xFF;
                    volatile short s17 = c16 * 2;
                    volatile int v18 = s17 + 256;
                    volatile long v19 = v18 * 3L;
                    
                    result += v19 & 0xFFFF;
                    
                    /* Another memory barrier */
                    asm volatile("" : : : "memory");
                    
                    /* Nested conditional for more complexity */
                    if (v19 > 1000) {
                        volatile int v20 = v19 >> 4;
                        volatile int v21 = v20 * 9;
                        result ^= v21;
                    }
                } else {
                    /* Alternative path with different computations */
                    volatile int v22 = v15 * 5;
                    volatile int v23 = v22 + 11;
                    volatile int v24 = v23 / 3;
                    result |= v24;
                }
                
                /* More computations to extend live ranges */
                volatile int v25 = result + i;
                volatile int v26 = v25 * outer;
                volatile int v27 = v26 & 0x7FFF;
                result = v27;
                
                /* Address arithmetic with loop-invariant components */
                /* This creates remat candidates for address calculations */
                volatile int *ptr1 = a + (i & 0x3F);      /* Base + offset */
                volatile int *ptr2 = b + (i >> 2);        /* Different offset */
                volatile int idx = *ptr1 + *ptr2;
                
                /* Use idx in computation */
                volatile int v28 = idx * 2;               /* Immediate constant */
                volatile int v29 = v28 + 1;               /* Immediate constant */
                result += v29;
            }
        }
        
        /* Modify array elements to create dependencies across iterations */
        a[outer % ARRAY_SIZE] = result;
        b[outer % ARRAY_SIZE] = result ^ 0x1234;
    }
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
static volatile int secondary_pressure(volatile int *arr, volatile int count) {
    volatile int sum = 0;
    
    for (volatile int i = 0; i < count; i++) {
        /* Many variables with arithmetic on immediate constants */
        volatile int x1 = arr[i] + 1;
        volatile int x2 = x1 * 2;
        volatile int x3 = x2 - 3;
        volatile int x4 = x3 & 0xF;
        volatile int x5 = x4 | 0x10;
        volatile int x6 = x5 ^ 0x20;
        volatile int x7 = x6 << 1;
        volatile int x8 = x7 >> 2;
        volatile int x9 = x8 + 4;
        volatile int x10 = x9 * 5;
        volatile int x11 = x10 % 7;
        volatile int x12 = x11 + 8;
        volatile int x13 = x12 - 9;
        volatile int x14 = x13 & 0xFF;
        volatile int x15 = x14 | 0x100;
        
        sum += x15;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
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
    
    /* Create register pressure */
    volatile int result1 = high_pressure_loop(array1, array2, array3, array4);
    
    /* Additional pressure with different pattern */
    volatile int temp[100];
    for (int i = 0; i < 100; i++) {
        temp[i] = i * 3;
    }
    volatile int result2 = secondary_pressure(temp, 100);
    
    /* Final computation mixing results */
    volatile int final_result = result1 ^ result2;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
