/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force architecture-specific scheduling model usage */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with volatile reads to create hard dependencies */
    v1 = *arr1;
    v2 = *arr2;
    mem_barrier = v1 + v2;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = arr1[i] & 0x7F; /* Use lower bits for more randomness */
        
        if (__builtin_expect((branch_cond > 64), 0)) {
            /* Path A: Integer-heavy computation */
            v3 = arr1[i] * 3;
            v4 = arr2[i] / 5;
            v5 = v3 ^ v4;
            v6 = v5 << 2;
            v7 = v6 + arr1[(i + 1) % size];
            v8 = v7 - arr2[(i + 2) % size];
            v9 = v8 | 0xABCD;
            v10 = v9 & 0x7FFF;
            v11 = v10 * v3;
            v12 = v11 + v4;
            v13 = v12 ^ v5;
            v14 = v13 - v6;
            v15 = v14 + v7;
            
            /* Mix in some floating point to use different functional units */
            f1 = (float)v3 * 1.5f;
            f2 = (float)v4 * 2.5f;
            f3 = f1 + f2;
            f4 = f3 * 0.75f;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            f5 = f4 - (float)v5;
            f6 = f5 * 3.14159f;
            v1 = (int)f6 + v8;
            v2 = v1 ^ v9;
        } else {
            /* Path B: Different operation mix */
            v3 = arr1[i] + 17;
            v4 = arr2[i] - 23;
            v5 = v3 * v4;
            v6 = v5 >> 1;
            v7 = v6 ^ arr1[(i + 3) % size];
            v8 = v7 & arr2[(i + 4) % size];
            v9 = v8 | 0x1234;
            v10 = v9 + 5678;
            v11 = v10 - v3;
            v12 = v11 * v4;
            v13 = v12 / 7;
            v14 = v13 ^ v5;
            v15 = v14 << 3;
            
            /* Different floating point pattern */
            f1 = (float)v3 / 2.0f;
            f2 = (float)v4 * 3.0f;
            f3 = f2 - f1;
            f4 = f3 * 1.618f;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            f5 = f4 + (float)v6;
            f6 = f5 / 2.71828f;
            v1 = (int)f6 | v7;
            v2 = v1 & v8;
        }
        
        /* Common merge point with more operations */
        f7 = (float)v1 * (float)v2;
        f8 = f7 + f3;
        
        /* Use goto to create complex control flow within loop */
        if (i & 1) {
            goto merge_point;
        }
        
        /* Additional computation on some iterations */
        v3 = v15 + i;
        v4 = v3 * 11;
        
    merge_point:
        /* Store results back to prevent elimination */
        arr1[i] = v1 + v2 + v3 + v4 + (int)f8;
        arr2[i] = v15 ^ (int)f8;
        
        /* Another barrier to increase scheduling complexity */
        asm volatile("" ::: "memory");
    }
}

/* Alternative implementation for MIPS with delay slots */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr, int size) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int barrier;
    
    /* MIPS often benefits from explicit scheduling hints */
    for (int idx = 0; idx < size; idx++) {
        /* Create data dependencies that span multiple instructions */
        a = arr[idx];
        barrier = a;
        b = a * 3;
        c = b + 5;
        d = c ^ 0xFF;
        
        /* Conditional with likely/unlikely hints for scheduler */
        if (__builtin_expect((a & 0x3) == 0, 0)) {
            e = d << 2;
            f = e + arr[(idx + 1) % size];
            g = f - arr[(idx + 2) % size];
            asm volatile("" ::: "memory");
            h = g | 0xAAAA;
            i = h & 0x5555;
        } else {
            e = d >> 1;
            f = e * arr[(idx + 3) % size];
            g = f / 7;
            asm volatile("" ::: "memory");
            h = g ^ 0x3333;
            i = h + 0x1111;
        }
        
        /* More operations to increase pressure */
        j = i * 13;
        k = j - 17;
        l = k ^ i;
        m = l + j;
        n = m * 3;
        o = n >> 2;
        p = o & 0xFF;
        
        arr[idx] = p;
    }
}
#endif

int main(void) {
    const int SIZE = 256;
    int *array1 = malloc(SIZE * sizeof(int));
    int *array2 = malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed & 0x7FFF);
    }
    
    /* Execute the scheduling-intensive kernel */
    complex_scheduling_kernel(array1, array2, SIZE);
    
#ifdef __mips__
    /* Also run MIPS-specific version if compiled for MIPS */
    mips_specific_kernel(array1, SIZE);
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
