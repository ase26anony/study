#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                       volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    volatile int i, j;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (i = 0; i < bound; i++) {
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Force register pressure with immediate constants (remat candidates) */
        v1 = arr1[i] + 1;          /* Immediate constant +1 */
        v2 = arr2[i] * 2;          /* Immediate constant *2 */
        v3 = v1 & 0xFF;            /* Immediate constant mask */
        v4 = v2 | 0x80;            /* Immediate constant OR */
        v5 = v3 - 7;               /* Immediate constant -7 */
        
        /* Complex interdependent calculations */
        v6 = v4 + v5;
        v7 = v6 * v1;
        v8 = v7 >> 3;              /* Immediate constant shift */
        v9 = v8 & v2;
        v10 = v9 | v3;
        
        /* More calculations with different data widths */
        volatile char c1 = (char)v10;
        volatile short s1 = (short)v10;
        v11 = c1 * s1 + 15;        /* Immediate constant +15 */
        
        /* Conditional branch creating multiple basic blocks */
        if (v11 > 1000) {
            v12 = v11 * 3;         /* Immediate constant *3 */
            v13 = v12 + arr3[i];
            v14 = v13 & 0x7F;      /* Immediate constant mask */
        } else {
            v12 = v11 / 4;         /* Immediate constant /4 */
            v13 = v12 - arr3[i];
            v14 = v13 | 0x3F;      /* Immediate constant OR */
        }
        
        /* Nested loop with more variables */
        for (j = 0; j < 5; j++) {
            volatile int w1, w2, w3, w4, w5;
            
            w1 = v14 + j * 8;      /* Immediate constant *8 */
            w2 = w1 & 0x1F;        /* Immediate constant mask */
            w3 = w2 * 5;           /* Immediate constant *5 */
            w4 = w3 + 17;          /* Immediate constant +17 */
            w5 = w4 - 9;           /* Immediate constant -9 */
            
            /* Memory barrier to prevent optimization */
            asm volatile("" : : : "memory");
            
            v15 = w5 + v14;
            v16 = v15 * 2;         /* Immediate constant *2 */
        }
        
        /* More calculations with address-like computations */
        volatile long addr_base = (long)&arr1[i];
        v17 = (int)(addr_base >> 2) + 64;  /* Immediate constant +64 */
        v18 = v17 & v16;
        v19 = v18 * 11;            /* Immediate constant *11 */
        v20 = v19 % 13;            /* Immediate constant %13 */
        
        /* Final result accumulation */
        result ^= v20;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more register pressure */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = a * 2;
    volatile int c = b - 3;
    volatile int d = c & 0xFF;
    volatile int e = d | 0x80;
    volatile int f = e << 2;
    volatile int g = f >> 1;
    volatile int h = g + 7;
    volatile int i = h * 3;
    volatile int j = i % 5;
    
    asm volatile("" : : : "memory");
    
    return j;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int i, checksum = 0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Create register pressure in main too */
    volatile int pressure_var1 = 0;
    volatile int pressure_var2 = 0;
    volatile int pressure_var3 = 0;
    volatile int pressure_var4 = 0;
    volatile int pressure_var5 = 0;
    
    /* Call the high pressure loop multiple times */
    for (int iter = 0; iter < 10; iter++) {
        volatile int bound = 100 + (iter % 50);  /* Volatile bound */
        
        checksum ^= high_pressure_loop(array1, array2, array3, bound);
        
        /* Create more register pressure between calls */
        pressure_var1 = create_more_pressure(checksum);
        pressure_var2 = create_more_pressure(pressure_var1);
        pressure_var3 = create_more_pressure(pressure_var2);
        pressure_var4 = create_more_pressure(pressure_var3);
        pressure_var5 = create_more_pressure(pressure_var4);
        
        /* Use all pressure variables to keep them live */
        checksum += pressure_var1 + pressure_var2 + pressure_var3 + 
                   pressure_var4 + pressure_var5;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
