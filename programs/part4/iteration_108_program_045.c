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
    if (p1) *p1 ^= 1;
    if (p2) *p2 ^= 2;
    if (p3) *p3 ^= 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n && i < 3; i++) {
        arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent but deterministic behavior */
    volatile int seed = argc + global_seed;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 1;  v2 = seed * 2;  v3 = seed * 3;  v4 = seed * 4;  v5 = seed * 5;
    v6 = seed * 6;  v7 = seed * 7;  v8 = seed * 8;  v9 = seed * 9;  v10 = seed * 10;
    v11 = seed * 11; v12 = seed * 12; v13 = seed * 13; v14 = seed * 14; v15 = seed * 15;
    v16 = seed * 16; v17 = seed * 17; v18 = seed * 18; v19 = seed * 19; v20 = seed * 20;
    v21 = seed * 21; v22 = seed * 22; v23 = seed * 23; v24 = seed * 24; v25 = seed * 25;
    v26 = seed * 26; v27 = seed * 27; v28 = seed * 28; v29 = seed * 29; v30 = seed * 30;
    
    /* Create data dependencies between variables */
    for (int i = 0; i < 3; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        v10 = v11 + v12;
        
        v11 = v12 + v13;
        v12 = v13 + v14;
        v13 = v14 + v15;
        v14 = v15 + v16;
        v15 = v16 + v17;
        v16 = v17 + v18;
        v17 = v18 + v19;
        v18 = v19 + v20;
        v19 = v20 + v21;
        v20 = v21 + v22;
        
        v21 = v22 + v23;
        v22 = v23 + v24;
        v23 = v24 + v25;
        v24 = v25 + v26;
        v25 = v26 + v27;
        v26 = v27 + v28;
        v27 = v28 + v29;
        v28 = v29 + v30;
        v29 = v30 + v1;
        v30 = v1 + v2;
    }
    
    /* Complex conditional to create different basic blocks */
    if (seed % 3 == 0) {
        /* HIGH REGISTER PRESSURE PATH - call at end of basic block */
        /* Use many variables before the call */
        int t1 = v1 + v2 + v3 + v4 + v5;
        int t2 = v6 + v7 + v8 + v9 + v10;
        int t3 = v11 + v12 + v13 + v14 + v15;
        
        /* Call with many live registers */
        clobber_callee(&v1, &v2, &v3);
        
        /* Use results after call - keeps variables live across call */
        v1 = t1 ^ v1;
        v2 = t2 ^ v2;
        v3 = t3 ^ v3;
        
        /* Another call with different arguments */
        int arr[3] = {v4, v5, v6};
        clobber_more(arr, 3);
        v4 = arr[0];
        v5 = arr[1];
        v6 = arr[2];
        
    } else if (seed % 3 == 1) {
        /* MEDIUM PRESSURE PATH */
        clobber_callee(&v7, &v8, &v9);
        
        /* Different computation pattern */
        for (int i = 0; i < 2; i++) {
            v10 = v11 + v12;
            v11 = v12 + v13;
            v12 = v13 + v14;
            clobber_callee(&v10, &v11, &v12);
        }
        
    } else {
        /* LOW PRESSURE PATH - simpler computations */
        v20 = v21 + v22;
        v21 = v22 + v23;
    }
    
    /* Loop to create multiple call sites with varying conditions */
    int checksum = 0;
    for (int iter = 0; iter < 5; iter++) {
        /* Varying condition based on iteration */
        if ((seed + iter) % 2 == 0) {
            /* Another high-pressure call site */
            int tmp1 = v1 + v3 + v5;
            int tmp2 = v7 + v9 + v11;
            
            clobber_callee(&v1, &v3, &v5);
            
            v1 ^= tmp1;
            v3 ^= tmp2;
            
            /* Nested condition */
            if (iter % 2 == 0) {
                clobber_more(&v2, 3);
            }
        } else {
            /* Different call pattern */
            clobber_callee(&v2, &v4, &v6);
        }
        
        /* Mix all variables to keep them live */
        checksum += v1 ^ v2 ^ v3 ^ v4 ^ v5;
        checksum += v6 ^ v7 ^ v8 ^ v9 ^ v10;
        checksum += v11 ^ v12 ^ v13 ^ v14 ^ v15;
        checksum += v16 ^ v17 ^ v18 ^ v19 ^ v20;
        checksum += v21 ^ v22 ^ v23 ^ v24 ^ v25;
        checksum += v26 ^ v27 ^ v28 ^ v29 ^ v30;
        
        /* Volatile memory operation to prevent reordering */
        global_seed = checksum;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (seed: %d)\n", checksum, seed);
    
    /* Final complex expression using all variables */
    int final_result = 
        v1 * v2 - v3 * v4 + v5 * v6 - v7 * v8 +
        v9 * v10 - v11 * v12 + v13 * v14 - v15 * v16 +
        v17 * v18 - v19 * v20 + v21 * v22 - v23 * v24 +
        v25 * v26 - v27 * v28 + v29 * v30;
    
    printf("Final: %d\n", final_result);
    
    return final_result % 256;
}
