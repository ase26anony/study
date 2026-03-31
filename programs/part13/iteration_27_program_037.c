/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force specific register usage */
register uint64_t reg_var1 asm ("r12");
register uint64_t reg_var2 asm ("r13");
register uint64_t reg_var3 asm ("r14");

/* Volatile variables to prevent optimization */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;
volatile int v6 = 6;
volatile int v7 = 7;
volatile int v8 = 8;
volatile int v9 = 9;
volatile int v10 = 10;

volatile double f1 = 1.1;
volatile double f2 = 2.2;
volatile double f3 = 3.3;
volatile double f4 = 4.4;

/* Non-volatile variables with complex live ranges */
int nv1, nv2, nv3, nv4, nv5, nv6, nv7, nv8, nv9, nv10;
double d1, d2, d3, d4, d5, d6;

int main(void) {
    int result = 0;
    volatile int checksum = 0;
    
    /* Initialize explicit register variables */
    reg_var1 = 0x12345678;
    reg_var2 = 0x87654321;
    reg_var3 = 0xABCDEF01;
    
    /* Initialize non-volatile variables */
    nv1 = v1 * 2;
    nv2 = v2 * 3;
    nv3 = v3 * 4;
    nv4 = v4 * 5;
    nv5 = v5 * 6;
    nv6 = v6 * 7;
    nv7 = v7 * 8;
    nv8 = v8 * 9;
    nv9 = v9 * 10;
    nv10 = v10 * 11;
    
    d1 = f1 * 2.0;
    d2 = f2 * 3.0;
    d3 = f3 * 4.0;
    d4 = f4 * 5.0;
    d5 = d1 + d2;
    d6 = d3 + d4;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = nv1 + nv2;
        int temp2 = nv3 + nv4;
        int temp3 = nv5 + nv6;
        
        /* Inline assembly that uses many registers and has conflicting constraints */
        asm volatile (
            /* Output operands with earlyclobber to force separate registers */
            "mov %[in1], %%rax\n\t"
            "add %[in2], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "mov %[in3], %%rbx\n\t"
            "imul %[in4], %%rbx\n\t"
            "mov %%rbx, %[out2]\n\t"
            /* Clobber many registers to force spills */
            :
            [out1] "=&r" (temp1),  /* Earlyclobber constraint */
            [out2] "=&r" (temp2)    /* Earlyclobber constraint */
            :
            [in1] "r" (nv7),        /* Register constraint */
            [in2] "r" (nv8),        /* Register constraint */
            [in3] "r" (nv9),        /* Register constraint */
            [in4] "r" (nv10)        /* Register constraint */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        nv1 = temp1;
        nv2 = temp2;
        result += temp3;
    }
    
    /* Block 2: Mixed data types and addressing modes */
block2:
    {
        short s1 = (short)v1;
        char c1 = (char)v2;
        long l1 = (long)v3 * 1000;
        
        /* Take address of variables while also using them in registers */
        int* ptr1 = &nv3;
        int* ptr2 = &nv4;
        
        /* Inline assembly with memory and register constraints on same variable */
        asm volatile (
            "mov (%[mem1]), %%eax\n\t"
            "add (%[mem2]), %%eax\n\t"
            "add %[reg1], %%eax\n\t"
            "mov %%eax, %[out]\n\t"
            "mov %[reg2], %%ebx\n\t"
            "add %%ebx, %[memout]\n\t"
            :
            [out] "=r" (s1),
            [memout] "+m" (*ptr1)  /* Read-write memory constraint */
            :
            [mem1] "r" (ptr2),     /* Pointer in register */
            [mem2] "r" (&nv5),     /* Address in register */
            [reg1] "r" (c1),       /* Char in register */
            [reg2] "r" (l1)        /* Long in register */
            : "rax", "rbx", "rcx", "memory"
        );
        
        result += s1 + *ptr1;
        
        /* Force use of explicit register variables that might be clobbered */
        asm volatile (
            "add %%r12, %%r13\n\t"
            "mov %%r13, %0\n\t"
            : "=r" (l1)
            : 
            : "r12", "r13", "r14"
        );
        
        reg_var1 = l1;
    }
    
    /* Block 3: Floating point with integer mix */
block3:
    {
        double temp_d = d5;
        int temp_i = nv5;
        float temp_f = (float)f3;
        
        /* Inline assembly mixing float and integer operations */
        asm volatile (
            "cvtsi2sd %[intval], %%xmm0\n\t"
            "addsd %[dblval], %%xmm0\n\t"
            "cvtsd2si %%xmm0, %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "movss %[fltval], %%xmm1\n\t"
            "cvtss2sd %%xmm1, %%xmm1\n\t"
            "movsd %%xmm1, %[out2]\n\t"
            :
            [out1] "=r" (temp_i),
            [out2] "=m" (temp_d)   /* Memory output for double */
            :
            [intval] "r" (nv6),
            [dblval] "x" (d6),     /* XMM register constraint */
            [fltval] "x" (temp_f)  /* XMM register constraint */
            : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        d5 = temp_d;
        nv5 = temp_i;
        result += temp_i;
    }
    
    /* Block 4: Complex control flow with live values */
    if (result > 0) {
        goto block4;
    } else {
        goto block5;
    }
    
block4:
    {
        /* Many live variables across this block */
        int live1 = nv1;
        int live2 = nv2;
        int live3 = nv3;
        int live4 = nv4;
        int live5 = nv5;
        
        /* Inline assembly that uses all available registers */
        asm volatile (
            "mov %[a], %%r8\n\t"
            "mov %[b], %%r9\n\t"
            "mov %[c], %%r10\n\t"
            "add %%r9, %%r8\n\t"
            "add %%r10, %%r8\n\t"
            "mov %%r8, %[out]\n\t"
            "mov %[d], %%rax\n\t"
            "mov %[e], %%rbx\n\t"
            "imul %%rbx, %%rax\n\t"
            :
            [out] "=r" (live1)
            :
            [a] "r" (live2),
            [b] "r" (live3),
            [c] "r" (live4),
            [d] "r" (live5),
            [e] "r" (reg_var1)  /* Use explicit register variable */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory"
        );
        
        nv1 = live1;
        result += live1;
        
        goto block6;
    }
    
block5:
    {
        /* Alternative path with different register pressure */
        double dtemp = d1;
        asm volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (dtemp)
            : [in] "m" (d2)
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        d1 = dtemp;
        goto block6;
    }
    
block6:
    /* Final computations using all variables */
    checksum = result + nv1 + nv2 + nv3 + nv4 + nv5 + 
               (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 +
               (int)reg_var1 + (int)reg_var2 + (int)reg_var3;
    
    printf("Checksum: %d\n", checksum);
    
    /* Use all volatile variables to prevent optimization */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    return checksum & 0xFF;
}
