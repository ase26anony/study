#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < bound; outer++) {
        /* Force register pressure with many live variables */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Load data with address arithmetic - creates remat candidates */
        v1 = arr1[outer % ARRAY_SIZE];
        v2 = arr2[outer % ARRAY_SIZE];
        v3 = arr3[outer % ARRAY_SIZE];
        
        /* Chain of dependent computations with immediate constants */
        v4 = v1 + 1;          /* Candidate for rematerialization */
        v5 = v2 * 2;          /* Candidate for rematerialization */
        v6 = v3 & 0xFF;       /* Candidate for rematerialization */
        
        /* More computations creating register pressure */
        v7 = v4 + v5;
        v8 = v6 | 0x80;       /* Another immediate constant */
        v9 = v7 - v8;
        v10 = v9 * 3;         /* Immediate constant */
        
        /* Conditional branch creating multiple basic blocks */
        if (v10 & 1) {
            v11 = v10 + 5;    /* Immediate constant */
            v12 = v11 << 2;   /* Immediate constant */
        } else {
            v11 = v10 - 3;    /* Immediate constant */
            v12 = v11 >> 1;   /* Immediate constant */
        }
        
        /* More variables with different data types to create partial regs */
        volatile char c1 = v12 & 0xFF;
        volatile short s1 = v12 & 0xFFFF;
        volatile long l1 = v12;
        
        v13 = c1 + s1;
        v14 = l1 * 2;         /* Immediate constant */
        v15 = v13 & v14;
        
        /* Nested loop with more pressure */
        for (volatile int inner = 0; inner < 4; inner++) {
            volatile int t1, t2, t3, t4, t5;
            
            t1 = v15 + inner;
            t2 = t1 * 2;      /* Immediate constant */
            t3 = t2 + 1;      /* Immediate constant */
            t4 = t3 & 0x7F;   /* Immediate constant */
            t5 = t4 | 0x40;   /* Immediate constant */
            
            v15 = t5;
            
            /* Memory barrier to prevent loop fusion */
            asm volatile("" : : : "memory");
        }
        
        v16 = v15 + 8;        /* Immediate constant */
        v17 = v16 - 4;        /* Immediate constant */
        v18 = v17 * 3;        /* Immediate constant */
        v19 = v18 / 2;        /* Immediate constant */
        v20 = v19 & 0x3F;     /* Immediate constant */
        
        /* Final result accumulation */
        result ^= v20;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another high pressure function with different pattern */
__attribute__((noinline, noipa))
static volatile int secondary_pressure(volatile int *arr, volatile int count) {
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    volatile int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    
    for (volatile int idx = 0; idx < count; idx++) {
        /* Complex address computation with immediate constants */
        int offset = idx + 1;           /* +1 immediate */
        int index = offset * 2;         /* *2 immediate */
        int mask = index & 0x3FF;       /* &0x3FF immediate */
        
        a = arr[mask % ARRAY_SIZE];
        b = a + 7;                      /* +7 immediate */
        c = b * 4;                      /* *4 immediate */
        d = c - 3;                      /* -3 immediate */
        e = d | 0x100;                  /* |0x100 immediate */
        f = e & 0xFFF;                  /* &0xFFF immediate */
        g = f ^ 0xAAA;                  /* ^0xAAA immediate */
        h = g << 3;                     /* <<3 immediate */
        i = h >> 1;                     /* >>1 immediate */
        j = i + idx;
        k = j * 5;                      /* *5 immediate */
        l = k % 17;                     /* %17 immediate */
        m = l + 11;                     /* +11 immediate */
        n = m * 2;                      /* *2 immediate */
        o = n - 9;                      /* -9 immediate */
        p = o & 0x7F;                   /* &0x7F immediate */
        
        /* Conditional with immediate comparisons */
        if (p > 50) {                   /* >50 immediate */
            a = p + 20;                 /* +20 immediate */
        } else if (p < 20) {            /* <20 immediate */
            a = p - 10;                 /* -10 immediate */
        } else {
            a = p * 3;                  /* *3 immediate */
        }
        
        /* Mix data types */
        volatile char ch = a & 0xFF;
        volatile short sh = a & 0x7FFF;
        volatile long lg = a;
        
        b = ch + sh;
        c = lg * 2;                     /* *2 immediate */
        
        asm volatile("" : : : "memory");
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
    }
    
    /* Create register pressure */
    volatile int bound = ITERATIONS;
    
    /* Call high pressure function */
    volatile int result1 = high_pressure_loop(array1, array2, array3, bound);
    
    /* Call secondary pressure function */
    volatile int result2 = secondary_pressure(array1, bound / 10);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 ^ result2;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
