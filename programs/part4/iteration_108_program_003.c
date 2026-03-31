/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
#elif __riscv
#define CLOBBER_LIST "x1", "x5", "x6", "x7", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x28", "x29", "x30", "x31", "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
#else
#define CLOBBER_LIST "memory"
#endif

/* Noinline function that clobbers registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *ptr1, int *ptr2, int *ptr3) {
    /* Opaque assembly to clobber registers */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3) : CLOBBER_LIST);
    *ptr1 += 1;
    *ptr2 += 2;
    *ptr3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *fptr1, float *fptr2) {
    asm volatile ("" : : "r"(fptr1), "r"(fptr2) : CLOBBER_LIST);
    *fptr1 += 1.5f;
    *fptr2 += 2.5f;
}

/* Function with high register pressure around calls */
int __attribute__((noinline, optimize("O0")))
high_pressure_function(int seed, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 1 + iter;
    v2 = seed * 2 - iter;
    v3 = seed * 3 + iter * 2;
    v4 = seed * 5 - iter * 3;
    v5 = seed * 7 + iter * 5;
    v6 = seed * 11 - iter * 7;
    v7 = seed * 13 + iter * 11;
    v8 = seed * 17 - iter * 13;
    v9 = seed * 19 + iter * 17;
    v10 = seed * 23 - iter * 19;
    
    v11 = v1 + v2 * 3;
    v12 = v3 + v4 * 5;
    v13 = v5 + v6 * 7;
    v14 = v7 + v8 * 11;
    v15 = v9 + v10 * 13;
    v16 = v11 * v12 - v13;
    v17 = v14 * v15 - v16;
    v18 = v17 * 3 + seed;
    v19 = v18 * 5 - seed;
    v20 = v19 * 7 + iter;
    
    /* Float computations */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = f1 + f2 * 2.0f;
    f6 = f3 + f4 * 3.0f;
    f7 = f5 * f6 - 1.0f;
    f8 = f7 * 0.5f + (float)seed;
    
    /* Read volatile global to create memory barrier */
    int barrier = global_seed;
    
    /* Complex conditional with call at end of basic block */
    if ((seed ^ iter) & 0x3) {
        /* Path 1: High register pressure before call */
        /* More computations to keep variables live */
        v1 = v1 + barrier;
        v2 = v2 * barrier;
        v3 = v3 - barrier;
        v4 = v4 / (barrier | 1);
        v5 = v5 ^ barrier;
        
        f1 = f1 + (float)barrier;
        f2 = f2 * (float)(barrier | 1);
        
        /* Call that clobbers registers - this should trigger caller-save */
        clobber_callee(&v1, &v2, &v3);
        
        /* BB_END was the call, now save instruction should be inserted after */
    } else if ((seed ^ iter) & 0x4) {
        /* Path 2: Different high pressure pattern */
        v6 = v6 + barrier * 2;
        v7 = v7 * (barrier | 1);
        v8 = v8 - barrier;
        
        f3 = f3 + (float)barrier * 0.5f;
        f4 = f4 * 1.1f;
        
        /* Another call site with different register pressure */
        clobber_callee2(&f3, &f4);
        
        /* More computations after call */
        v9 = v9 + v6;
        v10 = v10 * v7;
    } else {
        /* Path 3: No call, simpler computations */
        v11 = v11 + barrier;
        v12 = v12 * 2;
    }
    
    /* Switch statement to create more basic blocks */
    switch ((seed + iter) & 0x7) {
        case 0:
            v13 = v13 + v1;
            /* Call in middle of block */
            clobber_callee(&v13, &v14, &v15);
            v16 = v16 * 2;
            break;
        case 1:
            v14 = v14 - v2;
            v15 = v15 * 3;
            /* Call at end of case block */
            clobber_callee(&v16, &v17, &v18);
            break;  /* Call is at BB_END before save insertion */
        case 2:
            v17 = v17 + v3;
            v18 = v18 / 2;
            break;
        default:
            v19 = v19 * v4;
            v20 = v20 - v5;
            /* Another call at end of default block */
            clobber_callee2(&f5, &f6);
            break;
    }
    
    /* Loop to create multiple caller-save opportunities */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Variables live across loop iteration */
        int t1 = v1 + i;
        int t2 = v2 * i;
        int t3 = v3 - i;
        float tf1 = f1 + (float)i;
        float tf2 = f2 * (float)(i + 1);
        
        /* Conditional call inside loop */
        if (i & 1) {
            clobber_callee(&t1, &t2, &t3);
        } else {
            clobber_callee2(&tf1, &tf2);
        }
        
        /* Use results */
        sum += t1 + t2 + t3 + (int)tf1 + (int)tf2;
        
        /* Update outer variables to keep them live */
        v1 += t1;
        f1 += tf1;
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                 sum + barrier;
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize seed */
    int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    global_seed = seed;
    
    int total = 0;
    
    /* Multiple iterations with different conditions */
    for (int i = 0; i < 10; i++) {
        int result = high_pressure_function(seed + i, i);
        total += result;
        
        /* Volatile operation to prevent optimization */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
