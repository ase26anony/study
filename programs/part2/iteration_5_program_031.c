/* Compile with: gcc -O3 -fschedule-insns -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7;
    int i, j;
    
    /* Initialize with volatile reads to prevent optimization */
    v0 = *arr1; v1 = *arr2;
    f0 = (float)v0; f1 = (float)v1;
    
    for (i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = __builtin_expect((arr1[i] ^ arr2[i]) & 1, 0);
        
        if (branch_cond) {
            /* Path A: Integer-heavy computation with many dependencies */
            v2 = arr1[i] + v0;
            v3 = arr2[i] - v1;
            v4 = v2 * v3;
            v5 = v4 ^ v2;
            v6 = v5 | v3;
            v7 = v6 & v4;
            v8 = v7 << 2;
            v9 = v8 >> 1;
            v10 = v9 + v5;
            
            /* Floating-point ops mixed in */
            f2 = f0 * 1.5f;
            f3 = f1 + 2.0f;
            f4 = f2 / f3;
            f5 = f4 - f0;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v11 = v10 * arr1[i];
            v12 = v11 ^ arr2[i];
            f6 = f5 * f2;
            
            /* Another volatile operation */
            asm volatile("" : "+r"(v12) :: "memory");
            
            /* Complex control flow within the path */
            switch (v12 & 3) {
                case 0: v13 = v12 + 1; break;
                case 1: v13 = v12 - 1; break;
                case 2: v13 = v12 * 2; break;
                default: v13 = v12 / 2; break;
            }
            
            /* Force register pressure with many live values */
            v14 = v13 + v11 + v10 + v9 + v8 + v7;
            f7 = f6 + f5 + f4 + f3 + f2;
            
        } else {
            /* Path B: Different computation pattern to create scheduling conflicts */
            v2 = arr1[i] * v0;
            v3 = arr2[i] + v1;
            v4 = v2 ^ v3;
            v5 = v4 & v2;
            v6 = v5 | v3;
            v7 = v6 - v4;
            v8 = v7 * 3;
            v9 = v8 ^ 0xFF;
            
            /* Different FP sequence */
            f2 = f1 * 2.0f;
            f3 = f0 - 1.0f;
            f4 = f2 + f3;
            f5 = f4 / 1.5f;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* Operations that use different functional units */
            v10 = (v9 << 3) | (v8 >> 2);
            v11 = v10 % 17;
            f6 = f5 * 0.75f;
            
            /* Use goto to create complex CFG */
            if (v11 > 100) goto merge_point;
            
            v12 = v11 * arr1[i];
            v13 = v12 + arr2[i];
            
merge_point:
            /* Common merge point with different computations */
            v14 = v13 * 2 + v11;
            f7 = f6 * 2.0f - f5;
        }
        
        /* Merge point with memory barrier */
        asm volatile("" ::: "memory");
        
        /* Update persistent state with complex dependency chain */
        v0 = v14 ^ v0;
        v1 = v13 + v1;
        f0 = f7 + f0;
        f1 = f6 - f1;
        
        /* Occasionally force spilling with many simultaneous operations */
        if (i % 8 == 0) {
            int t0 = v0 * v1;
            int t1 = v2 + v3;
            int t2 = v4 ^ v5;
            int t3 = v6 | v7;
            int t4 = v8 & v9;
            int t5 = v10 << 1;
            int t6 = v11 >> 2;
            int t7 = v12 % 3;
            
            /* All used simultaneously */
            v15 = t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
            
            float ft0 = f0 * 1.1f;
            float ft1 = f1 + 0.5f;
            float ft2 = f2 / 1.3f;
            float ft3 = f3 - 0.2f;
            
            f0 = ft0 + ft1 + ft2 + ft3;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    volatile float fchecksum = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7;
    
    /* Use the results */
    printf("Checksums: %d, %f\n", checksum, fchecksum);
}

/* MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    /* MIPS often has delay slots and different scheduling constraints */
    volatile int reg[16];
    int i;
    
    for (i = 0; i < size; i++) {
        /* Create RAW dependencies */
        reg[0] = arr1[i];
        reg[1] = arr2[i];
        reg[2] = reg[0] + reg[1];
        reg[3] = reg[0] - reg[1];
        reg[4] = reg[2] * reg[3];
        
        /* Memory barrier for MIPS */
        asm volatile("" ::: "memory");
        
        /* More complex dependency chain */
        reg[5] = reg[4] << 2;
        reg[6] = reg[5] >> 1;
        reg[7] = reg[6] | reg[4];
        
        /* Force potential delay slot filling issues */
        if (reg[7] & 1) {
            reg[8] = reg[7] * 3;
        } else {
            reg[8] = reg[7] / 3;
        }
        
        /* Use all registers */
        reg[9] = reg[0] + reg[1] + reg[2] + reg[3] + reg[4] + reg[5] + reg[6] + reg[7] + reg[8];
    }
    
    printf("MIPS result: %d\n", reg[9]);
}
#endif

int main(void) {
    const int SIZE = 256;
    int arr1[SIZE], arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Run the scheduling stress test */
#ifdef __mips__
    mips_specific_kernel(arr1, arr2, SIZE);
#else
    complex_scheduling_kernel(arr1, arr2, SIZE);
#endif
    
    return 0;
}
