/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS: gcc -O3 -mips64 -mabi=64 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__mips__)
/* MIPS-specific patterns that may trigger state saving */
#else
/* Generic fallback */
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size, int *result) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6;
    volatile int mem_barrier; /* Force memory dependencies */
    
    /* Initialize with non-trivial values to prevent optimization */
    v1 = *arr1;
    v2 = *arr2;
    v3 = v1 ^ v2;
    v4 = v1 + v2;
    v5 = v1 - v2;
    v6 = v1 * v2;
    v7 = v1 | v2;
    v8 = v1 & v2;
    v9 = ~v1;
    v10 = v2 << 2;
    v11 = v2 >> 1;
    v12 = v1 % 31;
    v13 = v2 % 17;
    v14 = v1 * 3;
    v15 = v2 * 7;
    
    f1 = (float)v1 * 0.5f;
    f2 = (float)v2 * 1.5f;
    f3 = f1 + f2;
    f4 = f1 - f2;
    f5 = f1 * f2;
    f6 = f2 / (f1 + 1.0f);
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict pattern */
        int branch_cond = arr1[i] & 0x7;
        int threshold = arr2[i] & 0x3;
        
        /* Force scheduler to consider speculative motion */
        if (__builtin_expect((branch_cond > threshold), 0)) {
            /* Path A: Mixed integer/float operations with many dependencies */
            v1 = v1 + arr1[i];
            v2 = v2 - arr2[i];
            v3 = v3 ^ (arr1[i] * 2);
            v4 = v4 | (arr2[i] << 1);
            
            /* Floating point ops create different resource usage */
            f1 = f1 + (float)arr1[i];
            f2 = f2 - (float)arr2[i];
            f3 = f3 * 1.01f;
            
            v5 = v5 * 3;
            v6 = v6 / 2;
            v7 = v7 & 0xFFFF;
            v8 = v8 | 0xFF00;
            
            f4 = f4 + f1;
            f5 = f5 * f2;
            f6 = f6 / 1.5f;
            
            v9 = v9 + v1;
            v10 = v10 - v2;
            v11 = v11 ^ v3;
            v12 = v12 | v4;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            mem_barrier = v1;
            
            v13 = v13 * v5;
            v14 = v14 + v6;
            v15 = v15 - v7;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
        } else {
            /* Path B: Different operation mix, still high pressure */
            v1 = v1 - arr1[i];
            v2 = v2 + arr2[i];
            v3 = v3 | (arr1[i] >> 1);
            v4 = v4 ^ (arr2[i] & 0xF);
            
            f1 = f1 - (float)arr1[i];
            f2 = f2 + (float)arr2[i];
            f3 = f3 / 1.01f;
            
            v5 = v5 + 5;
            v6 = v6 * 2;
            v7 = v7 | 0x00FF;
            v8 = v8 & 0xFF0F;
            
            f4 = f4 - f1;
            f5 = f5 / f2;
            f6 = f6 * 2.0f;
            
            v9 = v9 - v1;
            v10 = v10 + v2;
            v11 = v11 | v3;
            v12 = v12 ^ v4;
            
            /* Memory barrier at different position */
            mem_barrier = v2;
            asm volatile("" ::: "memory");
            
            v13 = v13 + v5;
            v14 = v14 - v6;
            v15 = v15 * v7;
            
            /* Control flow complexity with goto */
            if (__builtin_expect((v13 & 1), 0)) {
                goto merge_point;
            }
            
            v13 = v13 ^ 0xAAAA;
            v14 = v14 | 0x5555;
            
            merge_point:
            /* Final barrier before paths merge */
            asm volatile("" ::: "memory");
        }
        
        /* Common code after branch with more operations */
        v1 = v1 ^ v15;
        v2 = v2 + v14;
        v3 = v3 * 3;
        v4 = v4 - v13;
        
        f1 = f1 + f6;
        f2 = f2 - f5;
        f3 = f3 * f4;
        
        /* Switch statement for additional control flow complexity */
        switch (arr1[i] & 0x3) {
            case 0:
                v5 = v5 << (arr2[i] & 0x3);
                break;
            case 1:
                v6 = v6 >> (arr2[i] & 0x3);
                break;
            case 2:
                v7 = v7 ^ arr2[i];
                break;
            case 3:
                v8 = v8 & arr2[i];
                break;
        }
    }
    
    /* Combine all variables to prevent dead code elimination */
    *result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
              v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
              (int)f3 + (int)f4 + (int)f5 + (int)f6;
}

/* Another function with different optimization attributes to potentially trigger
   different scheduler behaviors */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static int secondary_kernel(int *data, int size) {
    int sum = 0;
    volatile int *volatile_ptr = data;
    
    for (int i = 0; i < size; i++) {
        /* Complex pointer arithmetic to create address dependencies */
        int *ptr = data + (i & 0xF);
        
        /* Unpredictable branch */
        if (__builtin_expect((*ptr & (1 << (i & 0x7))), 0)) {
            /* Mix of operations */
            sum += *ptr * 3;
            sum ^= ptr[1] << 2;
            sum -= ptr[2] >> 1;
            
            /* Function pointer call to prevent optimization */
            void (*dummy)(void) = (void (*)(void))(&complex_scheduling_kernel);
            /* Don't actually call, but reference it */
            asm volatile("" ::"r"(dummy):"memory");
        } else {
            sum += *ptr / 2;
            sum ^= ptr[1] >> 2;
            sum -= ptr[2] << 1;
        }
        
        /* Memory clobber */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values using LCG */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    int result1 = 0, result2 = 0;
    
    /* Call the complex scheduling kernel */
    complex_scheduling_kernel(array1, array2, SIZE, &result1);
    
    /* Call secondary kernel to increase scheduling complexity */
    result2 = secondary_kernel(array1, SIZE);
    
    /* Final computation to ensure all work is used */
    int final_result = result1 ^ result2;
    
    /* Use volatile to prevent optimization of final result */
    volatile int output = final_result;
    printf("Result: %d\n", output);
    
    free(array1);
    free(array2);
    
    return 0;
}
