/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;

/* Explicit register variables to pin specific registers */
register int r12_var asm ("r12") = 0x1234;
register int r13_var asm ("r13") = 0x5678;
register int r14_var asm ("r14") = 0x9ABC;
register int r15_var asm ("r15") = 0xDEF0;

int main(void) {
    /* Non-volatile variables with different types/sizes */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000, i5 = 5000;
    long l1 = 10000L, l2 = 20000L;
    float f5 = 5.5f, f6 = 6.6f;
    double d5 = 5.55, d6 = 6.66;
    
    /* Variables whose addresses will be taken */
    int addr_var1 = 111, addr_var2 = 222, addr_var3 = 333;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2, *ptr3 = &addr_var3;
    
    /* Force many values to be live */
    int sum = v1 + v2 + v3 + v4 + v5;
    sum += v6 + v7 + v8 + v9 + v10;
    sum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    sum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    /* Use explicit register variables */
    sum += r12_var + r13_var + r14_var + r15_var;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1 = i1, tmp2 = i2, tmp3 = i3;
        
        /* Inline asm with multiple outputs, early clobber, and memory constraint */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            "lea (%[mem], %%eax, 2), %%ebx\n\t"
            "mov %%ebx, %[out3]"
            : [out1] "=&r" (i1),      /* Early clobber - can't share reg with inputs */
              [out2] "=r" (i2),
              [out3] "=r" (i3)
            : [in1] "r" (tmp1),
              [in2] "r" (tmp2),
              [in3] "r" (tmp3),
              [mem] "m" (addr_var1)   /* Memory constraint - address may need reload */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        sum += i1 + i2 + i3 + c1 + s1;
        goto block2;
    }
    
    /* Unreachable code to create separate basic block */
    sum += 9999;
    
block2:
    {
        /* More complex asm with floating point and different modes */
        double tmp_d = d5;
        float tmp_f = f5;
        
        /* Mixed type asm with register constraints that conflict */
        asm volatile (
            "cvtsd2ss %[din], %%xmm0\n\t"
            "addss %[fin], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %[dout]\n\t"
            "movd %%xmm0, %[iout]"
            : [dout] "=rm" (d5),      /* Register or memory - may force reload */
              [iout] "=r" (i4)
            : [din] "xm" (tmp_d),     /* SSE register or memory */
              [fin] "xm" (tmp_f)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "cc"
        );
        
        /* Use address-taken variables in ways that may force reloads */
        *ptr1 = i4;
        addr_var2 = *ptr2 + i4;
        
        sum += (int)d5 + i4 + addr_var1 + addr_var2;
        goto block3;
    }
    
block3:
    {
        /* Asm that clobbers explicit register variables */
        int tmp_r12 = r12_var;
        int tmp_r13 = r13_var;
        
        asm volatile (
            "mov %[a], %%r12\n\t"
            "add %[b], %%r12\n\t"
            "mov %%r12, %[out]\n\t"
            /* Clobber registers used by our explicit register variables */
            "xor %%r13, %%r13\n\t"
            "xor %%r14, %%r14"
            : [out] "=r" (i5)
            : [a] "r" (tmp_r12),
              [b] "r" (tmp_r13)
            : "r12", "r13", "r14", "r15", "cc"
        );
        
        /* Force reload of explicit register variables */
        r12_var = i5;
        r13_var = i5 * 2;
        r14_var = i5 * 3;
        
        sum += i5 + r12_var + r13_var + r14_var + r15_var;
        goto block4;
    }
    
block4:
    {
        /* Asm with multiple memory inputs and outputs */
        long tmp_l = l1;
        int tmp_i = i1;
        
        /* Complex addressing mode that may require reloads */
        asm volatile (
            "mov (%[mem1]), %%rax\n\t"
            "add %[reg1], %%rax\n\t"
            "mov %%rax, (%[mem2])\n\t"
            "imul %[reg2], %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=r" (l2)
            : [mem1] "r" (ptr1),
              [reg1] "r" (tmp_l),
              [mem2] "r" (ptr2),
              [reg2] "r" (tmp_i)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        sum += l1 + l2 + *ptr1 + *ptr2;
        
        /* Final computation using all variable types */
        sum += c2 + c3 + s2 + s3;
        sum += (int)f6 + (int)d6;
        sum += addr_var3;
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = sum;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
