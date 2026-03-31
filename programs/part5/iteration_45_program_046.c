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
    volatile int outer_bound = 100;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Force register pressure with many live variables */
        v1 = arr1[outer % ARRAY_SIZE];
        v2 = arr2[outer % ARRAY_SIZE];
        v3 = arr3[outer % ARRAY_SIZE];
        v4 = arr4[outer % ARRAY_SIZE];
        
        /* Create complex dependency chain with immediate constants */
        /* These constants are prime candidates for rematerialization */
        v5 = v1 + 1;          /* Immediate constant +1 */
        v6 = v2 * 2;          /* Immediate constant *2 */
        v7 = v3 & 0xFF;       /* Immediate constant &0xFF */
        v8 = v4 | 0x80;       /* Immediate constant |0x80 */
        v9 = v5 - 1;          /* Immediate constant -1 */
        v10 = v6 >> 1;        /* Immediate constant >>1 */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* More computations with different data types */
        volatile char c1 = (v7 & 0xFF);
        volatile short s1 = (v8 & 0xFFFF);
        volatile long l1 = v9 * 3L;    /* Immediate constant *3 */
        
        /* Nested conditional blocks to create multiple basic blocks */
        if (v1 > 0) {
            v11 = v5 + v6;
            v12 = v7 * v8;
            
            /* Address computation with loop-invariant base */
            /* This creates REG references that might be rematerialized */
            volatile int *ptr1 = arr1 + (v1 & 0x3F);
            volatile int *ptr2 = arr2 + (v2 & 0x3F);
            
            v13 = *ptr1 + *ptr2;
            v14 = v13 * 4;    /* Immediate constant *4 */
            
            /* More immediate constants in different contexts */
            if (v11 < 1000) {
                v15 = v12 + 256;      /* +256 */
                v16 = v13 - 128;      /* -128 */
                v17 = v14 & 0x7F;     /* &0x7F */
            } else {
                v15 = v12 + 512;      /* +512 */
                v16 = v13 - 256;      /* -256 */
                v17 = v14 | 0x80;     /* |0x80 */
            }
            
            /* Cross-type operations to create partial register dependencies */
            v18 = (int)c1 + (int)s1;
            v19 = v18 * 5;    /* *5 */
            v20 = l1 + v19;
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
            
            /* Use all variables to keep them live */
            result += v15 + v16 + v17 + v18 + v19 + v20;
        } else {
            v11 = v5 - v6;
            v12 = v7 / (v8 | 1);  /* Avoid division by zero */
            
            /* Different address computation pattern */
            volatile int idx = (v2 * 3 + 7) & 0x3F;  /* Constants 3 and 7 */
            volatile int *ptr3 = arr3 + idx;
            volatile int *ptr4 = arr4 + idx;
            
            v13 = *ptr3 ^ *ptr4;   /* XOR operation */
            v14 = v13 << 2;        /* <<2 */
            
            /* More conditional blocks */
            if (v11 > -1000) {
                v15 = v12 + 64;     /* +64 */
                v16 = v13 - 32;     /* -32 */
                v17 = v14 ^ 0x55;   /* ^0x55 */
            } else {
                v15 = v12 + 128;    /* +128 */
                v16 = v13 - 64;     /* -64 */
                v17 = v14 ^ 0xAA;   /* ^0xAA */
            }
            
            /* Mixed-width operations */
            volatile unsigned char uc1 = (v15 & 0xFF);
            volatile unsigned short us1 = (v16 & 0xFFFF);
            v18 = (int)uc1 * (int)us1;
            v19 = v18 % 17;         /* %17 */
            v20 = l1 - v19;
            
            /* Use all variables */
            result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        }
        
        /* Force spilling by using many variables after the conditional */
        volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        if (sum > 0) {
            result += sum;
        }
    }
    
    return result;
}

/* Another high-pressure function to create more register pressure */
__attribute__((noinline, noipa))
static volatile int secondary_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = x * 2;
    volatile int c = x & 0xFF;
    volatile int d = x | 0x80;
    volatile int e = a + b;
    volatile int f = c * d;
    volatile int g = e - f;
    volatile int h = g >> 1;
    volatile int i = h * 3;
    volatile int j = i & 0x7F;
    
    /* Create a long dependency chain with immediate constants */
    for (volatile int k = 0; k < 10; k++) {
        a = a + 1;
        b = b * 2;
        c = c & 0xFE;
        d = d | 0x01;
        e = e - 1;
        f = f + 2;
        g = g * 3;
        h = h >> 1;
        i = i | 0x80;
        j = j ^ 0x55;
    }
    
    return a + b + c + d + e + f + g + h + i + j;
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
    
    volatile int total = 0;
    
    /* Multiple calls to increase register pressure globally */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Additional pressure between calls */
        for (volatile int i = 0; i < 100; i++) {
            total += secondary_pressure(i);
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
