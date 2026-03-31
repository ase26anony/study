/* Test program to trigger reload.cc uncovered block */
#include <stdio.h>

/* Force many reloads through complex inline assembly with conflicting constraints */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x87654321;
    volatile int v3 = 0x55555555;
    volatile int v4 = 0xAAAAAAAA;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0xCCCCCCCC;
    volatile float f1 = 3.14159f;
    volatile float f2 = 2.71828f;
    volatile double d1 = 1.41421356;
    volatile double d2 = 1.73205080;
    
    /* Non-volatile variables with complex live ranges */
    int a = 100, b = 200, c = 300, d = 400;
    short s1 = 10, s2 = 20;
    char ch1 = 'A', ch2 = 'B';
    long l1 = 999999999L, l2 = 888888888L;
    
    /* Explicit register variables - pin to specific registers */
    register int reg_var1 asm ("r12") = 0x11111111;
    register int reg_var2 asm ("r13") = 0x22222222;
    register int reg_var3 asm ("r14") = 0x33333333;
    
    /* Variables for address-taking */
    int addr_var1 = 0x44444444;
    int addr_var2 = 0x55555555;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    a = v1 + v2;
    b = v3 - v4;
    c = a * b;
    d = c / (a + 1);
    
    /* Block 1: Multiple conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        
        /* Inline assembly with many constraints that conflict */
        asm volatile (
            /* Output operands with earlyclobber to force separate registers */
            "movl %[in1], %[out1]\n\t"
            "addl %[in2], %[out1]\n\t"
            "movl %[out1], %[out2]\n\t"
            "imull %[in3], %[out2]\n\t"
            /* Memory operand with address that might need reloading */
            "movl %[out2], %[mem1]\n\t"
            /* Use explicit register variables */
            "addl %%r12d, %[out3]\n\t"
            "subl %%r13d, %[out3]\n\t"
            : [out1] "=&r" (tmp1),   /* Earlyclobber - can't share with inputs */
              [out2] "=&r" (tmp2),   /* Another earlyclobber */
              [out3] "=r" (tmp3),
              [mem1] "=m" (addr_var1)  /* Memory output */
            : [in1] "r" (a),         /* Input in register */
              [in2] "r" (b),
              [in3] "r" (c),
              "m" (addr_var2)        /* Memory input */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* Use results to keep them live */
        v1 = tmp1 + tmp2 + tmp3;
        reg_var1 = tmp1;  /* Modify pinned register variable */
    }
    
    /* More arithmetic to create register pressure */
    l1 = v1 * v2 * v3;
    l2 = v4 + v5 + v6;
    f1 = f1 * 2.0f;
    d1 = d1 / 2.0;
    
    /* Block 2: Mixed data types and modes */
block2:
    {
        char c_out;
        short s_out;
        int i_out;
        long l_out;
        
        /* Inline assembly with mixed mode operands */
        asm volatile (
            /* Different sized operations */
            "movb %[ch_in], %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %[c_out]\n\t"
            
            "movw %[s_in], %%ax\n\t"
            "addw $100, %%ax\n\t"
            "movw %%ax, %[s_out]\n\t"
            
            "movl %[i_in], %%eax\n\t"
            "addl $1000, %%eax\n\t"
            "movl %%eax, %[i_out]\n\t"
            
            "movq %[l_in], %%rax\n\t"
            "addq $10000, %%rax\n\t"
            "movq %%rax, %[l_out]\n\t"
            : [c_out] "=m" (c_out),
              [s_out] "=m" (s_out),
              [i_out] "=m" (i_out),
              [l_out] "=m" (l_out)
            : [ch_in] "r" (ch1),
              [s_in] "r" (s1),
              [i_in] "r" (d),
              [l_in] "r" (l1)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        ch2 = c_out;
        s2 = s_out;
        a = i_out;
        l2 = l_out;
    }
    
    /* Block 3: Complex addressing modes */
block3:
    {
        int result1, result2;
        int * volatile ptr_vol = addr_ptr1;
        
        /* Take address and use in memory constraint while also using in register */
        asm volatile (
            /* Use same variable as memory operand and register operand */
            "movl (%[mem_ptr]), %%eax\n\t"
            "addl %[reg_val], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            
            /* Complex addressing that might need reload */
            "leal (%[reg1],%[reg2],4), %%ebx\n\t"
            "addl %%ebx, %[out2]\n\t"
            : [out1] "=r" (result1),
              [out2] "=r" (result2)
            : [mem_ptr] "r" (ptr_vol),    /* Pointer in register */
              [reg_val] "r" (*ptr_vol),   /* Dereferenced value - might need reload */
              [reg1] "r" (reg_var2),
              [reg2] "r" (reg_var3)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        v2 = result1;
        v3 = result2;
    }
    
    /* Block 4: Maximum register pressure */
block4:
    {
        /* Create many live values across the asm */
        int t1 = v1, t2 = v2, t3 = v3, t4 = v4, t5 = v5, t6 = v6;
        float ft1 = f1, ft2 = f2;
        double dt1 = d1, dt2 = d2;
        
        /* Assembly that uses and clobbers almost all registers */
        asm volatile (
            /* Use many input registers */
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "addl %3, %%eax\n\t"
            "addl %4, %%eax\n\t"
            "addl %5, %%eax\n\t"
            
            /* Floating point operations to use FP registers too */
            "movss %6, %%xmm0\n\t"
            "addss %7, %%xmm0\n\t"
            "movsd %8, %%xmm1\n\t"
            "addsd %9, %%xmm1\n\t"
            
            /* Force many clobbers */
            : /* no outputs */
            : "r" (t1), "r" (t2), "r" (t3), "r" (t4), "r" (t5), "r" (t6),
              "m" (ft1), "m" (ft2), "m" (dt1), "m" (dt2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3", 
              "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
              "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
        );
    }
    
    /* Final computation using all variables to prevent elimination */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6;
    checksum += a + b + c + d;
    checksum += s1 + s2;
    checksum += ch1 + ch2;
    checksum += l1 + l2;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
