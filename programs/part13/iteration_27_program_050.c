/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force the compiler to generate reloads */
volatile int checksum = 0;

int main(void) {
    /* Declare many volatile variables to force register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3;
    volatile float f4 = 4.4f, f5 = 5.5f;
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    double nvf1 = 100.1, nvf2 = 200.2;
    
    /* Explicit register variables - pin to specific registers */
    register int reg_var1 asm ("r12") = 0x1234;
    register int reg_var2 asm ("r13") = 0x5678;
    register double reg_fvar asm ("xmm8") = 3.14159;
    
    /* Variables for address-taking */
    int addr_var1 = 999, addr_var2 = 888;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Mixed size variables */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    long long ll1 = 0x1122334455667788LL;
    
    /* Start with some arithmetic to create live values */
    nv1 = v1 + v2 + v3;
    nv2 = v4 * v5 - v6;
    nvf1 = f1 + f2;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2;
        /* Force address computation and register conflict */
        asm volatile (
            "movl %[addr1], %%eax\n\t"
            "addl %[val1], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[val2], %%ebx\n\t"
            "imull %%eax, %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out1] "=r" (tmp1), [out2] "=r" (tmp2)
            : [addr1] "m" (addr_var1), [val1] "r" (nv1), 
              [val2] "r" (nv2)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use the results and modify live variables */
        v1 = tmp1;
        v2 = tmp2;
        reg_var1 = tmp1 + tmp2;  /* Use pinned register variable */
    }
    
    /* Block 2: More assembly with memory operands and clobbers */
block2:
    {
        long long result;
        /* Force reload by using same register for input and output */
        asm volatile (
            "movq %[in1], %%rax\n\t"
            "addq %[in2], %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            "movq %[in3], %%rbx\n\t"
            "subq %%rax, %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            : [out] "=&r" (result), [out2] "=m" (addr_var2)
            : [in1] "r" (ll1), [in2] "r" (reg_var1), 
              [in3] "m" (addr_var2)
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", 
              "r11", "memory", "cc"
        );
        
        ll1 = result;
        nv3 = addr_var2;
    }
    
    /* Block 3: Floating point with mixed modes */
block3:
    {
        double fp_result;
        float fp_result_f;
        
        /* Mixed precision operations forcing mode conversions */
        asm volatile (
            "movsd %[dbl1], %%xmm0\n\t"
            "addsd %[dbl2], %%xmm0\n\t"
            "cvtsd2ss %%xmm0, %%xmm1\n\t"
            "mulss %[flt1], %%xmm1\n\t"
            "cvtss2sd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[out1]\n\t"
            "movss %%xmm1, %[out2]\n\t"
            : [out1] "=m" (fp_result), [out2] "=m" (fp_result_f)
            : [dbl1] "m" (nvf1), [dbl2] "x" (reg_fvar),
              [flt1] "m" (f4)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "memory"
        );
        
        nvf2 = fp_result;
        f5 = fp_result_f;
    }
    
    /* Block 4: Character/short operations with sign extension */
block4:
    {
        int char_result;
        short short_result;
        
        /* Operations requiring mode changes */
        asm volatile (
            "movsbl %[ch1], %%eax\n\t"
            "movswl %[sh1], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movw %%ax, %[out2]\n\t"
            : [out1] "=r" (char_result), [out2] "=m" (short_result)
            : [ch1] "m" (c1), [sh1] "m" (s1)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        nv4 = char_result;
        s2 = short_result;
    }
    
    /* Block 5: Complex addressing with multiple outputs */
block5:
    {
        int out1, out2, out3;
        
        /* Multiple output operands with early clobber */
        asm volatile (
            "leaq (%[ptr]), %%rax\n\t"
            "movl (%%rax), %%ebx\n\t"
            "addl %[val], %%ebx\n\t"
            "movl %%ebx, %[o1]\n\t"
            "movl %%ebx, (%%rax)\n\t"
            "movl %[val2], %%ecx\n\t"
            "subl %%ebx, %%ecx\n\t"
            "movl %%ecx, %[o2]\n\t"
            "imull %%ecx, %%ebx\n\t"
            "movl %%ebx, %[o3]\n\t"
            : [o1] "=&r" (out1), [o2] "=&r" (out2), 
              [o3] "=r" (out3), "+m" (*ptr1)
            : [ptr] "r" (ptr1), [val] "r" (v7), 
              [val2] "r" (v8)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13",
              "r14", "r15", "memory", "cc"
        );
        
        v9 = out1 + out2 + out3;
        reg_var2 = out3;
    }
    
    /* Create control flow with goto to keep variables live */
    if (v1 > 0) {
        goto block2;
    } else if (v2 > 0) {
        goto block3;
    }
    
    /* Final computation using all variables */
    checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
               + nv1 + nv2 + nv3 + nv4
               + (int)f1 + (int)f2 + (int)f3
               + (int)f4 + (int)f5
               + c1 + c2 + s1 + s2
               + (int)ll1 + (int)reg_var1 + (int)reg_var2
               + addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
