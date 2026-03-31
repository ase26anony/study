#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 100000

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
        v3 = arr3[i] & 0xFF;       /* Immediate constant 0xFF */
        v4 = v1 | 0x7F;            /* Immediate constant 0x7F */
        v5 = v2 - 1;               /* Immediate constant -1 */
        
        /* Complex interdependent calculations */
        v6 = v1 + v2;
        v7 = v3 - v4;
        v8 = v5 * v6;
        v9 = v7 & v8;
        v10 = v9 | v1;
        
        /* More calculations with different data widths */
        volatile char c1 = (v10 & 0xFF);
        volatile short s1 = (v9 >> 8) & 0xFFFF;
        volatile long l1 = (long)v8 * (long)v7;
        
        v11 = c1 + s1;
        v12 = (int)l1 + v11;
        v13 = v12 * 3;             /* Immediate constant 3 */
        v14 = v13 / 4;             /* Immediate constant 4 */
        v15 = v14 << 2;            /* Immediate constant 2 */
        
        /* Conditional branches creating multiple basic blocks */
        if (v15 & 1) {
            v16 = v15 + 5;         /* Immediate constant 5 */
            v17 = v16 * 6;         /* Immediate constant 6 */
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
        } else {
            v16 = v15 - 7;         /* Immediate constant 7 */
            v17 = v16 / 8;         /* Immediate constant 8 */
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Nested loop with more pressure */
        for (j = 0; j < 3; j++) {
            volatile int t1, t2, t3, t4, t5;
            
            /* Address computation with loop-invariant base (remat candidate) */
            volatile int *ptr = arr1 + i;  /* Base address computation */
            t1 = *ptr + j;
            t2 = t1 * 9;           /* Immediate constant 9 */
            t3 = t2 & 0xF;         /* Immediate constant 0xF */
            t4 = t3 | 0xA;         /* Immediate constant 0xA */
            t5 = t4 ^ 0x5;         /* Immediate constant 0x5 */
            
            v18 = v17 + t5;
            v19 = v18 * 10;        /* Immediate constant 10 */
            v20 = v19 - 11;        /* Immediate constant 11 */
            
            /* Force all variables to be live */
            result += v20 + v19 + v18 + v17 + v16 + v15 + v14 + v13 + 
                     v12 + v11 + v10 + v9 + v8 + v7 + v6 + v5 + 
                     v4 + v3 + v2 + v1 + c1 + s1 + (int)l1 + t1 + t2 + t3 + t4 + t5;
        }
        
        /* Another conditional to split live ranges */
        if (result > 1000) {
            volatile int extra1 = result * 12;  /* Immediate constant 12 */
            volatile int extra2 = extra1 / 13;  /* Immediate constant 13 */
            volatile int extra3 = extra2 | 0xAA;/* Immediate constant 0xAA */
            result = extra3;
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Another noinline function to create more pressure */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 14;       /* Immediate constant 14 */
    volatile int b = a * 15;       /* Immediate constant 15 */
    volatile int c = b & 0xBB;     /* Immediate constant 0xBB */
    volatile int d = c | 0xCC;     /* Immediate constant 0xCC */
    volatile int e = d ^ 0xDD;     /* Immediate constant 0xDD */
    volatile int f = e << 3;       /* Immediate constant 3 */
    volatile int g = f >> 4;       /* Immediate constant 4 */
    
    /* Mix operations with different widths */
    volatile char c1 = g & 0xFF;
    volatile short s1 = (g >> 8) & 0xFFFF;
    volatile long l1 = (long)g * (long)a;
    
    return c1 + s1 + (int)l1 + a + b + c + d + e + f + g;
}

int main(void) {
    volatile int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    volatile int i, bound = ITERS % SIZE;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }
    
    /* Create register pressure */
    volatile int result1 = high_pressure_loop(arr1, arr2, arr3, bound);
    
    /* More pressure in main */
    volatile int pressure_var1 = result1 + 16;  /* Immediate constant 16 */
    volatile int pressure_var2 = pressure_var1 * 17;
    volatile int pressure_var3 = pressure_var2 & 0xEE;
    volatile int pressure_var4 = pressure_var3 | 0xFF;
    volatile int pressure_var5 = pressure_var4 ^ 0x11;
    
    /* Call another pressure-creating function */
    volatile int result2 = create_more_pressure(pressure_var5);
    
    /* Final computation with many live variables */
    volatile int final = result1 + result2 + pressure_var1 + pressure_var2 + 
                        pressure_var3 + pressure_var4 + pressure_var5;
    
    printf("Checksum: %d\n", final);
    return 0;
}
