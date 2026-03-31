/* Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer -c test.c */
/* Or for RISC-V: riscv32-unknown-elf-gcc -O2 -march=rv32imc -fno-schedule-insns -fno-inline -c test.c */

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

void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc to create input-dependent but deterministic behavior */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    volatile int vol_seed = seed; /* Prevent constant propagation */
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 3; iter++) {
        /* Declare MANY local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Initialize with complex arithmetic to prevent optimization */
        v1 = vol_seed + iter;
        v2 = v1 * 3;
        v3 = v2 - 17;
        v4 = v3 ^ 0xABCD;
        v5 = v4 * v1;
        v6 = v5 / (v2 + 1);
        v7 = v6 << 3;
        v8 = v7 | 0xFF;
        v9 = v8 & 0x3F;
        v10 = v9 * v3;
        
        v11 = v10 + global_seed;
        v12 = v11 * 2;
        v13 = v12 - v1;
        v14 = v13 ^ v4;
        v15 = v14 * 5;
        v16 = v15 / (v6 + 1);
        v17 = v16 >> 2;
        v18 = v17 | v8;
        v19 = v18 & 0xF0F0;
        v20 = v19 * v11;
        
        v21 = v20 + iter * 7;
        v22 = v21 * 11;
        v23 = v22 - v13;
        v24 = v23 ^ v14;
        v25 = v24 * 13;
        v26 = v25 / (v16 + 1);
        v27 = v26 << 1;
        v28 = v27 | v18;
        v29 = v28 & 0xAAAA;
        v30 = v29 * v21;
        
        /* Create conditional branch where one path has high register pressure */
        if ((vol_seed + iter) % 3 != 0) {
            /* High-pressure path: call with many live variables */
            
            /* Use getchar to create side effects and inhibit optimizations */
            int c = getchar();
            if (c != EOF) {
                v1 += c;
            }
            
            /* Call clobbering function - many variables are live across this call */
            clobber_callee(&v1, &v2, &v3);
            
            /* More computations to keep variables live */
            v4 = v1 + v2 + v3;
            v5 = v4 * v30;
            v6 = v5 - v29;
            
            /* Another call with different arguments */
            int arr[4] = {v7, v8, v9, v10};
            clobber_more(arr, 4);
            v7 = arr[0];
            v8 = arr[1];
            
            /* Complex use of variables */
            v30 = v30 + v1 * v2 - v3 / (v4 + 1) + v5 ^ v6 | v7 & v8;
        } else {
            /* Low-pressure path */
            v30 = v1 + v2;
        }
        
        /* Use all variables in a complex computation to keep them live */
        int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Mix in global volatile */
        checksum ^= global_seed;
        
        /* Use checksum to prevent dead code elimination */
        result += checksum;
        
        /* Modify seed for next iteration */
        vol_seed = (vol_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Switch statement to create more complex control flow */
    switch (result % 5) {
        case 0: {
            int a = result * 2, b = result / 2, c = result + 1;
            clobber_callee(&a, &b, &c);
            result = a + b + c;
            break;
        }
        case 1: {
            int x = result ^ 0xFF, y = result << 3, z = result >> 1;
            int arr[3] = {x, y, z};
            clobber_more(arr, 3);
            result = arr[0] + arr[1] + arr[2];
            break;
        }
        case 2:
            result = ~result;
            break;
        case 3: {
            /* Nested condition with call at block end */
            if (result > 1000) {
                int p = result * 3, q = result - 100;
                clobber_callee(&p, &q, &result);
                /* Call is at block end here - good candidate for BB_END */
            }
            result += 42;
            break;
        }
        default:
            result = 0;
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
