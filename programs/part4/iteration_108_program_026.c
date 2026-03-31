/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific x86 registers */
    asm volatile("" 
                 : 
                 : "r"(*p1), "r"(*p2), "r"(*p3), "r"(*p4)
                 : "eax", "ecx", "edx", "esi", "edi", "memory");
    
    /* Additional memory clobber to force spills */
    *p1 += *p2;
    *p3 ^= *p4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    asm volatile("" 
                 : 
                 : "r"(*f1), "r"(*f2), "r"(*i1)
                 : "xmm0", "xmm1", "xmm2", "eax", "ecx", "memory");
    
    *f1 = *f2 * 2.0f;
    *i1 += (int)(*f1);
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x1234;
    int v4 = seed - 100;
    int v5 = seed * seed;
    int v6 = v1 + v2;
    int v7 = v3 ^ v4;
    int v8 = v5 % 17;
    int v9 = v6 * v7;
    int v10 = v8 + v9;
    
    float f1 = (float)v1 * 1.5f;
    float f2 = (float)v2 * 2.5f;
    float f3 = (float)v3 * 0.5f;
    float f4 = (float)v4 * 3.5f;
    
    /* Use volatile read to prevent optimization */
    volatile int vol_read = global_seed;
    int v11 = v10 + vol_read;
    int v12 = v11 * v1;
    int v13 = v12 ^ v2;
    int v14 = v13 + v3;
    int v15 = v14 * v4;
    int v16 = v15 ^ v5;
    int v17 = v16 + v6;
    int v18 = v17 * v7;
    int v19 = v18 ^ v8;
    int v20 = v19 + v9;
    
    /* Call clobbering function with many live variables */
    clobber_callee(&v11, &v12, &v13, &v14);
    
    /* More computations to keep variables live */
    f1 += f2 * f3;
    f4 = f1 / f2;
    
    /* Another call with float registers */
    clobber_callee2(&f1, &f2, &v15);
    
    /* Use all variables in final computation */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    return result;
}

int low_pressure_path(int seed) {
    /* Simpler path with fewer live variables */
    int a = seed * 3;
    int b = seed + 7;
    int c = a ^ b;
    return c * 2;
}

/* Main function with complex control flow */
int main(int argc, char **argv) {
    int i, j;
    int total = 0;
    
    /* Use argc as seed for deterministic but variable behavior */
    int seed = argc;
    
    /* Loop to create multiple call sites */
    for (i = 0; i < 100; i++) {
        /* Complex condition to create different basic blocks */
        if ((seed + i) % 3 == 0) {
            /* High pressure path - many live variables across call */
            int result = high_pressure_path(seed + i);
            total += result;
            
            /* Nested condition to create more complex CFG */
            if (result % 2 == 0) {
                /* Another call site in a different block */
                int temp1 = result * 2;
                int temp2 = result + 5;
                int temp3 = temp1 ^ temp2;
                int temp4 = temp3 * 3;
                clobber_callee(&temp1, &temp2, &temp3, &temp4);
                total += temp4;
            }
        } else if ((seed + i) % 3 == 1) {
            /* Medium pressure path */
            int x1 = seed * i;
            int x2 = x1 + 10;
            int x3 = x2 ^ 0xABCD;
            int x4 = x3 * 2;
            int x5 = x4 - i;
            
            clobber_callee(&x1, &x2, &x3, &x4);
            
            total += x1 + x2 + x3 + x4 + x5;
        } else {
            /* Low pressure path */
            total += low_pressure_path(seed + i);
        }
        
        /* Switch statement to create more control flow complexity */
        switch (i % 4) {
            case 0: {
                /* Case with another call */
                int case_var = total ^ i;
                float fcase = (float)case_var;
                clobber_callee2(&fcase, &fcase, &case_var);
                total += case_var;
                break;
            }
            case 1:
                total += i * 2;
                break;
            case 2: {
                /* Another high pressure block */
                int y1 = total + i;
                int y2 = y1 * 3;
                int y3 = y2 ^ 0x1234;
                int y4 = y3 - i;
                clobber_callee(&y1, &y2, &y3, &y4);
                total += y4;
                break;
            }
            default:
                total -= i;
                break;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional loop with different pattern */
    for (j = 0; j < 50; j++) {
        volatile int loop_seed = global_seed + j;
        
        /* Many variables in loop to create different pressure */
        int l1 = loop_seed * j;
        int l2 = l1 + 100;
        int l3 = l2 ^ 0xF0F0;
        int l4 = l3 * 2;
        int l5 = l4 - j;
        int l6 = l5 ^ l1;
        int l7 = l6 + l2;
        int l8 = l7 * 3;
        int l9 = l8 ^ l3;
        int l10 = l9 + l4;
        
        /* Call at what might be block end */
        if (l1 > l2) {
            clobber_callee(&l1, &l2, &l3, &l4);
            total += l1 + l2;
        } else {
            clobber_callee2((float*)&l5, (float*)&l6, &l7);
            total += l3 + l4;
        }
        
        /* Use remaining variables */
        total += l5 + l6 + l7 + l8 + l9 + l10;
    }
    
    printf("Final result: %d\n", total);
    return total != 0;
}
