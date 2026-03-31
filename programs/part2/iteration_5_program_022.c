/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore cleanup
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o trigger haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to work with complex microarchitecture model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    float fa = 1.0f, fb = 2.0f, fc = 3.0f, fd = 4.0f;
    float fe = 5.0f, ff = 6.0f, fg = 7.0f, fh = 8.0f;
    
    /* Volatile to prevent optimization */
    volatile int barrier_var = 0;
    
    for (int idx = 0; idx < size; idx++) {
        /* Data-dependent branch with hard-to-predict condition */
        int cond1 = arr1[idx] & 0x7F;
        int cond2 = arr2[idx] & 0x3F;
        
        /* Complex branch network to create scheduling pressure */
        if (__builtin_expect((cond1 > cond2), 0)) {
            /* Path 1: Integer-heavy computation */
            a = b + c * d - e;
            f = (g << 2) | (h >> 1);
            i = j ^ k & l;
            
            /* Memory barrier to force serialization */
            asm volatile("" ::: "memory");
            
            /* More computations with dependencies */
            b = a * f - i;
            c = (b >> 3) + (f & 0xFF);
            d = e ^ (g * h);
            
            /* Another barrier */
            barrier_var = idx;
            asm volatile("" ::: "memory");
            
            /* Floating point mix */
            fa = fb * fc + fd;
            fe = ff / fg - fh;
            
            /* Cross-type operations */
            a = (int)fa + b;
            fb = (float)c * 1.5f;
            
        } else {
            /* Path 2: Different computation pattern */
            /* Use goto to create complex CFG */
            if (cond1 & 1) goto compute_block1;
            if (cond2 & 2) goto compute_block2;
            
        compute_block1:
            /* Alternative integer operations */
            k = l + a * b - c;
            d = e ^ f | g;
            h = (i * j) >> 2;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            /* Continue with different pattern */
            l = k - d + h;
            a = b * c / (d + 1);
            
            goto merge_point;
            
        compute_block2:
            /* Yet another pattern */
            fa = fb - fc * fd;
            fe = ff + fg / fh;
            
            /* Integer from float */
            j = (int)fa * (int)fe;
            k = j ^ (int)fb;
            
            /* Barrier */
            barrier_var = idx * 2;
            asm volatile("" ::: "memory");
            
            /* More mixed operations */
            l = k + (int)(fc * 2.0f);
            fb = (float)l / 3.0f;
            
        merge_point:
            /* Common merge point with more computation */
            c = d + e - f;
            fd = fe * ff - fg;
            
            /* Final barrier in this path */
            asm volatile("" ::: "memory");
        }
        
        /* Loop-carried dependencies */
        arr1[idx] = a + b + c;
        arr2[idx] = (int)(fa + fb) * d;
        
        /* Occasional extra barrier based on value */
        if ((idx & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l +
                         (int)fa + (int)fb + (int)fc + (int)fd +
                         (int)fe + (int)ff + (int)fg + (int)fh +
                         barrier_var;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
}

/* Secondary function with switch-based control flow */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void switch_based_scheduler_pressure(int *arr, int size) {
    int vars[16];
    float fvars[8];
    
    /* Initialize */
    for (int i = 0; i < 16; i++) vars[i] = i + 1;
    for (int i = 0; i < 8; i++) fvars[i] = i * 1.5f;
    
    for (int idx = 0; idx < size; idx++) {
        /* Complex switch based on array value */
        switch (arr[idx] & 0x7) {
            case 0:
                vars[0] = vars[1] * vars[2] - vars[3];
                fvars[0] = fvars[1] + fvars[2] * fvars[3];
                asm volatile("" ::: "memory");
                break;
                
            case 1:
                vars[4] = (vars[5] << 3) | (vars[6] >> 1);
                fvars[4] = fvars[5] / fvars[6] - fvars[7];
                /* No break - fall through */
                
            case 2:
                vars[7] = vars[8] ^ vars[9] & vars[10];
                fvars[1] = fvars[2] * fvars[0] + fvars[3];
                asm volatile("" ::: "memory");
                break;
                
            case 3:
                vars[11] = vars[12] + vars[13] - vars[14];
                /* Barrier in middle of case */
                asm volatile("" ::: "memory");
                fvars[5] = fvars[6] * 2.0f - fvars[7];
                break;
                
            default:
                vars[15] = vars[0] * vars[1] + vars[2];
                for (int i = 0; i < 4; i++) {
                    fvars[i] = fvars[i+4] * 0.5f;
                }
                asm volatile("" ::: "memory");
        }
        
        /* Cross-case dependencies */
        vars[0] += vars[15];
        fvars[0] -= fvars[7];
        
        /* Store result */
        arr[idx] = vars[0] + (int)fvars[0];
    }
}

/* MIPS-specific patterns if cross-compiling */
#ifdef __mips__
static void mips_delay_slot_pressure(int *arr, int size) {
    int a = 1, b = 2, c = 3;
    float fa = 1.0f, fb = 2.0f;
    
    for (int i = 0; i < size; i++) {
        /* Pattern that might exploit delay slots */
        asm volatile(
            "add %0, %1, %2\n\t"
            "mul.s %3, %4, %5\n\t"
            : "+r"(a), "+f"(fa)
            : "r"(b), "f"(fb), "r"(c), "f"(fa)
            : "memory"
        );
        
        b = a + i;
        c = b * 2 - a;
        fb = fa * 2.0f;
        
        asm volatile("" ::: "memory");
        
        arr[i] = a + b + c + (int)(fa + fb);
    }
}
#endif

int main() {
    const int SIZE = 512;
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
        array2[i] = (int)(seed & 0x3FFF);
    }
    
    /* Call scheduling-intensive kernels */
    complex_scheduling_kernel(array1, array2, SIZE);
    
    /* Process array1 with switch-based kernel */
    switch_based_scheduler_pressure(array1, SIZE);
    
#ifdef __mips__
    mips_delay_slot_pressure(array2, SIZE);
#endif
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] ^ array2[i];
        checksum = (checksum << 1) | (checksum >> 31); /* Rotate */
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
