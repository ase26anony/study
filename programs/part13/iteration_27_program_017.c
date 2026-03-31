/* Test program to trigger GCC reload pass uncovered block in push_reload */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f;
volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Non-volatile variables with mixed types */
    char c1 = 'a', c2 = 'b';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f5 = 5.5f, f6 = 6.6f;
    double d4 = 7.7, d5 = 8.8;
    
    /* Variables for address-taking */
    int addr_var1 = 111, addr_var2 = 222;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values */
    v1 = v2 + v3;
    f1 = f2 * f3;
    d1 = d2 / d3;
    
    /* Block 1: Complex inline asm with conflicting constraints */
block1:
    {
        int tmp1 = i1 + i2;
        int tmp2 = i3 + i4;
        
        /* Inline asm with multiple outputs, early clobber, and memory constraint */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            "lea (%[mem], %[in4]), %%ebx\n\t"
            "mov %%ebx, %[out3]"
            : [out1] "=r" (tmp1), 
              [out2] "=&r" (tmp2),  /* & = early clobber */
              [out3] "=m" (addr_var1)
            : [in1] "r" (i1),
              [in2] "r" (i2),
              [in3] "r" (v4),
              [in4] "r" (v5),
              [mem] "m" (addr_var2)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        i1 = tmp1;
        i2 = tmp2;
        v6 = addr_var1 + v7;
    }
    
    /* Modify variables to keep them live */
    c1++;
    s1 += v1;
    l1 *= 2;
    
    /* Block 2: More complex asm with explicit register clobbers */
block2:
    {
        long tmp_l = l1 + l2;
        double tmp_d = d4 + d5;
        
        /* Clobber our explicit register variables */
        asm volatile (
            "mov %[in_l], %%rax\n\t"
            "add $1000, %%rax\n\t"
            "mov %%rax, %[out_l]\n\t"
            "movq %[in_d], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movq %%xmm0, %[out_d]"
            : [out_l] "=r" (tmp_l),
              [out_d] "=m" (d4)
            : [in_l] "r" (l2),
              [in_d] "m" (d5)
            : "rax", "rbx", "rcx", "rdx", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
        );
        
        l2 = tmp_l;
        f5 = (float)tmp_d;
    }
    
    /* Force address mode conflicts */
    int * volatile ptr_vol = &i3;
    
    /* Block 3: Address conflicts and mixed modes */
block3:
    {
        short tmp_s = s1;
        char tmp_c = c2;
        
        /* Using same variable as memory and register operand */
        asm volatile (
            "movzwl %[in_s], %%eax\n\t"
            "addb %[in_c], %%al\n\t"
            "movl %%eax, %[out_mem]\n\t"
            "movl (%[ptr]), %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[out_reg]"
            : [out_mem] "=m" (addr_var2),
              [out_reg] "=r" (i4)
            : [in_s] "r" (tmp_s),
              [in_c] "r" (tmp_c),
              [ptr] "r" (ptr_vol),
              "m" (*ptr_vol)  /* Memory input */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        s2 = tmp_s + 100;
        c2 = tmp_c - 1;
    }
    
    /* Block 4: Many live values across goto */
    int live1 = r12_var + 1;
    int live2 = r13_var + 2;
    int live3 = r14_var + 3;
    int live4 = r15_var + 4;
    
    if (v8 > 0) {
        goto block4;
    }
    
    /* Intermediate block to create more live ranges */
    {
        float tmp_f = f2 + f3;
        asm volatile ("" : "+r" (tmp_f) : : "memory");
        f6 = tmp_f;
    }
    
block4:
    {
        /* Complex asm with many inputs/outputs */
        int a = live1, b = live2, c = live3, d = live4;
        
        asm volatile (
            "mov %[a_in], %%r8d\n\t"
            "mov %[b_in], %%r9d\n\t"
            "add %%r9d, %%r8d\n\t"
            "mov %[c_in], %%r10d\n\t"
            "add %%r10d, %%r8d\n\t"
            "mov %[d_in], %%r11d\n\t"
            "add %%r11d, %%r8d\n\t"
            "mov %%r8d, %[sum_out]"
            : [sum_out] "=rm" (v9)
            : [a_in] "r" (a),
              [b_in] "r" (b),
              [c_in] "r" (c),
              [d_in] "r" (d)
            : "r8", "r9", "r10", "r11", "memory", "cc"
        );
    }
    
    /* Final block: Mixed type operations */
    {
        /* Force mode mismatches */
        unsigned char uc = 0xFF;
        unsigned short us = 0xFFFF;
        unsigned int ui = 0xFFFFFFFF;
        
        asm volatile (
            "movzbl %[uc], %%eax\n\t"
            "movzwl %[us], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %[ui], %%eax\n\t"
            "movl %%eax, %[result]"
            : [result] "=rm" (v10)
            : [uc] "r" (uc),
              [us] "r" (us),
              [ui] "r" (ui)
            : "rax", "rbx", "cc"
        );
    }
    
    /* Aggregate results to prevent optimization */
    volatile int checksum = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        i1 + i2 + i3 + i4 +
        (int)c1 + (int)c2 +
        s1 + s2 +
        (int)l1 + (int)l2 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
        r12_var + r13_var + r14_var + r15_var;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
