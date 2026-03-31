/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to prevent optimization */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

void __attribute__((noinline, noclone)) clobber_callee(int *p) {
    /* Opaque assembly that clobbers many registers */
    asm volatile("" : : "r"(p) : CLOBBER_LIST);
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(int *p, int *q) {
    asm volatile("" : : "r"(p), "r"(q) : CLOBBER_LIST);
}

/* Function to create complex control flow with high register pressure */
int __attribute__((noinline)) high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Complex computation that cannot be optimized away */
    v1 = seed * 3;
    v2 = seed + global_seed;
    v3 = v1 ^ v2;
    v4 = v2 * 7;
    v5 = v3 - v4;
    v6 = v4 / 2;
    v7 = v5 | v6;
    v8 = v6 & v7;
    v9 = v7 ^ v8;
    v10 = v8 + v9;
    v11 = v9 * v10;
    v12 = v10 - v11;
    v13 = v11 ^ v12;
    v14 = v12 | v13;
    v15 = v13 & v14;
    v16 = v14 + v15;
    v17 = v15 * v16;
    v18 = v16 - v17;
    v19 = v17 ^ v18;
    v20 = v18 | v19;
    
    /* Call clobbering function - many registers are live across this call */
    clobber_callee(&v1);
    
    /* Use results after call to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Another call with different arguments */
    clobber_callee2(&v2, &v3);
    
    return result ^ seed;
}

/* Low pressure path for contrast */
int __attribute__((noinline)) low_pressure_path(int seed) {
    int a = seed * 2;
    int b = seed + 1;
    return a ^ b;
}

/* Main function with complex control flow */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Multiple call sites with different register pressures */
    for (int i = 0; i < 10; i++) {
        /* Complex conditional creating different basic blocks */
        if ((seed + i) % 3 == 0) {
            /* High pressure path - call at end of basic block */
            total += high_pressure_path(seed + i);
        } else if ((seed + i) % 3 == 1) {
            /* Medium pressure path */
            int x1 = seed * i + 1;
            int x2 = seed ^ i;
            int x3 = x1 * x2;
            int x4 = x2 - x1;
            int x5 = x3 | x4;
            
            /* Conditional inside conditional */
            if (x5 > 100) {
                clobber_callee(&x1);
                total += x1 + x2;
            } else {
                clobber_callee2(&x3, &x4);
                total += x3 - x4;
            }
            
            /* More computations after call */
            x5 = x5 * 2;
            total += x5;
        } else {
            /* Low pressure path */
            total += low_pressure_path(seed + i);
        }
        
        /* Switch statement for additional control flow complexity */
        switch (i % 4) {
            case 0: {
                /* Another high pressure block */
                int y1 = total * 3;
                int y2 = total + i;
                int y3 = y1 ^ y2;
                int y4 = y2 * 7;
                
                clobber_callee(&y1);
                total = y1 + y2 + y3 + y4;
                break;
            }
            case 1: {
                int z = total;
                clobber_callee2(&z, &i);
                total = z ^ i;
                break;
            }
            case 2:
                /* Simple path */
                total += 1;
                break;
            case 3:
                /* Path with volatile read */
                total += global_seed;
                break;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional test with nested loops */
    {
        int outer_sum = 0;
        for (int j = 0; j < 5; j++) {
            int inner_vars[8];
            for (int k = 0; k < 8; k++) {
                inner_vars[k] = (j * 8 + k) * seed;
            }
            
            /* Mix computations */
            for (int k = 0; k < 7; k++) {
                inner_vars[k] = inner_vars[k] ^ inner_vars[k + 1];
            }
            
            /* Call at what might be block end */
            if (j % 2 == 0) {
                clobber_callee(&inner_vars[0]);
            }
            
            for (int k = 0; k < 8; k++) {
                outer_sum += inner_vars[k];
            }
        }
        printf("Outer sum: %d\n", outer_sum);
    }
    
    return total > 0 ? 0 : 1;
}
