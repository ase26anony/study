/* Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer -c test.c */
/* For RISC-V: riscv32-unknown-elf-gcc -O2 -march=rv32imc -fno-schedule-insns -fno-inline -c test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers */
void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers */
    #ifdef __i386__
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #elif __riscv
    asm volatile("" : : : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
    #endif
    
    /* Opaque operations to use the pointers */
    if (p1) *p1 += 1;
    if (p2) *p2 ^= 0x55;
    if (p3) *p3 *= 2;
    if (p4) *p4 -= 3;
}

/* Another clobbering function with different signature */
int __attribute__((noinline, noclone))
clobber_callee2(int a, int b, int c, int d, int e, int f, int g, int h) {
    int result;
    #ifdef __i386__
    asm volatile("" : "=r"(result) : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #elif __riscv
    asm volatile("" : "=r"(result) : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h)
                 : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory");
    #else
    result = a + b + c + d + e + f + g + h;
    #endif
    return result;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    global_seed = argc;
    
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent optimization */
    v1 = global_seed + 1;
    v2 = global_seed * 2;
    v3 = global_seed ^ 0x1234;
    v4 = global_seed - 5;
    v5 = global_seed / 2;
    v6 = global_seed % 7;
    v7 = v1 + v2;
    v8 = v3 * v4;
    v9 = v5 ^ v6;
    v10 = v7 - v8;
    
    v11 = v9 * v10;
    v12 = v1 + v11;
    v13 = v2 * v12;
    v14 = v3 + v13;
    v15 = v4 * v14;
    v16 = v5 + v15;
    v17 = v6 * v16;
    v18 = v7 + v17;
    v19 = v8 * v18;
    v20 = v9 + v19;
    
    v21 = v10 * v20;
    v22 = v11 + v21;
    v23 = v12 * v22;
    v24 = v13 + v23;
    v25 = v14 * v24;
    v26 = v15 + v25;
    v27 = v16 * v26;
    v28 = v17 + v27;
    v29 = v18 * v28;
    v30 = v19 + v29;
    
    /* Create conditional branch where one path has high register pressure */
    if (global_seed > 2) {
        /* High register pressure path - many variables live across call */
        
        /* More computations to increase live range */
        v1 = v30 + v1;
        v2 = v29 * v2;
        v3 = v28 ^ v3;
        v4 = v27 - v4;
        v5 = v26 / (v5 ? v5 : 1);
        v6 = v25 % (v6 ? v6 : 1);
        
        /* Call that clobbers many registers with many live variables */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* Use results immediately to keep them live */
        v7 = v1 + v2;
        v8 = v3 * v4;
        
        /* Another call with different register pressure */
        v9 = clobber_callee2(v5, v6, v7, v8, v9, v10, v11, v12);
        
        /* More computations after call */
        v13 = v9 * v10;
        v14 = v11 + v13;
        
        /* Third call site */
        clobber_callee(&v15, &v16, &v17, &v18);
        
        v19 = v15 + v16;
        v20 = v17 * v18;
    } else {
        /* Low pressure path - simpler computations */
        v1 = v2 + v3;
        v4 = v5 * v6;
    }
    
    /* Loop to create multiple basic blocks with calls at different positions */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Varying condition to create different block structures */
        if ((global_seed + i) % 2) {
            /* Call at what might be block end */
            int temp = clobber_callee2(v1 + i, v2 + i, v3 + i, v4 + i, 
                                      v5 + i, v6 + i, v7 + i, v8 + i);
            sum += temp;
            
            /* More live variables across another call */
            v9 = v10 + temp;
            v11 = v12 * temp;
            
            clobber_callee(&v9, &v11, &v13, &v14);
            
            v15 = v9 + v11;
        } else {
            /* Different computation without call */
            sum += v1 + v2 + v3;
        }
        
        /* Switch statement to create more complex control flow */
        switch (i) {
            case 0:
                /* Call in one case */
                v16 = clobber_callee2(v17, v18, v19, v20, v21, v22, v23, v24);
                break;
            case 1:
                /* Different computation */
                v16 = v25 + v26;
                break;
            default:
                /* Another call */
                clobber_callee(&v27, &v28, &v29, &v30);
                v16 = v27 * v28;
        }
        
        sum += v16;
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    int final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                      sum;
    
    printf("Result: %d\n", final_result);
    
    /* Additional call sites in different control flow contexts */
    {
        int a = final_result;
        int b = a * 2;
        int c = b + 1;
        int d = c ^ 0xAA;
        
        /* Nested conditionals */
        if (a > 0) {
            if (b > 10) {
                /* Call at potential block end */
                clobber_callee(&a, &b, &c, &d);
            } else {
                c = d * 3;
            }
            /* Another call after the if */
            b = clobber_callee2(a, b, c, d, a, b, c, d);
        }
        
        printf("Final: %d\n", a + b + c + d);
    }
    
    return final_result & 0xFF;
}
