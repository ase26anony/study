#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define NUM_VARS 18

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, volatile int *arr3) {
    volatile int result = 0;
    
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Volatile loop counter to prevent optimization */
    volatile int outer_bound = ITERATIONS / 100;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Initialize with array values to create dependencies */
        v0 = arr1[outer % ARRAY_SIZE];
        v1 = arr2[outer % ARRAY_SIZE];
        v2 = arr3[outer % ARRAY_SIZE];
        
        /* Create complex web of dependencies with immediate constants */
        /* These constants are prime candidates for rematerialization */
        v3 = v0 + 1;          /* Immediate constant +1 */
        v4 = v1 * 2;          /* Immediate constant *2 */
        v5 = v2 & 0xFF;       /* Immediate constant 0xFF */
        v6 = v3 | 0x80;       /* Immediate constant 0x80 */
        v7 = v4 - 1;          /* Immediate constant -1 */
        v8 = v5 ^ 0x55;       /* Immediate constant 0x55 */
        v9 = v6 << 2;         /* Immediate constant <<2 */
        v10 = v7 >> 1;        /* Immediate constant >>1 */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* More computations with different data types to create partial reg dependencies */
        volatile char c1 = (v8 & 0xFF);
        volatile short s1 = (v9 & 0xFFFF);
        volatile long l1 = v10;
        
        v11 = c1 + s1;
        v12 = l1 * 3;         /* Immediate constant *3 */
        v13 = v11 & 0x7F;     /* Immediate constant 0x7F */
        v14 = v12 | 0x8000;   /* Immediate constant 0x8000 */
        
        /* Conditional branch creating multiple basic blocks */
        if (v13 & 1) {        /* Immediate constant &1 */
            v15 = v14 + 4;    /* Immediate constant +4 */
            v16 = v15 * 5;    /* Immediate constant *5 */
        } else {
            v15 = v14 - 4;    /* Immediate constant -4 */
            v16 = v15 / 3;    /* Immediate constant /3 */
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* More operations mixing different widths */
        volatile unsigned char uc1 = (v16 & 0xFF);
        volatile unsigned short us1 = (v15 & 0xFFFF);
        
        v17 = uc1 + us1 + 42; /* Immediate constant +42 */
        
        /* Final result accumulation */
        result ^= v17;
        
        /* Create register pressure by keeping many values live */
        /* Force all variables to be used in computation */
        v0 = v1 + v2;
        v1 = v3 + v4;
        v2 = v5 + v6;
        v3 = v7 + v8;
        v4 = v9 + v10;
        v5 = v11 + v12;
        v6 = v13 + v14;
        v7 = v15 + v16;
        v8 = v17 + result;
        
        /* Another conditional to create more basic blocks */
        if (v0 > v8) {
            v9 = v0 - v8;
        } else {
            v9 = v8 - v0;
        }
        
        result += v9;
    }
    
    return result;
}

/* Another high pressure function with different pattern */
__attribute__((noinline, noipa))
static volatile int nested_pressure(volatile int *arr, volatile int start) {
    volatile int a = start;
    volatile int b = a + 1;
    volatile int c = b * 2;
    volatile int d = c & 0xFF;
    volatile int e = d | 0x80;
    volatile int f = e - 1;
    volatile int g = f ^ 0x55;
    volatile int h = g << 2;
    volatile int i = h >> 1;
    volatile int j = i + 42;
    volatile int k = j * 3;
    volatile int l = k & 0x7F;
    volatile int m = l | 0x8000;
    volatile int n = m + 4;
    volatile int o = n * 5;
    volatile int p = o - 3;
    volatile int q = p / 2;
    volatile int r = q ^ 0xFF00;
    
    /* Use array indexing with loop-invariant base - good for remat */
    volatile int *base_ptr = arr;
    for (volatile int idx = 0; idx < 100; idx++) {
        /* Address computation that could be rematerialized */
        volatile int *elem = base_ptr + idx;
        a += *elem;
        
        /* Chain of dependent computations */
        b = a + r;
        c = b * idx;
        d = c & (idx + 1);  /* Varying immediate */
        e = d | (idx * 2);   /* Another varying immediate */
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return a + b + c + d + e + r;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    volatile int total = 0;
    
    /* Multiple calls to increase pressure */
    for (int iter = 0; iter < 10; iter++) {
        total ^= high_pressure_loop(array1, array2, array3);
        total += nested_pressure(array1, iter);
        
        /* Shuffle arrays to prevent pattern recognition */
        for (int i = 0; i < ARRAY_SIZE - 1; i++) {
            volatile int tmp = array1[i];
            array1[i] = array1[i + 1];
            array1[i + 1] = tmp;
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
