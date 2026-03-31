/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_seed;

/* Function that clobbers many registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Force register clobbering with inline asm */
    #ifdef __x86_64__
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    #elif __i386__
    asm volatile("" ::: "eax", "ecx", "edx", "esi", "edi");
    #elif __riscv
    asm volatile("" ::: "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7");
    #endif
    
    /* Opaque memory operations */
    if (p1) *p1 ^= 0x55;
    if (p2) *p2 ^= 0xAA;
    if (p3) *p3 ^= 0xFF;
    if (p4) *p4 ^= 0x33;
}

/* Another clobbering function with different signature */
int __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2) {
    #ifdef __x86_64__
    asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    #elif __i386__
    asm volatile("" ::: "st", "st(1)", "st(2)", "st(3)");
    #endif
    
    volatile int r = global_seed;
    if (f1) *f1 += r;
    if (f2) *f2 -= r;
    return r;
}

int main(int argc, char **argv) {
    /* Use argc for deterministic but variable behavior */
    volatile int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    global_seed = seed;
    
    int checksum = 0;
    
    /* Loop to create multiple call sites */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Declare MANY local variables to create register pressure */
        int v1 = seed + iteration * 1;
        int v2 = seed + iteration * 2;
        int v3 = seed + iteration * 3;
        int v4 = seed + iteration * 4;
        int v5 = seed + iteration * 5;
        int v6 = seed + iteration * 6;
        int v7 = seed + iteration * 7;
        int v8 = seed + iteration * 8;
        int v9 = seed + iteration * 9;
        int v10 = seed + iteration * 10;
        int v11 = seed + iteration * 11;
        int v12 = seed + iteration * 12;
        int v13 = seed + iteration * 13;
        int v14 = seed + iteration * 14;
        int v15 = seed + iteration * 15;
        int v16 = seed + iteration * 16;
        
        /* Float variables to potentially use FP registers */
        float f1 = seed * 0.1f;
        float f2 = seed * 0.2f;
        float f3 = seed * 0.3f;
        float f4 = seed * 0.4f;
        
        /* Complex computation that can't be optimized away */
        v1 = v1 * v2 + v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 + v6 - v7;
        v4 = v4 * v8 / (v9 + 1);
        v5 = (v5 << 3) | (v10 >> 2);
        v6 = v6 & v11 ^ v12;
        v7 = v7 + v13 * v14;
        v8 = v8 - v15 + v16;
        
        f1 = f1 * f2 + f3;
        f2 = f2 - f4 * 0.5f;
        f3 = f3 / (f1 + 1.0f);
        f4 = f4 + f2 * 2.0f;
        
        /* Read volatile global to create memory barrier */
        volatile int barrier = global_seed;
        v1 ^= barrier;
        v2 += barrier;
        
        /* Conditional with different register pressure paths */
        if ((seed + iteration) % 3 == 0) {
            /* HIGH REGISTER PRESSURE PATH - call at end of basic block */
            
            /* More computations to keep variables live */
            v9 = v1 + v2 + v3 + v4;
            v10 = v5 * v6 - v7;
            v11 = v8 ^ v9 & v10;
            v12 = v11 | v13 ^ v14;
            
            /* Call that clobbers registers - this should be BB_END before save insertion */
            clobber_callee(&v1, &v2, &v3, &v4);
            
            /* This creates a new basic block after the call */
            if (v9 > 1000) {
                v9 = v9 / 2;
            }
        } 
        else if ((seed + iteration) % 3 == 1) {
            /* MEDIUM PRESSURE PATH with different call pattern */
            v13 = v15 + v16 * v1;
            v14 = v2 - v3 ^ v4;
            
            clobber_callee2(&f1, &f2);
            
            v15 = v13 | v14;
        }
        else {
            /* LOW PRESSURE PATH - no call, simpler computation */
            v16 = v1 + v2;
        }
        
        /* Use all variables after conditional to keep them live across calls */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        checksum += v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
        checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        
        /* Another call site inside the loop but with different live vars */
        if (iteration % 2 == 0) {
            int temp1 = v1 + v3;
            int temp2 = v2 + v4;
            clobber_callee(&temp1, &temp2, NULL, NULL);
            checksum += temp1 + temp2;
        }
    }
    
    /* Final call with different register pressure */
    int final1 = checksum * 2;
    int final2 = checksum / 3;
    float final3 = checksum * 0.25f;
    
    clobber_callee(&final1, &final2, NULL, NULL);
    clobber_callee2(&final3, NULL);
    
    checksum = final1 + final2 + (int)final3;
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}
