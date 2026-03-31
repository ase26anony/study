/* Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer -c test.c */
/* For RISC-V: riscv32-unknown-elf-gcc -O2 -march=rv32imc -fno-schedule-insns -fno-inline -c test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

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

void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    
    /* Force memory side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    global_seed = argc;
    
    /* Many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = global_seed + 1;
    v2 = global_seed * 2;
    v3 = global_seed ^ 0x1234;
    v4 = v1 + v2;
    v5 = v2 * v3;
    v6 = v3 - v1;
    v7 = v4 ^ v5;
    v8 = v5 + v6;
    v9 = v6 * v7;
    v10 = v7 - v8;
    
    v11 = v8 + v9;
    v12 = v9 * v10;
    v13 = v10 ^ v11;
    v14 = v11 + v12;
    v15 = v12 * v13;
    v16 = v13 - v14;
    v17 = v14 ^ v15;
    v18 = v15 + v16;
    v19 = v16 * v17;
    v20 = v17 - v18;
    
    v21 = v18 + v19;
    v22 = v19 * v20;
    v23 = v20 ^ v21;
    v24 = v21 + v22;
    v25 = v22 * v23;
    v26 = v23 - v24;
    v27 = v24 ^ v25;
    v28 = v25 + v26;
    v29 = v26 * v27;
    v30 = v27 - v28;
    
    /* Create conditional branches with different register pressure */
    int result = 0;
    
    /* First conditional: high pressure path */
    if (global_seed & 1) {
        /* Use many variables before call */
        int t1 = v1 + v2 + v3 + v4;
        int t2 = v5 * v6 - v7;
        int t3 = v8 ^ v9 ^ v10;
        int t4 = v11 + v12 - v13;
        int t5 = v14 * v15 + v16;
        
        /* Call with many live variables - forces caller-saves */
        clobber_callee(&t1, &t2, &t3);
        
        /* Use results after call */
        result += t1 + t2 + t3 + t4 + t5;
        
        /* Additional computation to keep variables live */
        v17 = t1 * 2;
        v18 = t2 + 3;
        v19 = t3 ^ 0xFF;
    } else {
        /* Low pressure path */
        result = v1 + v2;
    }
    
    /* Second conditional with different structure */
    if (global_seed & 2) {
        /* Another set of live variables */
        int u1 = v21 + v22;
        int u2 = v23 * v24;
        int u3 = v25 - v26;
        int u4 = v27 ^ v28;
        int u5 = v29 + v30;
        int u6 = u1 * u2;
        int u7 = u3 ^ u4;
        int u8 = u5 + u6;
        int u9 = u7 * u8;
        int u10 = u9 - u1;
        
        /* Call at what might be block end */
        clobber_more(&u1, 5);
        
        /* Use results - keeps them live across call */
        result += u1 + u2 + u3 + u4 + u5 + u6 + u7 + u8 + u9 + u10;
        
        /* More computation */
        v30 = u10 * 2;
    }
    
    /* Loop to create multiple call sites */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        /* Varying register pressure in loop */
        int w1 = v1 + i;
        int w2 = v2 * i;
        int w3 = v3 ^ i;
        int w4 = v4 + w1;
        int w5 = v5 * w2;
        
        /* Conditional inside loop */
        if (i & 1) {
            /* Call with moderate pressure */
            clobber_callee(&w1, &w2, &w3);
            
            /* Complex use of results */
            arr[i] = w1 + w2 * w3 - w4 + w5;
        } else {
            arr[i] = w1 + w2;
        }
        
        /* Use result to prevent elimination */
        result += arr[i];
    }
    
    /* Switch statement to create more complex CFG */
    switch (global_seed % 4) {
        case 0: {
            int x1 = v10 + v11;
            int x2 = v12 * v13;
            int x3 = v14 ^ v15;
            clobber_callee(&x1, &x2, &x3);
            result += x1 + x2 + x3;
            break;
        }
        case 1: {
            int y1 = v16 + v17;
            int y2 = v18 * v19;
            int y3 = v20 ^ v21;
            int y4 = v22 + v23;
            clobber_more(&y1, 4);
            result += y1 + y2 + y3 + y4;
            break;
        }
        case 2: {
            /* No call in this path */
            result += v24 + v25 + v26;
            break;
        }
        case 3: {
            int z1 = v27 + v28;
            int z2 = v29 * v30;
            clobber_callee(&z1, &z2, &result);
            break;
        }
    }
    
    /* Final use of all variables to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    checksum += result;
    
    /* Print to prevent elimination */
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return checksum & 0xFF;
}
