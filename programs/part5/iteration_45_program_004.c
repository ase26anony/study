#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int i = 0; i < bound; i++) {
        /* Force register pressure with many live variables */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Load operations creating register pressure */
        v1 = arr1[i];
        v2 = arr2[i];
        v3 = arr3[i];
        
        /* Chain of dependent computations with immediate constants */
        /* These constants are candidates for rematerialization */
        v4 = v1 + 1;          /* Immediate constant +1 */
        v5 = v2 * 2;          /* Immediate constant *2 */
        v6 = v3 & 0xFF;       /* Immediate constant 0xFF */
        v7 = v4 | 0x80;       /* Immediate constant 0x80 */
        v8 = v5 - 42;         /* Immediate constant 42 */
        
        /* More computations creating complex live ranges */
        v9 = v6 + v7;
        v10 = v8 * v9;
        v11 = v10 >> 3;       /* Immediate constant 3 */
        v12 = v11 << 2;       /* Immediate constant 2 */
        
        /* Conditional branch creating multiple basic blocks */
        if (v12 > 1000) {
            /* Different computation path with more immediates */
            v13 = v12 + 256;  /* Immediate constant 256 */
            v14 = v13 * 3;    /* Immediate constant 3 */
            v15 = v14 & 0x7F; /* Immediate constant 0x7F */
            
            /* Address computation with loop-invariant base */
            /* This creates REG references for rematerialization */
            volatile int *ptr1 = arr1 + i;
            volatile int *ptr2 = arr2 + i;
            volatile int *ptr3 = arr3 + i;
            
            v16 = *ptr1 + *ptr2;
            v17 = v16 * *ptr3;
            v18 = v17 + 512;  /* Immediate constant 512 */
            
            /* Mix operations with different widths */
            volatile char c1 = (char)v18;
            volatile short s1 = (short)v18;
            v19 = (int)c1 + (int)s1 + v18;
            
            result += v19;
        } else {
            /* Alternative path with different computations */
            v13 = v12 - 128;  /* Immediate constant 128 */
            v14 = v13 / 4;    /* Immediate constant 4 */
            v15 = v14 | 0x3F; /* Immediate constant 0x3F */
            
            /* More address computations */
            volatile int idx = i * 2;  /* Immediate constant 2 */
            v16 = arr1[idx] + arr2[idx];
            v17 = v16 - arr3[idx];
            v18 = v17 & 0x1FF; /* Immediate constant 0x1FF */
            
            /* Use different data types */
            volatile long l1 = (long)v18;
            volatile char c2 = (char)(l1 & 0xFF);
            v19 = (int)l1 + (int)c2;
            
            result -= v19;
        }
        
        /* Use all variables to keep them live */
        v20 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
              v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Final computation using all variables */
        result ^= v20;
        
        /* Nested loop for additional pressure */
        for (volatile int j = 0; j < 4; j++) {
            volatile int t1 = result + j;
            volatile int t2 = t1 * (j + 1);  /* Immediate constant +1 */
            volatile int t3 = t2 & (0xFF >> j);  /* Computed immediate */
            volatile int t4 = t3 | (1 << j);     /* Computed immediate */
            result = t4;
            
            /* Conditional inside nested loop */
            if (t4 % 2 == 0) {  /* Immediate constant 2 */
                volatile int t5 = t4 + 64;  /* Immediate constant 64 */
                volatile int t6 = t5 * 5;   /* Immediate constant 5 */
                result = t6;
            }
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = a * 2;
    volatile int c = b - 3;
    volatile int d = c & 0xF;
    volatile int e = d | 0x10;
    volatile int f = e << 1;
    volatile int g = f >> 2;
    volatile int h = g + 7;
    volatile int i = h * 11;
    volatile int j = i % 13;
    
    /* Use all variables */
    return a + b + c + d + e + f + g + h + i + j;
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
    
    /* Volatile bound to prevent loop unrolling */
    volatile int iterations = ITERATIONS;
    
    /* Call the high pressure function */
    volatile int result = high_pressure_loop(array1, array2, array3, iterations);
    
    /* Create additional register pressure in main */
    volatile int checksum = 0;
    for (volatile int i = 0; i < 100; i++) {
        checksum += create_more_pressure(i);
        checksum ^= result;
    }
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
