/* caller-save-test.c */
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
    *p1 = *p1 + 1;
    *p2 = *p2 + 2;
    *p3 = *p3 + 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *f1, float *f2) {
    asm volatile("" : : "r"(f1), "r"(f2) : CLOBBER_LIST);
    *f1 = *f1 * 1.5f;
    *f2 = *f2 * 2.0f;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    global_seed = argc;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = global_seed * 1;
    v2 = global_seed * 2 + argc;
    v3 = global_seed * 3 - argc;
    v4 = global_seed * 4 ^ argc;
    v5 = global_seed * 5 | argc;
    v6 = global_seed * 6 & argc;
    v7 = global_seed * 7 + (argc << 1);
    v8 = global_seed * 8 - (argc >> 1);
    v9 = global_seed * 9 ^ (argc * 3);
    v10 = global_seed * 10 | (argc + 1);
    
    v11 = v1 + v2;
    v12 = v3 - v4;
    v13 = v5 * v6;
    v14 = v7 ^ v8;
    v15 = v9 | v10;
    v16 = v11 & v12;
    v17 = v13 + v14;
    v18 = v15 - v16;
    v19 = v17 * v18;
    v20 = v19 ^ (v1 + v2 + v3);
    
    /* Float computations */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = f1 + f2;
    f6 = f3 - f4;
    f7 = f5 * f6;
    f8 = f7 / 2.0f;
    
    /* Create multiple basic blocks with different register pressure */
    for (int i = 0; i < 3; i++) {
        /* Complex condition creating separate basic blocks */
        if ((global_seed + i) % 3 == 0) {
            /* HIGH REGISTER PRESSURE PATH - many live variables across call */
            
            /* More computations keeping variables live */
            v1 = v20 + i;
            v2 = v19 - i;
            v3 = v18 * i;
            v4 = v17 ^ i;
            v5 = v16 | i;
            
            f1 = f8 + (float)i;
            f2 = f7 - (float)i;
            
            /* Call that clobbers registers with many live variables */
            clobber_callee(&v1, &v2, &v3);
            
            /* Use results immediately to keep them live */
            v6 = v1 + v4;
            v7 = v2 - v5;
            v8 = v3 ^ v6;
            
            /* Another call with float live variables */
            clobber_callee2(&f1, &f2);
            
            f3 = f1 * f2;
            f4 = f3 + f8;
            
        } else if ((global_seed + i) % 3 == 1) {
            /* MEDIUM PRESSURE PATH */
            v9 = v15 + i;
            v10 = v14 - i;
            clobber_callee(&v9, &v10, &v13);
            v11 = v9 * v10;
        } else {
            /* LOW PRESSURE PATH - simpler computation, no call */
            v12 = v13 ^ i;
            v13 = v12 * 2;
        }
        
        /* Mix in some volatile operations to prevent reordering */
        {
            volatile int barrier = global_seed;
            v1 = v1 + barrier;
            v2 = v2 - barrier;
        }
        
        /* Switch statement creating more basic blocks */
        switch (i) {
            case 0:
                /* Another call site at potential block end */
                if (v1 > 0) {
                    v14 = v1 + v2;
                    v15 = v3 - v4;
                    clobber_callee(&v14, &v15, &v5);
                    v16 = v14 * v15;
                }
                break;
            case 1:
                /* Different call pattern */
                v17 = v6 ^ v7;
                v18 = v8 | v9;
                clobber_callee(&v17, &v18, &v10);
                break;
            default:
                /* Computation without call */
                v19 = v11 & v12;
                v20 = v13 | v14;
        }
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum % 256;
}
