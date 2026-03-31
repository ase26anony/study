/* caller-save-test.c
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 * Or for RISC-V: riscv32-unknown-elf-gcc -O2 -march=rv32imc -fno-schedule-insns -fno-inline caller-save-test.c -o caller-save-test.elf
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Noinline function that clobbers registers */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    
    /* Modify through pointers to create side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n && i < 3; i++) {
        if (arr) arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + global_seed;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 1;
    v2 = seed * 2 + argc;
    v3 = v1 + v2 * 3;
    v4 = v2 - v1 / 2;
    v5 = (v3 << 3) | (v4 >> 1);
    v6 = v5 ^ v3;
    v7 = v6 * 7 + 12345;
    v8 = v7 - v6 / 3;
    v9 = v8 | v5;
    v10 = v9 & 0xFFFF;
    
    v11 = v10 * 11;
    v12 = v11 + seed;
    v13 = v12 - v10;
    v14 = v13 * v11;
    v15 = v14 / (seed + 1);
    v16 = v15 ^ v14;
    v17 = v16 << 2;
    v18 = v17 >> 1;
    v19 = v18 + v16;
    v20 = v19 * 20;
    
    v21 = v20 + argc;
    v22 = v21 * 22;
    v23 = v22 - v21;
    v24 = v23 | v22;
    v25 = v24 & 0xFF;
    v26 = v25 * 26;
    v27 = v26 + seed;
    v28 = v27 - v25;
    v29 = v28 * 29;
    v30 = v29 / (argc + 2);
    
    /* Create conditional branch where one path has high register pressure */
    if (seed % 3 == 0) {
        /* High register pressure path - call with many live variables */
        
        /* Use all variables in computation before call */
        int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                        v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Call clobbering function - many registers must be saved */
        clobber_callee(&v1, &v2, &v3);
        
        /* More computations to keep variables live */
        v4 = v1 + v2 + v3;
        v5 = v4 * 2;
        
        /* Another call with different arguments */
        int arr[3] = {v6, v7, v8};
        clobber_more(arr, 3);
        v6 = arr[0];
        v7 = arr[1];
        v8 = arr[2];
        
        /* Use results in complex way */
        v9 = v4 + v5 + v6 + v7 + v8;
        
    } else if (seed % 3 == 1) {
        /* Medium pressure path */
        clobber_callee(&v10, &v11, &v12);
        v13 = v10 + v11 + v12;
    } else {
        /* Low pressure path - no call, but still use variables */
        v14 = v15 + v16;
    }
    
    /* Loop to create multiple call sites with varying pressure */
    int loop_sum = 0;
    for (int i = 0; i < 5; i++) {
        /* Re-initialize some variables in loop to keep them live */
        v1 += i; v2 += i*2; v3 += i*3;
        
        /* Conditional inside loop with call at block end */
        if (i % 2 == 0) {
            /* Call at what might be block end */
            clobber_callee(&v1, &v2, &v3);
            
            /* More computations after call */
            v4 = v1 * v2 - v3;
        } else {
            /* Different computation path */
            v4 = v1 + v2 + v3;
        }
        
        /* Switch statement to create more complex control flow */
        switch (i % 3) {
            case 0:
                /* Call at potential block end */
                clobber_more(&v5, 1);
                v5 += v4;
                break;
            case 1:
                /* Another call site */
                clobber_callee(&v6, &v7, NULL);
                v8 = v6 + v7;
                break;
            case 2:
                /* No call, just computation */
                v9 = v4 * 2;
                break;
        }
        
        loop_sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
        loop_sum;
    
    /* Use result with printf to prevent optimization */
    printf("Result: %d (seed: %d)\n", final_result, seed);
    
    /* Additional volatile operations to inhibit reordering */
    asm volatile("" : : : "memory");
    
    return final_result % 256;
}
