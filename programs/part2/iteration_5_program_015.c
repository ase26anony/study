/* haifa_sched_trigger.c
 * Designed to trigger GCC's Haifa scheduler state save/restore mechanism
 * and execute the cleanup code for saved scheduler contexts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__aarch64__)
/* AArch64 has complex scheduling models */
#elif defined(__mips__)
/* MIPS often has delay slots requiring scheduler backtracking */
#endif
static int process_block(int *arr1, int *arr2, int idx, int threshold) {
    /* High register pressure: many local variables */
    volatile int v0 = arr1[idx];  /* volatile to prevent optimization */
    int v1 = arr2[idx];
    int v2 = v0 + v1;
    int v3 = v0 - v1;
    int v4 = v0 * v1;
    int v5 = v0 ^ v1;
    int v6 = v0 | v1;
    int v7 = v0 & v1;
    float f0 = (float)v0;
    float f1 = (float)v1;
    float f2 = f0 + f1;
    float f3 = f0 - f1;
    float f4 = f0 * f1;
    int v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Data-dependent branch creating scheduling pressure */
    if (__builtin_expect((v0 & 0x3F) > threshold, 0)) {
        /* Path A: Complex arithmetic chain */
        v8 = v2 * v3;
        v9 = v4 ^ v5;
        v10 = v6 | v7;
        v11 = v8 + v9;
        v12 = v10 - v8;
        f2 = f2 * f3;
        f3 = f4 + f2;
        v13 = (int)f3 + v11;
        v14 = v12 * v13;
        v15 = v14 ^ v8;
        
        /* Memory barrier to create serialization point */
        asm volatile("" ::: "memory");
        
        /* More operations after barrier */
        v0 = v15 + v9;
        v1 = v10 * v11;
        f0 = f4 - f3;
    } else {
        /* Path B: Different arithmetic pattern */
        v8 = v3 * v4;
        v9 = v5 ^ v6;
        v10 = v7 | v2;
        v11 = v8 - v9;
        v12 = v10 + v8;
        f2 = f4 / (f3 + 1.0f);
        f3 = f2 * f0;
        v13 = (int)f3 - v11;
        v14 = v12 ^ v13;
        v15 = v14 | v8;
        
        /* Memory barrier at different position */
        asm volatile("" ::: "memory");
        
        /* Different operations after barrier */
        v0 = v15 - v9;
        v1 = v10 ^ v11;
        f0 = f4 + f3;
    }
    
    /* Merge point with more operations */
    v2 = v0 * v1;
    v3 = (int)f0 + v2;
    
    /* Another barrier before return */
    asm volatile("" ::: "memory");
    
    return v3 + v15;
}

/* Complex control flow with goto to create CFG complexity */
static int process_with_goto(int *arr, int idx) {
    int result = arr[idx];
    
    switch (arr[idx] & 0x7) {
        case 0: goto common_label;
        case 1: result += 1; goto common_label;
        case 2: result *= 2; goto common_label;
        case 3: result ^= 0xFF; goto common_label;
        case 4: result -= 5; goto common_label;
        case 5: result |= 0xAA; goto common_label;
        case 6: result &= 0x55; goto common_label;
        case 7: result = ~result; goto common_label;
    }
    
common_label:
    /* Complex operations at merge point */
    int a = result * 3;
    int b = result + 7;
    int c = a ^ b;
    int d = c | result;
    int e = d & 0x3F;
    float f = (float)e;
    f = f * 1.5f;
    e = (int)f + d;
    
    asm volatile("" ::: "memory");
    
    return e;
}

int main(void) {
    const int SIZE = 256;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* High register pressure accumulator variables */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    float facc1 = 0.0f, facc2 = 0.0f;
    
    /* Outer loop with data-dependent inner paths */
    for (int iter = 0; iter < 1000; iter++) {
        int threshold = (iter & 0x1F) + 10;  /* Varying threshold */
        
        for (int i = 0; i < SIZE; i++) {
            /* Process with high register pressure function */
            int res1 = process_block(array1, array2, i, threshold);
            
            /* Process with goto-based control flow */
            int res2 = process_with_goto(array1, i);
            
            /* Accumulate results to prevent elimination */
            acc1 += res1;
            acc2 ^= res2;
            acc3 = acc3 * 3 + res1;
            acc4 = acc4 - res2;
            
            /* Floating point accumulation */
            facc1 += (float)res1 * 0.5f;
            facc2 -= (float)res2 * 0.25f;
            
            /* Occasional memory barrier */
            if ((i & 0x1F) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Mix accumulators to create dependencies */
        acc1 = acc1 ^ acc3;
        acc2 = acc2 + acc4;
        facc1 = facc1 - facc2;
    }
    
    /* Final computation to use all accumulators */
    int final_result = acc1 + acc2 + (int)facc1 + (int)facc2;
    
    /* Use volatile function pointer to inhibit optimization */
    volatile int (*volatile_print)(const char *, ...) = (int (*)(const char *, ...))printf;
    volatile_print("Result: %d\n", final_result);
    
    free(array1);
    free(array2);
    
    return 0;
}
