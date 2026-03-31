/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -funroll-loops=2 -fno-peel-loops -o trigger haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to consider complex microarchitecture details */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void compute_kernel(int* restrict arr1, int* restrict arr2, int size) {
    /* High register pressure setup - many live variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    
    /* Initialize with non-trivial values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16;
    
    f0 = 1.0f; f1 = 2.0f; f2 = 3.0f; f3 = 4.0f;
    f4 = 5.0f; f5 = 6.0f; f6 = 7.0f; f7 = 8.0f;
    
    /* Complex control flow with data-dependent branches */
    for (int i = 0; i < size; i++) {
        /* Force memory dependency to prevent hoisting */
        int val1 = arr1[i];
        int val2 = arr2[i];
        
        /* Hard-to-predict branch using runtime data */
        if (__builtin_expect((val1 ^ val2) & 0x1, 0)) {
            /* Path A: Integer-heavy computation with many dependencies */
            v0 = v0 + val1;
            v1 = v1 - val2;
            v2 = v2 * (val1 & 0xFF);
            v3 = v3 ^ val2;
            v4 = (v4 << 3) | (val1 & 0x7);
            v5 = v5 + (val2 >> 2);
            v6 = v6 - (val1 * 3);
            v7 = v7 & ~val2;
            
            /* Mix in floating point to use different functional units */
            f0 = f0 + (float)val1 * 0.5f;
            f1 = f1 - (float)val2 * 0.25f;
            f2 = f2 * (float)(val1 & 0xF);
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v8 = v8 + (v0 >> 1);
            v9 = v9 - (v1 << 1);
            v10 = v10 * (v2 & 0x7F);
            
            /* Another volatile operation to prevent reordering */
            asm volatile("" : "+r" (v11), "+r" (v12) : : "memory");
            
            v11 = v11 + val1;
            v12 = v12 - val2;
            
            /* Complex conditional with side effects */
            if ((val1 + val2) > 1000) {
                v13 = v13 * 2;
                v14 = v14 / 3;
                f3 = f3 * 1.5f;
            } else {
                v13 = v13 / 2;
                v14 = v14 * 3;
                f3 = f3 / 1.5f;
            }
        } else {
            /* Path B: Different computation pattern to create scheduling pressure */
            v0 = v0 - val1;
            v1 = v1 + val2;
            v2 = v2 ^ (val1 & 0xFF);
            v3 = v3 * val2;
            v4 = (v4 >> 3) | (val2 << 5);
            v5 = v5 - (val1 >> 2);
            v6 = v6 + (val2 * 3);
            v7 = v7 | val1;
            
            /* Different floating point pattern */
            f4 = f4 - (float)val2 * 0.5f;
            f5 = f5 + (float)val1 * 0.25f;
            f6 = f6 / (float)((val2 & 0xF) + 1);
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            v8 = v8 - (v0 << 1);
            v9 = v9 + (v1 >> 1);
            v10 = v10 ^ (v3 & 0x3F);
            
            /* Force register spilling with many simultaneous operations */
            v15 = v15 + v0 + v1 + v2 + v3 + v4;
            f7 = f7 + f0 + f1 + f2 + f3 + f4;
            
            /* Unpredictable branch inside the else path */
            switch (val1 & 0x3) {
                case 0:
                    v11 = v11 * 3;
                    v12 = v12 / 4;
                    f5 = f5 * 2.0f;
                    break;
                case 1:
                    v11 = v11 / 3;
                    v12 = v12 * 4;
                    f5 = f5 / 2.0f;
                    break;
                case 2:
                    v11 = v11 ^ 0xAAAA;
                    v12 = v12 | 0x5555;
                    f5 = f5 + f6;
                    break;
                default:
                    v11 = v11 & 0xCCCC;
                    v12 = v12 ^ 0x3333;
                    f5 = f5 - f6;
                    break;
            }
        }
        
        /* Merge point with more operations */
        v0 = v0 ^ v15;
        v1 = v1 + v14;
        f0 = f0 + f7;
        
        /* Another barrier to force scheduler state consideration */
        asm volatile("" ::: "memory");
        
        /* Cross-path dependencies */
        if (i & 0x1) {
            v2 = v2 + v13;
            v3 = v3 - v12;
            f1 = f1 * f6;
        } else {
            v2 = v2 - v13;
            v3 = v3 + v12;
            f1 = f1 / f6;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                           v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
                           (int)f0 + (int)f1 + (int)f2 + (int)f3 +
                           (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Use checksum to prevent optimization */
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    }
}

/* MIPS-specific variant if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_compute_kernel(int* arr1, int* arr2, int size) {
    int v0, v1, v2, v3, v4, v5, v6, v7;
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6; v6 = 7; v7 = 8;
    
    for (int i = 0; i < size; i++) {
        int val1 = arr1[i];
        int val2 = arr2[i];
        
        /* Create delay slot scheduling pressure */
        if (val1 > val2) {
            v0 = v0 + val1;
            v1 = v1 - val2;
            asm volatile("nop" ::: "memory");
            v2 = v2 * val1;
            v3 = v3 ^ val2;
        } else {
            v0 = v0 - val1;
            v1 = v1 + val2;
            asm volatile("nop" ::: "memory");
            v2 = v2 / (val1 + 1);
            v3 = v3 | val2;
        }
        
        /* Force multiple instruction types */
        v4 = (v4 << 2) | (val1 & 0x3);
        v5 = v5 + (val2 >> 1);
        v6 = v6 - val1;
        v7 = v7 & val2;
    }
    
    volatile int sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    if (sum != 0) {
        printf("MIPS sum: %d\n", sum);
    }
}
#endif

int main() {
    const int SIZE = 256;
    int arr1[SIZE];
    int arr2[SIZE];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Execute computation kernel */
#ifdef __mips__
    mips_compute_kernel(arr1, arr2, SIZE);
#else
    compute_kernel(arr1, arr2, SIZE);
#endif
    
    /* Additional loop to increase scheduling complexity */
    for (int iter = 0; iter < 10; iter++) {
        int temp = 0;
        for (int i = 0; i < SIZE; i++) {
            /* Complex dependency chain */
            temp = temp ^ arr1[i];
            temp = temp + arr2[i];
            temp = temp * 3;
            temp = temp >> 1;
            
            /* Branch with side effects */
            if (temp & 0x100) {
                arr1[i] = arr1[i] + 1;
                asm volatile("" ::: "memory");
            } else {
                arr2[i] = arr2[i] - 1;
                asm volatile("" ::: "memory");
            }
        }
    }
    
    return 0;
}
