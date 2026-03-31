/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile long vl1 = 100, vl2 = 200, vl3 = 300;
volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
volatile double vd1 = 10.5, vd2 = 20.5, vd3 = 30.5;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 0x1234;
register int r13_var asm ("r13") = 0x5678;
register int r14_var asm ("r14") = 0x9ABC;
register int r15_var asm ("r15") = 0xDEF0;

int main(void) {
    /* Non-volatile variables with different types/sizes */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 10000, i2 = 20000, i3 = 30000;
    long l1 = 100000L, l2 = 200000L, l3 = 300000L;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 10.1, d2 = 20.2, d3 = 30.3;
    
    /* Variables for address-taking */
    int addr_var1 = 0x1111, addr_var2 = 0x2222;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values by using all variables */
    v1 = i1 + s1;
    v2 = i2 * 2;
    v3 = c1 + c2;
    vl1 = l1 + l2;
    vf1 = f1 * f2;
    vd1 = d1 / d2;
    
    /* BLOCK 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        long ltmp;
        double dtmp;
        
        /* Assembly with multiple outputs, early clobber, and memory operands */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            "lea (%[mem], %%eax, 2), %%ebx\n\t"
            "mov %%ebx, %[out3]"
            : [out1] "=&r" (tmp1),  /* Early clobber - conflicts with inputs */
              [out2] "=r" (tmp2),
              [out3] "=r" (tmp3),
              "+m" (addr_var1)      /* Read-write memory operand */
            : [in1] "r" (i1),
              [in2] "r" (i2),
              [in3] "r" (i3),
              [mem] "m" (addr_var2) /* Memory constraint forcing address reload */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        i1 = tmp1 + tmp2;
        addr_var1 = tmp3;
        
        /* Use pinned register variables to force conflicts */
        r12_var = i1;
        r13_var = i2;
    }
    
    /* Modify variables to keep them live */
    i2 = v2 + v3;
    l1 = vl1 * 2;
    f1 = vf1 + 1.0f;
    
    /* BLOCK 2: More assembly with different data types */
block2:
    {
        short stmp;
        char ctmp;
        float ftmp;
        
        /* Mixed-type operands causing mode mismatches */
        asm volatile (
            "movswl %[sin], %%eax\n\t"
            "addl %[iin], %%eax\n\t"
            "movb %%al, %[cout]\n\t"
            "cvtsi2ss %%eax, %%xmm0\n\t"
            "movss %%xmm0, %[fout]"
            : [cout] "=r" (ctmp),
              [fout] "=x" (ftmp)
            : [sin] "r" (s1),
              [iin] "r" (i1)
            : "rax", "xmm0", "xmm1", "cc"
        );
        
        c1 = ctmp;
        f2 = ftmp;
        
        /* Force use of explicit register variables */
        asm volatile (
            "addl %%r12d, %%r13d\n\t"
            "movl %%r13d, %[out]"
            : [out] "=r" (i3)
            : 
            : "r12", "r13", "cc"
        );
    }
    
    /* More variable modifications */
    s1 = v1 + c1;
    d1 = vd1 * 2.0;
    r14_var = i3;
    
    /* BLOCK 3: Assembly with many clobbers */
block3:
    {
        long ltmp1, ltmp2;
        
        /* Extensive clobber list forcing many spills */
        asm volatile (
            "mov %[lin1], %%rax\n\t"
            "add %[lin2], %%rax\n\t"
            "mov %%rax, %[lout1]\n\t"
            "mov %[din1], %%xmm0\n\t"
            "addsd %[din2], %%xmm0\n\t"
            "movq %%xmm0, %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[lout2]"
            : [lout1] "=&r" (ltmp1),
              [lout2] "=r" (ltmp2)
            : [lin1] "r" (l1),
              [lin2] "r" (l2),
              [din1] "x" (d1),
              [din2] "x" (d2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory", "cc"
        );
        
        l3 = ltmp1 + ltmp2;
    }
    
    /* Final computations using all variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += vl1 + vl2 + vl3;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2 + (int)vd3;
    checksum += c1 + c2 + c3;
    checksum += s1 + s2 + s3;
    checksum += i1 + i2 + i3;
    checksum += (int)l1 + (int)l2 + (int)l3;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += r12_var + r13_var + r14_var + r15_var;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
