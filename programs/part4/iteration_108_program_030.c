/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
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
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Modify through pointers to create side effects */
    if (p1) *p1 ^= 0x1234;
    if (p2) *p2 ^= 0x5678;
    if (p3) *p3 ^= 0x9ABC;
    if (p4) *p4 ^= 0xDEF0;
}

/* Another clobbering function with different signature */
int __attribute__((noinline, noclone))
another_callee(int a, int b, int c, int d, int e, int f) {
    volatile int sink;
    sink = a + b + c + d + e + f;
    
    #ifdef __i386__
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #endif
    
    return sink ^ global_seed;
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
    v4 = (v2 ^ v3) & 0xFF;
    v5 = v4 * v3 - v2;
    v6 = v5 / (seed | 1);
    v7 = v6 << 3;
    v8 = v7 >> 1;
    v9 = v8 | v7;
    v10 = v9 & v8;
    
    v11 = v10 * 11;
    v12 = v11 - seed;
    v13 = v12 ^ v11;
    v14 = v13 + argc * 14;
    v15 = v14 * v13;
    v16 = v15 % 256;
    v17 = v16 | 0xABCD;
    v18 = v17 & 0x1234;
    v19 = v18 + v17;
    v20 = v19 * v18;
    
    v21 = v20 / (argc + 2);
    v22 = v21 << 2;
    v23 = v22 >> 1;
    v24 = v23 ^ v22;
    v25 = v24 + v23;
    v26 = v25 * 26;
    v27 = v26 - seed;
    v28 = v27 ^ 0xDEADBEEF;
    v29 = v28 & 0x7FFFFFFF;
    v30 = v29 + argc * 30;
    
    /* Create conditional branch where one path has high register pressure */
    if (seed & 0x1) {
        /* High pressure path: many variables live across call */
        
        /* More computations to keep variables in registers */
        v1 = v30 + v1;
        v2 = v29 + v2;
        v3 = v28 + v3;
        v4 = v27 + v4;
        v5 = v26 + v5;
        v6 = v25 + v6;
        v7 = v24 + v7;
        v8 = v23 + v8;
        v9 = v22 + v9;
        v10 = v21 + v10;
        
        /* Call function that clobbers registers - many variables are live */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* Use results after call to keep them live */
        v11 = v1 + v2 + v3 + v4;
        v12 = v5 + v6 + v7 + v8;
        v13 = v9 + v10 + v11 + v12;
        
        /* Another call with different live variables */
        int result = another_callee(v13, v14, v15, v16, v17, v18);
        
        v19 = v19 + result;
        v20 = v20 ^ result;
        
    } else {
        /* Low pressure path */
        v1 = v1 + 1;
        v2 = v2 - 1;
    }
    
    /* Second conditional to create another basic block with call at end */
    if (seed & 0x2) {
        /* Different set of live variables */
        int t1 = v21 + v22;
        int t2 = v23 + v24;
        int t3 = v25 + v26;
        int t4 = v27 + v28;
        
        /* Call at what might be end of basic block */
        clobber_callee(&t1, &t2, &t3, &t4);
        
        v29 = t1 + t2 + t3 + t4;
        v30 = v29 * 2;
    }
    
    /* Loop to create multiple call sites */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Variables that are live across loop iteration */
        int loop_var1 = v1 + i;
        int loop_var2 = v2 * i;
        int loop_var3 = v3 ^ i;
        int loop_var4 = v4 - i;
        int loop_var5 = v5 + i * 2;
        int loop_var6 = v6 * (i + 1);
        
        /* Call inside loop - creates another caller-save site */
        if (i & 0x1) {
            clobber_callee(&loop_var1, &loop_var2, &loop_var3, &loop_var4);
        } else {
            int r = another_callee(loop_var1, loop_var2, loop_var3, 
                                  loop_var4, loop_var5, loop_var6);
            loop_var5 = r;
        }
        
        /* Use results to keep them live */
        sum += loop_var1 + loop_var2 + loop_var3 + loop_var4 + loop_var5 + loop_var6;
        
        /* Volatile memory operation to prevent reordering */
        global_seed = global_seed + 1;
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    int final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                      sum + global_seed;
    
    printf("Result: %d\n", final_result);
    
    /* Additional switch statement to create complex control flow */
    switch (final_result & 0x3) {
        case 0: {
            int s1 = final_result * 2;
            int s2 = final_result / 2;
            clobber_callee(&s1, &s2, &v1, &v2);
            printf("Case 0: %d\n", s1 + s2);
            break;
        }
        case 1: {
            int s3 = final_result ^ 0xAAAA;
            int s4 = final_result | 0x5555;
            another_callee(s3, s4, v3, v4, v5, v6);
            printf("Case 1: %d\n", s3 - s4);
            break;
        }
        default:
            printf("Default case\n");
    }
    
    return final_result & 0xFF;
}
