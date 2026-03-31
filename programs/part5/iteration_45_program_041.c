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
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* Immediate constants - candidates for rematerialization */
                volatile int c1 = 1;      /* Will likely be rematerialized */
                volatile int c2 = 2;
                volatile int c3 = 4;
                volatile int c4 = 8;
                volatile int c5 = 16;
                
                /* Variables with complex dependencies */
                volatile int v1 = arr1[i] + c1;  /* REG with constant */
                volatile int v2 = arr2[i] * c2;  /* Another constant use */
                volatile int v3 = v1 & arr3[i];
                volatile int v4 = v2 | arr4[i];
                volatile int v5 = v3 + v4;
                volatile int v6 = v5 * c3;       /* Constant multiplication */
                volatile int v7 = v6 - c4;       /* Constant subtraction */
                volatile int v8 = v7 ^ arr1[(i + 1) % ARRAY_SIZE];
                volatile int v9 = v8 << 2;       /* Shift by constant */
                volatile int v10 = v9 >> 1;
                volatile int v11 = v10 + c5;
                volatile int v12 = v11 * 3;      /* Another immediate */
                volatile int v13 = v12 & 0xFF;
                volatile int v14 = v13 | 0x80;
                volatile int v15 = v14 + outer;  /* Loop-invariant but volatile */
                
                /* Address computation with loop-invariant base - remat candidate */
                volatile int *base_ptr = arr1;
                volatile int offset = i * sizeof(int);
                volatile int *addr = (volatile int*)((char*)base_ptr + offset);
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
                
                /* Conditional branches creating multiple basic blocks */
                if (v15 & 1) {
                    /* Different computation path */
                    v1 = v15 + 7;        /* New constant */
                    v3 = v1 * 5;
                    v5 = v3 - 3;
                    asm volatile("" : : : "memory");
                } else if (v15 & 2) {
                    /* Another path */
                    v2 = v15 + 11;
                    v4 = v2 / 2;         /* Division by constant 2 */
                    v6 = v4 * 9;
                    asm volatile("" : : : "memory");
                } else {
                    /* Default path with more constants */
                    v7 = v15 + 17;
                    v8 = v7 * 13;
                    v9 = v8 & 0x7F;
                    asm volatile("" : : : "memory");
                }
                
                /* More operations mixing different widths */
                volatile char c = (v9 & 0xFF);
                volatile short s = (v8 & 0xFFFF);
                volatile long l = (v7 * v6);
                
                /* Complex interdependent calculations */
                v10 = (c * s) & l;
                v11 = v10 + (v5 * 2);    /* Constant multiplication */
                v12 = v11 | (v4 * 4);
                v13 = v12 ^ (v3 * 8);
                v14 = v13 + (v2 * 16);
                v15 = v14 - (v1 * 32);
                
                /* Use the address computation */
                volatile int loaded = *addr;
                v15 += loaded;
                
                /* Final result accumulation */
                result ^= v15;
                
                /* Another memory barrier */
                asm volatile("" : : : "memory");
            }
        }
        
        /* Modify loop-invariant data to prevent optimizations */
        arr1[outer % ARRAY_SIZE] ^= result;
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more pressure */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    /* Many local variables with constants */
    volatile int a = x + 1;
    volatile int b = a * 2;
    volatile int c = b + 4;
    volatile int d = c * 8;
    volatile int e = d - 16;
    volatile int f = e & 0xFF;
    volatile int g = f | 0x80;
    volatile int h = g << 3;
    volatile int i = h >> 1;
    volatile int j = i + 32;
    volatile int k = j * 64;
    volatile int l = k & 0x7FFF;
    volatile int m = l | 0x8000;
    volatile int n = m + 128;
    volatile int o = n * 256;
    
    asm volatile("" : : : "memory");
    
    return o;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
        array4[i] = rand();
    }
    
    volatile int final_result = 0;
    
    /* Multiple calls to increase pressure */
    for (int iter = 0; iter < 10; iter++) {
        volatile int res = high_pressure_loop(array1, array2, array3, array4);
        final_result ^= res;
        
        /* Call another function to create more register pressure */
        volatile int extra = create_more_pressure(res);
        final_result += extra;
        
        /* Shuffle arrays to prevent optimizations */
        asm volatile("" : : : "memory");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array1[i] ^= final_result;
            array2[i] += iter;
        }
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", (int)final_result);
    return 0;
}
