/* Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                  : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Do some actual work to prevent removal */
    *p1 = *p2 + *p3;
    *p4 = *p1 ^ *p2;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1)
                  : "eax", "ebx", "ecx", "edx", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    *f1 = *f2 * 2.0f;
    *i1 = (int)(*f1);
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    global_seed = argc;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = global_seed + 1;
    v2 = global_seed * 2;
    v3 = v1 + v2;
    v4 = v2 - v1;
    v5 = v3 * v4;
    v6 = v5 / (v1 | 1);  /* Avoid division by zero */
    v7 = v6 ^ v5;
    v8 = v7 & v6;
    v9 = v8 | v7;
    v10 = v9 << 2;
    
    /* More computations creating data dependencies */
    v11 = v10 + argc;
    v12 = v11 * 3;
    v13 = v12 - v11;
    v14 = v13 ^ v12;
    v15 = v14 & 0xFF;
    v16 = v15 | 0x80;
    v17 = v16 >> 1;
    v18 = v17 + v16;
    v19 = v18 * v17;
    v20 = v19 - v18;
    
    /* Float computations */
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = f1 + f2;
    f4 = f2 - f1;
    f5 = f3 * f4;
    
    /* Create conditional branch where one path has high register pressure */
    if (global_seed > 1) {
        /* High register pressure path - many live variables across call */
        
        /* Use volatile read to prevent moving computations */
        volatile int barrier = global_seed;
        
        /* Complex computation using many variables before call */
        int t1 = v1 + v2 + v3 + v4 + v5;
        int t2 = v6 + v7 + v8 + v9 + v10;
        int t3 = v11 + v12 + v13 + v14 + v15;
        float t4 = f1 + f2 + f3 + f4 + f5;
        
        /* Call that clobbers registers with many live variables */
        clobber_callee(&t1, &t2, &v20, &v19);
        
        /* Use results after call to keep variables live */
        v1 = t1 + barrier;
        v2 = t2 - barrier;
        
        /* Another call with different register pressure */
        clobber_callee2(&f1, &f2, &v3);
        
        /* More computations after call */
        v4 = v1 * v2 + v3;
        v5 = v4 ^ v1;
        
        /* Nested condition to create more basic blocks */
        if (barrier > 2) {
            /* Another call site with different live set */
            int t5 = v5 + v6 + v7;
            int t6 = v8 + v9 + v10;
            clobber_callee(&t5, &t6, &v11, &v12);
            v5 = t5;
            v6 = t6;
        }
    } else {
        /* Low pressure path */
        v1 = argc;
        v2 = argc * 2;
    }
    
    /* Loop to create multiple caller-save opportunities */
    int checksum = 0;
    for (int i = 0; i < 3; i++) {
        /* Varying conditions create different basic blocks */
        if (i == 0) {
            /* Call at end of basic block with moderate pressure */
            int tmp1 = v1 + v2 + i;
            int tmp2 = v3 + v4 + i;
            clobber_callee(&tmp1, &tmp2, &v5, &v6);
            checksum += tmp1 + tmp2;
        } else if (i == 1) {
            /* Different call with float registers live */
            float ftmp = f1 + f2 + (float)i;
            int itmp = v7 + v8 + i;
            clobber_callee2(&ftmp, &f3, &itmp);
            checksum += itmp + (int)ftmp;
        } else {
            /* Call with many variables live */
            int sum1 = v1 + v3 + v5 + v7 + v9;
            int sum2 = v2 + v4 + v6 + v8 + v10;
            clobber_callee(&sum1, &sum2, &v11, &v12);
            checksum += sum1 + sum2;
            
            /* Additional computation to create another basic block */
            if (checksum > 100) {
                int extra = v13 + v14 + v15;
                clobber_callee(&extra, &v16, &v17, &v18);
                checksum += extra;
            }
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    checksum += (int)(f1 + f2 + f3 + f4 + f5);
    
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}
