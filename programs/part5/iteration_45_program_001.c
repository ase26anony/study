#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Create many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    
    /* Force register pressure with complex data dependencies */
    for (volatile int i = 0; i < bound; i++) {
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Load operations creating register pressure */
        v1 = arr1[i];           /* Candidate for remat: arr1[i] */
        v2 = arr2[i];
        v3 = arr1[i + 1];       /* Same as v1 but offset - potential remat */
        v4 = arr2[i + 1];
        
        /* Chain of dependent computations with immediate constants */
        /* These constants (1, 2, 3, 4) are rematerialization candidates */
        v5 = v1 + 1;            /* REG + CONST -> remat candidate */
        v6 = v2 * 2;            /* REG * CONST -> remat candidate */
        v7 = v3 & 3;            /* REG & CONST -> remat candidate */
        v8 = v4 | 4;            /* REG | CONST -> remat candidate */
        
        /* More computations creating register pressure */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * v10;
        
        /* Address computation with loop-invariant base + offset */
        /* This creates REG + (CONST * i) patterns for remat */
        volatile int *ptr1 = arr1 + i;      /* Base + index */
        volatile int *ptr2 = arr2 + (i * 2); /* Base + (CONST * index) */
        
        v12 = *ptr1;
        v13 = *ptr2;
        
        /* Nested conditional to create multiple basic blocks */
        if (v12 > 0) {
            /* Different width operations to create partial reg dependencies */
            volatile char c1 = (char)v12;
            volatile short s1 = (short)v13;
            volatile long l1 = (long)v11;
            
            v14 = c1 * s1;
            v15 = l1 / (v14 + 1);  /* Another immediate constant */
            
            /* More arithmetic with constants */
            v16 = v15 + 100;       /* REG + CONST */
            v17 = v16 * 200;       /* REG * CONST */
            
            asm volatile("" : : : "memory");
        } else {
            /* Alternative path with different computations */
            volatile unsigned char uc1 = (unsigned char)v12;
            volatile unsigned short us1 = (unsigned short)v13;
            
            v14 = uc1 | 0xFF;      /* REG | CONST */
            v15 = us1 & 0x7FFF;    /* REG & CONST */
            v16 = v14 ^ 0x55;      /* REG ^ CONST */
            v17 = v15 << 2;        /* REG << CONST */
            
            asm volatile("" : : : "memory");
        }
        
        /* Merge point - all variables still live */
        v18 = v11 + v14 + v15 + v16 + v17;
        
        /* More computations with different data types */
        volatile int8_t b1 = v18 & 0xFF;
        volatile int16_t w1 = v18 & 0xFFFF;
        volatile int32_t d1 = v18;
        volatile int64_t q1 = (int64_t)d1 * (int64_t)w1;
        
        v19 = (int)(q1 >> 32);
        v20 = (int)(q1 & 0xFFFFFFFF);
        
        /* Final accumulation with conditional */
        if ((i & 7) == 0) {  /* Create irregular control flow */
            result += v19;
            asm volatile("" : : : "memory");
        } else {
            result += v20;
            asm volatile("" : : : "memory");
        }
        
        /* Force spill/reload by using all variables again */
        v1 = v1 ^ v19;
        v2 = v2 ^ v20;
        v3 = v3 + v19;
        v4 = v4 + v20;
        
        /* Complex expression with multiple constants */
        result += (v1 * 3) + (v2 / 5) - (v3 & 7) | (v4 ^ 9);
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int *data, volatile int count) {
    volatile int a, b, c, d, e, f, g, h, j, k;
    volatile int sum = 0;
    
    for (volatile int i = 0; i < count; i++) {
        a = data[i] + i;      /* REG + REG (i is loop invariant after unrolling) */
        b = a * 2;            /* REG * CONST */
        c = b + 1;            /* REG + CONST */
        d = c - i;            /* REG - REG */
        e = d & 0xF;          /* REG & CONST */
        f = e | 0x10;         /* REG | CONST */
        g = f << 3;           /* REG << CONST */
        h = g >> 1;           /* REG >> CONST */
        j = h ^ 0xAA;         /* REG ^ CONST */
        k = j % 17;           /* REG % CONST */
        
        sum += k;
        
        /* Force all variables to be live simultaneously */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), 
                          "+r"(e), "+r"(f), "+r"(g), "+r"(h), 
                          "+r"(j), "+r"(k) : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    /* Use rand() for unpredictable values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
    }
    
    /* Volatile bound to prevent loop unrolling prediction */
    volatile int iterations = ITERATIONS;
    
    /* Call high pressure function */
    volatile int result1 = high_pressure_loop(array1, array2, array3, iterations);
    
    /* Create additional register pressure in main */
    volatile int extra_pressure = create_more_pressure(array1, iterations / 2);
    
    /* Mix results to prevent dead code elimination */
    volatile int final_result = result1 ^ extra_pressure;
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return 0;
}
