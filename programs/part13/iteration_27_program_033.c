/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage */
register long reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_var3 asm ("xmm8");

int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    long nv5 = 500, nv6 = 600, nv7 = 700;
    float nvf1 = 10.1f, nvf2 = 20.2f;
    double nvd1 = 10.11, nvd2 = 20.22;
    
    /* Explicit register variables */
    reg_var1 = 0x123456789ABCDEF0;
    reg_var2 = 0xDEADBEEF;
    reg_var3 = 3.141592653589793;
    
    /* Take addresses to create addressing mode conflicts */
    int *ptr1 = &nv1, *ptr2 = &nv2;
    long *ptr3 = &nv5;
    float *ptr4 = &nvf1;
    double *ptr5 = &nvd1;
    
    /* Complex checksum to prevent dead code elimination */
    volatile long checksum = 0;
    
    /* Block 1: Multiple conflicting constraints */
block1:
    {
        int temp1 = v1 + v2;
        long temp2 = v6 * v7;
        
        /* Inline asm with conflicting constraints */
        __asm__ volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output1]\n\t"
            "movq %[input3], %%rbx\n\t"
            "imulq %[input4], %%rbx\n\t"
            "movq %%rbx, %[output2]\n\t"
            : [output1] "=r" (nv1),          /* Output in register */
              [output2] "=&r" (nv5)          /* Early clobber output */
            : [input1] "m" (v1),             /* Memory constraint */
              [input2] "r" (temp1),          /* Register constraint */
              [input3] "r" (v6),             /* Register constraint */
              [input4] "m" (v7)              /* Memory constraint */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        v1 = nv1 + 1;
        v6 = nv5 >> 1;
        
        /* Force spill by using many variables */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    /* Block 2: Mixed data types and explicit clobbers */
block2:
    {
        float ftemp = f1 * f2;
        double dtemp = d1 / d2;
        
        /* Complex asm with mixed types and clobbers */
        __asm__ volatile (
            "cvtsi2ssl %[int_in], %%xmm0\n\t"
            "addss %[float_in], %%xmm0\n\t"
            "movss %%xmm0, %[float_out]\n\t"
            "cvtsi2sdq %[long_in], %%xmm1\n\t"
            "addsd %[double_in], %%xmm1\n\t"
            "movsd %%xmm1, %[double_out]\n\t"
            : [float_out] "=m" (nvf1),       /* Memory output */
              [double_out] "=m" (nvd1)       /* Memory output */
            : [int_in] "r" (nv2),            /* Register input */
              [float_in] "x" (ftemp),        /* XMM register input */
              [long_in] "r" (nv6),           /* Register input */
              [double_in] "x" (dtemp)        /* XMM register input */
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "memory"
        );
        
        /* Use the explicit register variables that were clobbered */
        __asm__ volatile (
            "addq %%r12, %[sum]\n\t"
            "addl %%r13d, %[sum]\n\t"
            : [sum] "+r" (checksum)
            : 
            : "cc"
        );
        
        checksum += (long)(nvf1 * 1000) + (long)(nvd1 * 1000);
        goto block3;
    }
    
    /* Block 3: Addressing mode conflicts and early clobber */
block3:
    {
        /* Create addressing mode conflict */
        int index = v3;
        int *dynamic_ptr = &nv3 + index;
        
        __asm__ volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl %[val], %%eax\n\t"
            "movl %%eax, (%[ptr])\n\t"
            "leaq (%[base],%[index],4), %[addr]\n\t"
            : [addr] "=&r" (ptr2)            /* Early clobber output */
            : [ptr] "r" (dynamic_ptr),       /* Register containing address */
              [val] "r" (v4),                /* Register value */
              [base] "r" (ptr1),             /* Base register */
              [index] "r" (index)            /* Index register */
            : "rax", "memory", "cc"
        );
        
        /* Force reload of reg_var3 */
        reg_var3 = reg_var3 * 2.0;
        checksum += (long)reg_var3;
        
        /* Another asm with many operands */
        __asm__ volatile (
            "mov %[a], %%rax\n\t"
            "mov %[b], %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[c]\n\t"
            "mov %[d], %%rcx\n\t"
            "sub %%rcx, %%rax\n\t"
            : [c] "=rm" (nv7)                /* Register or memory output */
            : [a] "rm" (nv5),                /* Input can be reg or mem */
              [b] "rm" (nv6),
              [d] "rm" (v8)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "cc"
        );
        
        checksum += nv7;
        goto block4;
    }
    
    /* Block 4: Final computations and output */
block4:
    {
        /* Use all variables one more time */
        checksum += reg_var1 + reg_var2;
        checksum += (long)(f1 + f2 + f3) * 1000;
        checksum += (long)(d1 + d2 + d3) * 1000;
        
        /* Final inline asm with output constraint */
        long final_result;
        __asm__ volatile (
            "movq %[chk], %%rax\n\t"
            "shrq $3, %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=r" (final_result)
            : [chk] "rm" (checksum)
            : "rax", "cc"
        );
        
        printf("Result: %ld\n", final_result);
    }
    
    return 0;
}
