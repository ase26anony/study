/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile long vl1 = 100, vl2 = 200, vl3 = 300;
volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
volatile double vd1 = 10.5, vd2 = 20.5;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 42;
register int r13_var asm ("r13") = 43;
register int r14_var asm ("r14") = 44;

int main(void) {
    /* Non-volatile variables with different types */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000, i5 = 50000;
    long l1 = 100000L, l2 = 200000L;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 10.1, d2 = 20.2, d3 = 30.3;
    
    /* Variables for address taking */
    int addr_var1 = 777, addr_var2 = 888;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values by doing arithmetic */
    v1 = v1 + 1; v2 = v2 * 2; v3 = v3 - 1;
    vl1 = vl1 + v1; vl2 = vl2 + v2;
    vf1 = vf1 * 2.0f; vf2 = vf2 / 2.0f;
    vd1 = vd1 + 1.0; vd2 = vd2 - 1.0;
    
    /* Use explicit register variables */
    r12_var = r12_var + v1;
    r13_var = r13_var * 2;
    r14_var = r14_var - v2;
    
    /* Block 1: Complex inline assembly with many constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        long ltmp;
        double dtmp;
        
        /* Inline assembly with conflicting constraints and clobbers */
        asm volatile (
            /* Output operands with earlyclobber to force separate registers */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[in3], %%rbx\n\t"
            "addq %[in4], %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            /* Memory operand with address conflict */
            "movl (%[mem]), %%ecx\n\t"
            "addl %%eax, %%ecx\n\t"
            "movl %%ecx, (%[mem])\n\t"
            /* Floating point operation to use different register set */
            "movsd %[din], %%xmm0\n\t"
            "addsd %[din2], %%xmm0\n\t"
            "movsd %%xmm0, %[dout]\n\t"
            : [out1] "=&r" (tmp1),      /* Earlyclobber reg constraint */
              [out2] "=&r" (ltmp),      /* Earlyclobber for different type */
              [dout] "=m" (dtmp),       /* Memory output */
              "+m" (addr_var1)          /* Read-write memory operand */
            : [in1] "r" (i1),           /* Register input */
              [in2] "r" (i2),           /* Another register input */
              [in3] "r" (l1),           /* Long in register */
              [in4] "r" (l2),           /* Another long */
              [mem] "r" (ptr1),         /* Pointer in register */
              [din] "x" (d1),           /* XMM register constraint */
              [din2] "x" (d2)           /* Another XMM input */
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "memory", "cc"
        );
        
        i1 = tmp1 + v1;
        l1 = ltmp + vl1;
        d3 = dtmp + vd1;
    }
    
    /* Modify variables to keep them live */
    c1 = c1 + 1; c2 = c2 - 1; c3 = c3 * 2;
    s1 = s1 + v2; s2 = s2 - v3; s3 = s3 * 2;
    i2 = i2 + i1; i3 = i3 - i2; i4 = i4 * 2; i5 = i5 / 2;
    
    /* Block 2: More inline assembly with different constraints */
block2:
    {
        short stmp;
        int itmp1, itmp2;
        float ftmp;
        
        /* Another asm with register constraints that conflict with live values */
        asm volatile (
            /* Mixed size operations */
            "movw %[sin], %%ax\n\t"
            "addw $100, %%ax\n\t"
            "movw %%ax, %[sout]\n\t"
            /* Use the same register for input and output (conflict) */
            "movl %[iin1], %%ebx\n\t"
            "imull %[iin2], %%ebx\n\t"
            "movl %%ebx, %[iout1]\n\t"
            /* Floating point with memory operand */
            "movss %[fin], %%xmm6\n\t"
            "mulss %[fin2], %%xmm6\n\t"
            "movss %%xmm6, %[fout]\n\t"
            /* Clobber explicit register variables */
            "xorq %%r12, %%r12\n\t"
            "xorq %%r13, %%r13\n\t"
            : [sout] "=r" (stmp),
              [iout1] "=r" (itmp1),
              [fout] "=m" (ftmp)
            : [sin] "r" (s1),
              [iin1] "0" (i3),          /* Same as output 0 */
              [iin2] "r" (i4),
              [fin] "x" (f1),
              [fin2] "x" (f2)
            : "rax", "rbx", "rcx", "r12", "r13", "r14", "r15",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
              "memory", "cc"
        );
        
        s2 = stmp + c1;
        i5 = itmp1 + i2;
        f3 = ftmp + vf1;
    }
    
    /* Force more arithmetic to keep values live */
    f1 = f1 * 1.5f; f2 = f2 / 1.5f;
    d1 = d1 * 1.1; d2 = d2 / 1.1;
    addr_var2 = addr_var1 + i5;
    
    /* Block 3: Inline assembly with addressing mode conflicts */
block3:
    {
        int tmp;
        double dtmp2;
        
        /* Take address of a variable and use it in asm while also using the variable */
        int local_var = 9999;
        int *local_ptr = &local_var;
        
        asm volatile (
            /* Use memory operand and register operand for same variable */
            "movl %[val], %%eax\n\t"
            "addl (%[ptr]), %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            /* Double with mismatched constraints */
            "movsd %[dval], %%xmm15\n\t"
            "sqrtsd %%xmm15, %%xmm15\n\t"
            "movsd %%xmm15, %[dout]\n\t"
            : [out] "=r" (tmp),
              [dout] "=m" (dtmp2)
            : [val] "r" (local_var),    /* Variable in register */
              [ptr] "r" (local_ptr),    /* Its address in another register */
              [dval] "x" (d3)
            : "rax", "rbx", "rcx", "rdx",
              "xmm15", "xmm14", "xmm13", "xmm12",
              "memory", "cc"
        );
        
        i3 = tmp + local_var;
        d1 = dtmp2 + d2;
    }
    
    /* Create control flow with goto to make values live across blocks */
    if (v1 > 0) {
        goto block1;
    } else if (v2 < 0) {
        goto block2;
    } else {
        goto block3;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int checksum = 0;
    checksum = v1 + v2 + v3 + v4 + v5;
    checksum += vl1 + vl2 + vl3;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    checksum += c1 + c2 + c3 + s1 + s2 + s3;
    checksum += i1 + i2 + i3 + i4 + i5;
    checksum += (int)l1 + (int)l2;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += r12_var + r13_var + r14_var;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
