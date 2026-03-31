/* Program to trigger scheduler state save/restore cleanup in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force use of specific x86 microarchitecture with detailed scheduling model */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int *volatile_ptr = arr1; /* Prevent optimizations */
    
    /* Initialize variables to create dependencies */
    v0 = *volatile_ptr;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 - v0;
    v4 = v3 ^ v1;
    v5 = v4 | v2;
    f0 = (float)v0 * 0.5f;
    f1 = (float)v1 * 1.5f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = __builtin_expect((arr1[i] & 0x7F) > (arr2[i] & 0x3F), 0);
        
        if (branch_cond) {
            /* Path A: Complex integer arithmetic chain */
            v6 = arr1[i] * v0;
            v7 = arr2[i] + v1;
            v8 = v6 - v7;
            v9 = v8 * v2;
            v10 = v9 ^ v3;
            v11 = v10 | v4;
            v12 = v11 & v5;
            v13 = v12 << 2;
            v14 = v13 >> 1;
            v15 = v14 + v6;
            
            /* Floating-point ops mixed with integer */
            f2 = f0 * (float)v6;
            f3 = f1 + (float)v7;
            f4 = f2 - f3;
            f5 = f4 * 2.0f;
            f6 = f5 / 1.5f;
            f7 = f6 + f0;
            
            /* Memory barrier to create serialization point */
            asm volatile("" ::: "memory");
            
            /* Use computed values to prevent dead code elimination */
            v0 = v15 ^ v0;
            v1 = v14 + v1;
            f0 = f7 - f0;
        } else {
            /* Path B: Different operation mix */
            v6 = arr1[i] + v0;
            v7 = arr2[i] * v1;
            v8 = v7 - v6;
            v9 = v8 ^ v2;
            v10 = v9 | v3;
            v11 = v10 & v4;
            v12 = v11 + v5;
            v13 = v12 >> 1;
            v14 = v13 << 2;
            v15 = v14 - v6;
            
            /* Different floating-point sequence */
            f2 = f0 + (float)v6;
            f3 = f1 * (float)v7;
            f4 = f3 - f2;
            f5 = f4 / 2.0f;
            f6 = f5 * 1.5f;
            f7 = f6 - f1;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
            
            v0 = v15 | v0;
            v1 = v14 ^ v1;
            f1 = f7 + f1;
        }
        
        /* Common merge point with more operations */
        switch (i & 0x3) {
            case 0:
                v2 = v0 + v1;
                f2 = f0 + f1;
                goto common_label;
            case 1:
                v2 = v0 - v1;
                f2 = f0 - f1;
                goto common_label;
            case 2:
                v2 = v0 * v1;
                f2 = f0 * f1;
                goto common_label;
            default:
                v2 = v0 ^ v1;
                f2 = f0;
                /* fall through */
        }
        
    common_label:
        /* More operations to increase pressure */
        v3 = v2 + v15;
        v4 = v3 * v14;
        v5 = v4 - v13;
        f3 = f2 + f7;
        f4 = f3 * f6;
        
        /* Force register spilling with many live variables */
        arr1[i] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        arr2[i] = (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    }
    
    /* Use all variables to prevent optimization */
    volatile int sink = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    (void)sink;
}

/* Alternative MIPS-specific version if cross-compiling */
#ifdef __mips__
__attribute__((noinline))
static void mips_specific_kernel(int *arr1, int *arr2, int size) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    
    /* MIPS delay slot patterns can trigger scheduler backtracking */
    for (int i = 0; i < size; i++) {
        r0 = arr1[i];
        r1 = arr2[i];
        
        /* Create branch-likely pattern for MIPS */
        if (r0 > r1) {
            r2 = r0 + r1;
            r3 = r0 - r1;
            asm volatile("" ::: "memory");
        } else {
            r2 = r0 * r1;
            r3 = r0 ^ r1;
            asm volatile("" ::: "memory");
        }
        
        /* Many dependent operations */
        r4 = r2 << 2;
        r5 = r3 >> 1;
        r6 = r4 + r5;
        r7 = r6 - r2;
        r8 = r7 * r3;
        r9 = r8 ^ r4;
        r10 = r9 | r5;
        r11 = r10 & r6;
        r12 = r11 + r7;
        r13 = r12 - r8;
        r14 = r13 * r9;
        r15 = r14 ^ r10;
        
        arr1[i] = r15;
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
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (int)(seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    /* Call the scheduling-intensive kernel */
#ifdef __mips__
    mips_specific_kernel(array1, array2, SIZE);
#else
    complex_scheduling_kernel(array1, array2, SIZE);
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
