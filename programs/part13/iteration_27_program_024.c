/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile long vl1 = 100, vl2 = 200, vl3 = 300;
volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
volatile double vd1 = 10.5, vd2 = 20.5, vd3 = 30.5;

/* Explicit register variables to pin registers */
register int reg_var1 asm ("r12") = 42;
register int reg_var2 asm ("r13") = 43;
register int reg_var3 asm ("r14") = 44;

int main(void) {
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 10000, i2 = 20000, i3 = 30000;
    long l1 = 100000L, l2 = 200000L, l3 = 300000L;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 10.1, d2 = 20.2, d3 = 30.3;
    
    /* Pointers for addressing mode conflicts */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    volatile int *vp1 = &v1;
    
    /* Force many live values */
    int result = 0;
    volatile int checksum = 0;
    
    /* Block 1: Complex inline asm with conflicting constraints */
block1:
    {
        int temp1, temp2, temp3;
        
        /* Inline asm with output that conflicts with inputs */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=&r" (temp1),  /* Early clobber to force separate reg */
              [out2] "=r" (temp2)     /* Output in same reg as computation */
            : [in1] "r" (i1),         /* Input in register */
              [in2] "r" (i2),
              [in3] "rm" (i3)         /* Can be reg or memory */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use results to keep them live */
        result += temp1 + temp2;
        
        /* Modify variables to force spills */
        v1 = temp1;
        v2 = temp2;
    }
    
    /* Block 2: More complex asm with memory addressing conflicts */
block2:
    {
        long temp_l;
        int temp_i;
        
        /* Take address and use in asm while also using the value */
        int *addr = &i1;
        
        /* This creates addressing mode conflicts */
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "addl %[val], %%eax\n\t"
            "movl %%eax, %[out_i]\n\t"
            "leaq (%[addr], %%rax, 4), %[out_l]\n\t"
            : [out_i] "=r" (temp_i),
              [out_l] "=r" (temp_l)
            : [addr] "r" (addr),      /* Address in register */
              [val] "rm" (v3)         /* Value that might need reload */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
        );
        
        result += temp_i + (int)temp_l;
        
        /* Clobber explicit register variables */
        asm volatile (
            "xorq %%r12, %%r12\n\t"
            "xorq %%r13, %%r13\n\t"
            : : : "r12", "r13", "cc"
        );
        
        /* Force reload of explicit register vars */
        reg_var1 = result;
        reg_var2 = result + 1;
    }
    
    /* Block 3: Mixed data types and modes */
block3:
    {
        short out_s;
        char out_c;
        double out_d;
        
        /* Different modes in same asm statement */
        asm volatile (
            "movswl %[in_s], %%eax\n\t"
            "addb %[in_c], %%al\n\t"
            "movsbw %%al, %[out_s]\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "addsd %[in_d], %%xmm0\n\t"
            "movsd %%xmm0, %[out_d]\n\t"
            : [out_s] "=r" (out_s),
              [out_d] "=x" (out_d)
            : [in_s] "r" (s1),
              [in_c] "r" (c1),
              [in_d] "x" (d1)
            : "rax", "xmm0", "xmm1", "cc"
        );
        
        result += out_s + (int)out_d;
        
        /* Force floating point spills */
        vf1 = out_d;
        vd1 = out_d * 2.0;
    }
    
    /* Block 4: Multiple outputs with early clobber */
block4:
    {
        int out1, out2, out3;
        
        /* Complex constraints that likely need reloads */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[o1]\n\t"
            "movl %[c], %%ecx\n\t"
            "imull %%ecx, %%eax\n\t"
            "movl %%eax, %[o2]\n\t"
            "movl %[d], %%edx\n\t"
            "subl %%edx, %%eax\n\t"
            "movl %%eax, %[o3]\n\t"
            : [o1] "=&r" (out1),  /* Early clobber */
              [o2] "=&r" (out2),  /* Early clobber */
              [o3] "=r" (out3)
            : [a] "r" (v1),
              [b] "rm" (v2),
              [c] "rm" (v3),
              [d] "rm" (v4)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        result += out1 + out2 + out3;
    }
    
    /* Block 5: Large clobber list forcing many spills */
block5:
    {
        int final;
        
        /* Clobber almost all registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (final)
            : [in1] "r" (result),
              [in2] "r" (reg_var3)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2",
              "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
              "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory", "cc"
        );
        
        checksum = final;
    }
    
    /* Use goto to create complex control flow with live values */
    if (checksum > 0)
        goto block1;
    
    /* Print to prevent optimization */
    printf("Result: %d\n", checksum);
    
    /* Use all variables to prevent dead code elimination */
    result += c1 + c2 + c3 + s1 + s2 + s3;
    result += *p1 + *p2 + *p3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)d1 + (int)d2 + (int)d3;
    result += reg_var1 + reg_var2 + reg_var3;
    
    return result & 0xFF;
}
