/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of specific scheduling model hooks */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization */
    
    /* Initialize with non-trivial values */
    v0 = arr1[0];
    v1 = arr2[0];
    v2 = v0 ^ v1;
    v3 = v0 & v1;
    v4 = v0 | v1;
    v5 = v0 + v1;
    v6 = v0 - v1;
    v7 = v0 * v1;
    v8 = v0 / (v1 ? v1 : 1);
    v9 = v0 % (v1 ? v1 : 1);
    v10 = ~v0;
    v11 = v1 << 2;
    v12 = v1 >> 2;
    v13 = v0 * 3;
    v14 = v1 * 5;
    v15 = v0 + v1 * 7;
    
    f0 = (float)v0;
    f1 = (float)v1;
    f2 = f0 + f1;
    f3 = f0 - f1;
    f4 = f0 * f1;
    f5 = f0 / (f1 != 0.0f ? f1 : 1.0f);
    f6 = f0 * 2.5f;
    f7 = f1 * 3.7f;
    
    /* Complex loop with data-dependent branches */
    for (int i = 1; i < size; i++) {
        int idx = i & 255; /* Keep within bounds */
        int val1 = arr1[idx];
        int val2 = arr2[idx];
        
        /* Hard-to-predict branch */
        if (__builtin_expect((val1 ^ val2) & 1, 0)) {
            /* Path A: Integer-heavy operations */
            v0 = v0 + val1;
            v1 = v1 - val2;
            v2 = v2 ^ val1;
            v3 = v3 & val2;
            v4 = v4 | (val1 ^ val2);
            v5 = v5 * (val1 + 1);
            v6 = v6 / (val2 ? val2 : 1);
            v7 = v7 % (val1 ? val1 : 1);
            v8 = v8 << (val2 & 3);
            v9 = v9 >> (val1 & 3);
            v10 = v10 + val1 * val2;
            v11 = v11 - val1 + val2;
            v12 = v12 ^ (val1 << 1);
            v13 = v13 & (val2 >> 1);
            v14 = v14 | (val1 + val2);
            v15 = v15 * 2 - val1;
            
            /* Floating point ops mixed in */
            f0 = f0 + (float)val1;
            f1 = f1 - (float)val2;
            f2 = f2 * 1.1f;
            f3 = f3 / (f2 != 0.0f ? f2 : 1.0f);
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v0;
            
            /* Another unpredictable branch inside the path */
            if ((val1 + val2) > 1000) {
                f4 = f4 + f0 * 2.0f;
                f5 = f5 - f1 * 0.5f;
                v0 = v0 ^ v15;
                v1 = v1 | v14;
            } else {
                f6 = f6 * f2;
                f7 = f7 / (f3 != 0.0f ? f3 : 1.0f);
                v2 = v2 & v13;
                v3 = v3 ^ v12;
            }
            
            /* More operations after the inner branch */
            v4 = v4 + v11;
            v5 = v5 - v10;
            f0 = f0 + f6;
            f1 = f1 - f7;
            
        } else {
            /* Path B: Different operation mix */
            v15 = v15 + val2;
            v14 = v14 - val1;
            v13 = v13 ^ val2;
            v12 = v12 & val1;
            v11 = v11 | (val2 ^ val1);
            v10 = v10 * (val2 + 1);
            v9 = v9 / (val1 ? val1 : 1);
            v8 = v8 % (val2 ? val2 : 1);
            v7 = v7 << (val1 & 3);
            v6 = v6 >> (val2 & 3);
            v5 = v5 + val2 * val1;
            v4 = v4 - val2 + val1;
            v3 = v3 ^ (val2 << 1);
            v2 = v2 & (val1 >> 1);
            v1 = v1 | (val2 + val1);
            v0 = v0 * 3 - val2;
            
            /* Different floating point sequence */
            f7 = f7 + (float)val2;
            f6 = f6 - (float)val1;
            f5 = f5 * 1.3f;
            f4 = f4 / (f5 != 0.0f ? f5 : 1.0f);
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            mem_barrier = v15;
            
            /* Inner branch with goto to create complex CFG */
            if ((val1 * val2) < 500) {
                f3 = f3 + f7 * 1.5f;
                f2 = f2 - f6 * 0.7f;
                v15 = v15 ^ v0;
                v14 = v14 | v1;
                goto merge_point;
            } else {
                f1 = f1 * f5;
                f0 = f0 / (f4 != 0.0f ? f4 : 1.0f);
                v13 = v13 & v2;
                v12 = v12 ^ v3;
            }
            
            /* Label for goto target */
            merge_point:
            v11 = v11 + v4;
            v10 = v10 - v5;
            f7 = f7 + f1;
            f6 = f6 - f0;
        }
        
        /* Common code after both paths */
        v0 = v0 ^ v15;
        v1 = v1 ^ v14;
        v2 = v2 ^ v13;
        v3 = v3 ^ v12;
        v4 = v4 ^ v11;
        v5 = v5 ^ v10;
        
        f0 = f0 + f7;
        f1 = f1 + f6;
        f2 = f2 + f5;
        f3 = f3 + f4;
        
        /* Another barrier to increase scheduling complexity */
        asm volatile("" ::: "memory");
    }
    
    /* Use all variables to prevent elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + (int)f0 + (int)f1 + (int)f2 + 
                   (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Volatile store to force computation */
    volatile int *volatile_ptr = &mem_barrier;
    *volatile_ptr = checksum;
}

/* Alternate version for MIPS if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    volatile int barrier;
    
    /* MIPS delay slot patterns might trigger different scheduler behavior */
    for (int i = 0; i < size; i++) {
        int val = arr1[i] + arr2[i];
        
        /* Create dependency chains */
        r0 = r0 + val;
        r1 = r1 - val;
        r2 = r2 ^ val;
        r3 = r3 & val;
        r4 = r4 | val;
        r5 = r5 * (val + 1);
        r6 = r6 / (val ? val : 1);
        r7 = r7 % (val ? val : 1);
        r8 = r8 << (val & 3);
        r9 = r9 >> (val & 3);
        r10 = r10 + r0;
        r11 = r11 - r1;
        r12 = r12 ^ r2;
        r13 = r13 & r3;
        r14 = r14 | r4;
        r15 = r15 * r5;
        
        asm volatile("" ::: "memory");
        barrier = r0;
        
        if (__builtin_expect(val & 0x80, 0)) {
            r0 = r15 + r14;
            r1 = r14 - r13;
            asm volatile("" ::: "memory");
        }
    }
}
#endif

int main() {
    const int SIZE = 1024;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
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
    
    /* Execute the scheduling-intensive kernel multiple times */
    for (int iter = 0; iter < 100; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE);
        
#ifdef __mips__
        mips_specific_kernel(array1, array2, SIZE);
#endif
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            array1[i] = array1[i] + (i & 0xF);
            array2[i] = array2[i] - (i & 0x7);
        }
    }
    
    /* Final checksum */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
