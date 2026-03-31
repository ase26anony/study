/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile long vl1 = 100, vl2 = 200, vl3 = 300;
volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
volatile double vd1 = 10.5, vd2 = 20.5, vd3 = 30.5;

/* Explicit register variables to force conflicts */
register int r12_var asm ("r12") = 42;
register int r13_var asm ("r13") = 43;
register int r14_var asm ("r14") = 44;

int main(void) {
    /* Non-volatile variables with mixed types */
    int a = 10, b = 20, c = 30, d = 40, e = 50;
    short s1 = 100, s2 = 200;
    char ch1 = 'A', ch2 = 'B';
    float f1 = 1.1f, f2 = 2.2f;
    double d1 = 3.3, d2 = 4.4;
    
    /* Variables for address-taking */
    int addr_var1 = 1000, addr_var2 = 2000;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values by using all variables */
    v1 = a + b;
    v2 = c * d;
    vf1 = f1 + f2;
    vd1 = d1 * d2;
    
    /* BLOCK 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        /* Inline asm with multiple outputs, inputs, and clobbers */
        __asm__ volatile (
            /* Output operands with different constraints */
            "movl %[in1], %[out1]\n\t"
            "addl %[in2], %[out1]\n\t"
            "movl %[out1], %[out2]\n\t"
            "imull %[in3], %[out2]\n\t"
            /* Memory operand with address conflict */
            "movl %[mem], %%eax\n\t"
            "addl %%eax, %[out3]\n\t"
            /* Clobber many registers to force spills */
            : [out1] "=&r" (tmp1),  /* Early clobber */
              [out2] "=r" (tmp2),
              [out3] "=r" (tmp3)
            : [in1] "r" (a),        /* Register constraint */
              [in2] "r" (b),
              [in3] "0" (c),        /* Same as output 0 */
              [mem] "m" (addr_var1) /* Memory constraint */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "cc", "memory"
        );
        
        /* Use results to keep them live */
        v3 = tmp1 + tmp2 + tmp3;
        a = tmp1 ^ tmp2;
    }
    
    /* Modify variables to keep them live across blocks */
    b += v1;
    c *= v2;
    d = r12_var + r13_var;
    
    /* BLOCK 2: More assembly with mode mismatches */
block2:
    {
        short s_out;
        int i_out;
        long l_out;
        
        /* Mixed size operands to trigger mode reloads */
        __asm__ volatile (
            "movw %[sin], %%ax\n\t"
            "cwtl\n\t"
            "movl %%eax, %[iout]\n\t"
            "cltq\n\t"
            "movq %%rax, %[lout]\n\t"
            "addw $1, %[sout]\n\t"
            : [sout] "=r" (s_out),
              [iout] "=r" (i_out),
              [lout] "=r" (l_out)
            : [sin] "r" (s1)
            : "rax", "rdx", "cc"
        );
        
        s1 = s_out;
        vl1 = l_out;
        v4 = i_out;
    }
    
    /* Force address conflicts */
    e = *ptr1 + *ptr2;
    __asm__ volatile (
        "addl $1, %0\n\t"
        : "+m" (addr_var2)  /* Read-write memory operand */
        :
        : "cc"
    );
    
    /* BLOCK 3: Assembly that clobbers explicit register variables */
block3:
    {
        int result;
        /* This asm clobbers r12, forcing reload of r12_var */
        __asm__ volatile (
            "movl $999, %%r12d\n\t"
            "movl %%r12d, %[res]\n\t"
            "xorl %%r12d, %%r12d\n\t"
            : [res] "=r" (result)
            :
            : "r12", "cc"
        );
        
        /* Use both the result and the register variable */
        v5 = result + r12_var;
        
        /* Force r12_var to be reloaded */
        r12_var = result;
    }
    
    /* BLOCK 4: Floating point with integer mix */
block4:
    {
        float f_out;
        double d_out;
        int i_out;
        
        /* Mixed float/int operations */
        __asm__ volatile (
            "cvtsi2ssl %[iin], %%xmm0\n\t"
            "addss %[fin], %%xmm0\n\t"
            "movss %%xmm0, %[fout]\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, %[dout]\n\t"
            "cvttsd2si %%xmm1, %[iout]\n\t"
            : [fout] "=x" (f_out),
              [dout] "=x" (d_out),
              [iout] "=r" (i_out)
            : [iin] "r" (a),
              [fin] "x" (f1)
            : "xmm0", "xmm1", "cc"
        );
        
        f1 = f_out;
        vd2 = d_out;
        v2 = i_out;
    }
    
    /* Final computation using all live variables */
    volatile int checksum = 
        a + b + c + d + e + 
        s1 + s2 + ch1 + ch2 +
        (int)f1 + (int)f2 + 
        (int)d1 + (int)d2 +
        v1 + v2 + v3 + v4 + v5 +
        (int)vl1 + (int)vl2 + (int)vl3 +
        (int)vf1 + (int)vf2 + (int)vf3 +
        (int)vd1 + (int)vd2 + (int)vd3 +
        r12_var + r13_var + r14_var +
        addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    /* Use goto to create complex control flow with live values */
    if (checksum > 1000) {
        goto block1;
    } else if (checksum > 500) {
        goto block2;
    } else if (checksum > 250) {
        goto block3;
    }
    
    return checksum & 0xFF;
}
