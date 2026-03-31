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
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    #ifdef __i386__
    asm volatile("" 
                 : /* no outputs */
                 : "r"(*p1), "r"(*p2), "r"(*p3), "r"(*p4)
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" ::: "memory");
    #endif
    
    /* Opaque operation to prevent dead code elimination */
    *p1 ^= *p2;
    *p3 |= *p4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" 
                 : /* no outputs */
                 : "r"(*f1), "r"(*f2), "r"(*i1)
                 : "eax", "ecx", "edx", "memory");
    #else
    asm volatile("" ::: "memory");
    #endif
    
    *f1 = *f2 + 1.0f;
    *i1 += (int)(*f1);
}

/* Function to create complex control flow */
int __attribute__((noinline))
complex_control_flow(int seed, int mode) {
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    volatile int init = global_seed + seed;
    
    v1 = init * 1;
    v2 = init * 2 + v1;
    v3 = init * 3 ^ v2;
    v4 = init * 4 | v3;
    v5 = init * 5 & v4;
    v6 = init * 6 + v5;
    v7 = init * 7 - v6;
    v8 = init * 8 ^ v7;
    v9 = init * 9 | v8;
    v10 = init * 10 & v9;
    
    v11 = v1 + v2;
    v12 = v3 - v4;
    v13 = v5 * v6;
    v14 = v7 ^ v8;
    v15 = v9 | v10;
    v16 = v11 & v12;
    v17 = v13 + v14;
    v18 = v15 - v16;
    v19 = v17 * v18;
    v20 = v19 ^ init;
    
    /* Floating point variables also need registers */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 2.2f;
    f3 = (float)v3 * 3.3f;
    f4 = (float)v4 * 4.4f;
    f5 = (float)v5 * 5.5f;
    
    /* Complex conditional to create basic block boundaries */
    if (mode == 0) {
        /* Path 1: High register pressure before call */
        /* Use most variables in computation before call */
        int temp1 = v1 + v2 + v3 + v4 + v5;
        int temp2 = v6 * v7 - v8 + v9 ^ v10;
        float ftemp = f1 * f2 - f3 + f4 / f5;
        
        /* Call with many live variables - forces caller-save */
        clobber_callee(&v11, &v12, &v13, &v14);
        
        /* Use results after call - keeps variables live across call */
        v15 = temp1 + v11;
        v16 = temp2 ^ v12;
        f1 = ftemp + (float)v13;
    } 
    else if (mode == 1) {
        /* Path 2: Different register pressure pattern */
        int temp3 = v15 + v16 + v17;
        float ftemp2 = f3 * f4;
        
        /* Another clobbering call */
        clobber_callee2(&f1, &f2, &v18);
        
        v19 = temp3 + v18;
        f5 = ftemp2;
    }
    else {
        /* Path 3: No call, but still use variables */
        v20 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    /* Switch statement to create more control flow */
    switch (seed & 0x3) {
        case 0:
            /* Another call site at end of basic block */
            clobber_callee(&v19, &v20, &v1, &v2);
            v3 = v19 + v20;
            break;
        case 1:
            v3 = v1 * v2 - v3;
            break;
        case 2:
            /* Call with different arguments */
            clobber_callee2(&f3, &f4, &v4);
            v3 = v4 + (int)f3;
            break;
        default:
            v3 = v5 ^ v6;
    }
    
    /* Final computation using all variables to prevent elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Function with loop creating multiple call sites */
int __attribute__((noinline))
multiple_call_sites(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying conditions create different basic blocks */
        int mode = i % 3;
        
        /* Each iteration creates new register pressure scenario */
        int result = complex_control_flow(i, mode);
        
        /* Use volatile to prevent loop optimizations */
        volatile int prevent_opt = result;
        total += prevent_opt ^ i;
        
        /* Occasionally call another function */
        if ((i & 7) == 0) {
            int temp1 = total * 2;
            int temp2 = total ^ 0xABCD;
            clobber_callee(&temp1, &temp2, &total, &i);
        }
    }
    
    return total;
}

int main(int argc, char **argv) {
    /* Use argc for deterministic but varying behavior */
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    /* Initialize global volatile */
    global_seed = seed & 0xFF;
    
    /* Create high register pressure scenarios */
    int result1 = complex_control_flow(seed, 0);
    int result2 = complex_control_flow(seed + 1, 1);
    int result3 = complex_control_flow(seed + 2, 2);
    
    /* Multiple call sites with loop */
    int result4 = multiple_call_sites(10);
    
    /* Final result prevents dead code elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
