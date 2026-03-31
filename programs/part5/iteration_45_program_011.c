#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int i = 0; i < bound; i++) {
        /* Force multiple basic blocks with conditional */
        if (arr1[i] & 1) {
            /* Chain of dependent computations using many variables */
            volatile int v1 = arr1[i];      /* Candidate for remat: arr1[i] */
            volatile int v2 = v1 + 1;       /* Immediate constant +1 */
            volatile int v3 = v2 * 2;       /* Immediate constant *2 */
            volatile int v4 = v3 & arr2[i];
            volatile int v5 = v4 | 0xFF;    /* Immediate constant 0xFF */
            volatile int v6 = v5 - 7;       /* Immediate constant -7 */
            volatile int v7 = v6 ^ arr3[i];
            volatile int v8 = v7 << 3;      /* Immediate constant <<3 */
            volatile int v9 = v8 >> 1;      /* Immediate constant >>1 */
            volatile int v10 = v9 + i;      /* Loop invariant i */
            volatile int v11 = v10 * 3;     /* Immediate constant *3 */
            volatile int v12 = v11 & 0x7F;  /* Immediate constant 0x7F */
            volatile int v13 = v12 | v1;
            volatile int v14 = v13 - v2;
            volatile int v15 = v14 + v3;
            
            /* Mix different data widths to create partial register dependencies */
            volatile char c1 = (v15 & 0xFF);
            volatile short s1 = (v14 & 0xFFFF);
            volatile long l1 = v13 * v12;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
            
            /* More computations with address arithmetic */
            volatile int idx = i * 4;       /* Loop-invariant base *4 */
            volatile int addr_calc = idx + 8; /* Immediate +8 */
            
            /* Nested scope for additional pressure */
            {
                volatile int w1 = addr_calc & 0x3F;
                volatile int w2 = w1 * 5;   /* Immediate *5 */
                volatile int w3 = w2 + arr1[idx % SIZE];
                volatile int w4 = w3 | 0x1F;
                
                result += w4 + c1 + s1 + (l1 & 0xFFFFFFFF);
            }
            
            /* Another conditional to split basic block */
            if (v15 > 1000) {
                volatile int t1 = v15 * 2;  /* Another *2 immediate */
                volatile int t2 = t1 - 17;  /* Immediate -17 */
                result ^= t2;
            }
        } else {
            /* Alternative path with different computations */
            volatile int x1 = arr2[i];
            volatile int x2 = x1 + 4;       /* Immediate +4 */
            volatile int x3 = x2 * 3;       /* Immediate *3 */
            volatile int x4 = x3 & arr3[i];
            volatile int x5 = x4 | 0x3F;    /* Immediate 0x3F */
            volatile int x6 = x5 - 11;      /* Immediate -11 */
            volatile int x7 = x6 ^ arr1[i];
            
            /* More mixed-width operations */
            volatile short s2 = x7 & 0x7FFF;
            volatile char c2 = (x6 >> 8) & 0xFF;
            
            result += x7 + s2 + c2;
        }
        
        /* Intermediate memory barrier */
        asm volatile("" : : : "memory");
        
        /* Additional computations to extend live ranges */
        volatile int y1 = arr3[i];
        volatile int y2 = y1 + 2;          /* Immediate +2 */
        volatile int y3 = y2 * 7;          /* Immediate *7 */
        volatile int y4 = y3 & arr1[i];
        
        result += y4;
    }
    
    return result;
}

/* Another noinline function to create more register pressure */
__attribute__((noinline, noipa))
static volatile int secondary_pressure(volatile int *arr, volatile int count) {
    volatile int sum = 0;
    
    for (volatile int j = 0; j < count; j++) {
        /* Complex expression with many intermediates */
        volatile int a = arr[j];
        volatile int b = a + j;            /* Loop invariant j */
        volatile int c = b * 2;            /* Immediate *2 */
        volatile int d = c & 0x7FFFFFFF;
        volatile int e = d | 0x80000000;
        volatile int f = e - 1;            /* Immediate -1 */
        volatile int g = f ^ arr[(j + 1) % SIZE];
        volatile int h = g << 2;           /* Immediate <<2 */
        volatile int i = h >> 1;           /* Immediate >>1 */
        volatile int k = i + 42;           /* Immediate +42 */
        
        sum += k;
        
        /* Force spill/reload with volatile access */
        volatile int *ptr = &arr[j % SIZE];
        asm volatile("" : "+r" (*ptr) : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[SIZE];
    volatile int array2[SIZE];
    volatile int array3[SIZE];
    
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Create register pressure with volatile bounds */
    volatile int outer_bound = ITERS;
    volatile int checksum = 0;
    
    /* Multiple calls to increase pressure */
    for (volatile int iter = 0; iter < 5; iter++) {
        checksum += high_pressure_loop(array1, array2, array3, outer_bound);
        checksum += secondary_pressure(array1, ITERS / 2);
        checksum += secondary_pressure(array2, ITERS / 3);
        checksum += secondary_pressure(array3, ITERS / 4);
        
        /* Modify arrays slightly to prevent complete optimization */
        for (volatile int i = 0; i < SIZE; i += 64) {
            array1[i] ^= checksum & 0xFF;
            array2[i] ^= (checksum >> 8) & 0xFF;
            array3[i] ^= (checksum >> 16) & 0xFF;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
