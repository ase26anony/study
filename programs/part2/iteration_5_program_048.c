/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o trigger haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    v0 = *arr1;
    v1 = *arr2;
    mem_barrier = v0 + v1;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        if (__builtin_expect((arr1[i] ^ arr2[i]) & 0x1F, 0)) {
            /* Path A: Integer-heavy computation */
            v2 = arr1[i] * 3;
            v3 = arr2[i] / 7;
            v4 = v2 ^ v3;
            v5 = v4 << 3;
            v6 = v5 | 0x7F;
            v7 = v6 - v3;
            v8 = v7 * v2;
            v9 = v8 >> 2;
            v10 = v9 & 0xFF;
            
            /* Mix in floating point to use different functional units */
            f0 = (float)v2 * 1.5f;
            f1 = (float)v3 * 0.7f;
            f2 = f0 + f1;
            f3 = f2 * 2.3f;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* More computations after barrier */
            v11 = (int)f3 + v10;
            v12 = v11 * v7;
            v13 = v12 ^ v8;
            v14 = v13 - v9;
            v15 = v14 & 0xFFFF;
            
            f4 = f3 * 1.1f;
            f5 = f4 - 2.0f;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Store results back to create dependencies */
            arr1[i] = v15 ^ (int)f5;
        } else {
            /* Path B: Different computation pattern */
            v2 = arr1[i] + 17;
            v3 = arr2[i] - 23;
            v4 = v2 * v3;
            v5 = v4 ^ 0xABCD;
            v6 = v5 >> 1;
            v7 = v6 | 0x3F;
            v8 = v7 + v2;
            v9 = v8 * 5;
            v10 = v9 % 256;
            
            /* Different floating point pattern */
            f0 = (float)arr1[i] * 2.7f;
            f1 = (float)arr2[i] * 1.3f;
            f2 = f0 - f1;
            f3 = f2 / 1.8f;
            f4 = f3 + 4.2f;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            v11 = v10 ^ (int)f4;
            v12 = v11 * 3;
            v13 = v12 + v8;
            v14 = v13 ^ v9;
            v15 = v14 & 0x7FFF;
            
            f5 = f4 * 0.9f;
            f6 = f5 + 1.1f;
            f7 = f6 * 2.0f;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            arr2[i] = v15 + (int)f7;
        }
        
        /* Converge point with more computation */
        v0 = v0 ^ v15;
        v1 = v1 + (int)f3;
        
        /* Force spilling with many live variables */
        if (i & 1) {
            f0 = f0 * 1.01f;
            v2 = v2 + 1;
            v3 = v3 - 1;
            v4 = v4 ^ 0xAA;
            v5 = v5 | 0x55;
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                   v11 + v12 + v13 + v14 + v15 + (int)f0 + (int)f1 + 
                   (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    /* Volatile write to force computation */
    mem_barrier = checksum;
}

/* Another function with switch-based control flow */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void switch_based_computation(int *arr, int size) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    float x = 1.0f, y = 2.0f, z = 3.0f;
    
    for (int i = 0; i < size; i++) {
        int op = arr[i] % 6;
        
        /* Complex switch creates multiple basic blocks */
        switch (op) {
            case 0:
                a = b * c;
                x = y + z;
                asm volatile("" ::: "memory");
                d = e ^ f;
                break;
            case 1:
                b = c + d;
                y = z * x;
                asm volatile("" ::: "memory");
                e = f & a;
                break;
            case 2:
                c = d - e;
                z = x - y;
                asm volatile("" ::: "memory");
                f = a | b;
                break;
            case 3:
                d = e * f;
                x = y / 2.0f;
                asm volatile("" ::: "memory");
                a = b ^ c;
                break;
            case 4:
                e = f + a;
                y = z * 1.5f;
                asm volatile("" ::: "memory");
                b = c & d;
                /* goto creates complex CFG */
                if (i & 1) goto common_label;
                break;
            case 5:
                f = a - b;
                z = x + 0.5f;
                asm volatile("" ::: "memory");
                c = d | e;
                break;
        }
        
        common_label:
        /* Common computation after switch */
        a = a ^ 0x11;
        b = b + 1;
        c = c * 2;
        
        /* Force register pressure */
        x = x * 1.01f;
        y = y + 0.01f;
        z = z - 0.01f;
    }
    
    /* Use variables */
    arr[0] = a + b + c + d + e + f + (int)x + (int)y + (int)z;
}

/* MIPS-specific patterns if cross-compiling */
#ifdef __mips__
static void mips_delay_slot_pattern(int *arr, int size) {
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern that might trigger delay slot scheduling */
        r0 = arr[i];
        r1 = r0 * 2;
        r2 = r1 + 1;
        
        /* Branch likely pattern for MIPS */
        if (r0 & 1) {
            r3 = r2 ^ 0xFF;
            asm volatile("" ::: "memory");
            r4 = r3 << 2;
        } else {
            r3 = r2 & 0x7F;
            asm volatile("" ::: "memory");
            r4 = r3 >> 1;
        }
        
        arr[i] = r4;
    }
}
#endif

int main() {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    /* Execute kernels that create scheduling pressure */
    complex_scheduling_kernel(array1, array2, SIZE);
    switch_based_computation(array1, SIZE);
    
    #ifdef __mips__
    mips_delay_slot_pattern(array2, SIZE);
    #endif
    
    /* Compute final checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
